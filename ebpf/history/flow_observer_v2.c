#include <linux/bpf.h>
#include <linux/pkt_cls.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <linux/in.h>
#include <linux/udp.h>
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_endian.h>

/*
 * Canonical flow key:
 *
 * client = device on 192.0.2.0/24 (documentation example)
 * remote = Internet endpoint
 *
 * Thus:
 *   LAN -> WAN and WAN -> LAN generate the SAME key.
 */
struct flow_key {
    __u32 client_ip;      /* host-order IPv4 */
    __u32 remote_ip;      /* host-order IPv4 */
    __u16 client_port;    /* host-order */
    __u16 remote_port;    /* host-order */
};

struct dir_stats {
    __u64 packets;
    __u64 bytes;

    __u64 small_128;
    __u64 small_256;
    __u64 large_1000;

    __u64 first_ns;
    __u64 last_ns;

    __u64 gap_sum_ns;
    __u64 gap_count;

    __u64 gap_lt_20ms;
    __u64 gap_20_50ms;
    __u64 gap_50_250ms;
    __u64 gap_ge_250ms;

    __u32 min_len;
    __u32 max_len;
};

struct flow_stats {
    struct dir_stats up;
    struct dir_stats down;
};

struct {
    __uint(type, BPF_MAP_TYPE_LRU_HASH);
    __uint(max_entries, 4096);
    __type(key, struct flow_key);
    __type(value, struct flow_stats);
} udp_flows_v2 SEC(".maps");

static __always_inline int is_lan(__u32 ip_host)
{
    /* ip_host is after bpf_ntohl(): 192.0.2.x */
    return ((ip_host >> 24) == 192 &&
            ((ip_host >> 16) & 0xff) == 0 &&
            ((ip_host >> 8) & 0xff) == 2);
}

static __always_inline void
init_dir(struct dir_stats *d, __u32 len, __u64 now)
{
    d->packets = 1;
    d->bytes = len;

    d->small_128 = len <= 128;
    d->small_256 = len <= 256;
    d->large_1000 = len >= 1000;

    d->first_ns = now;
    d->last_ns = now;

    d->min_len = len;
    d->max_len = len;
}

static __always_inline void
update_dir(struct dir_stats *d, __u32 len, __u64 now)
{
    /*
     * This is an observer, so approximate min/max/last timing under
     * extremely rare cross-CPU races is acceptable.
     */
    __u64 previous = d->last_ns;

    if (d->packets == 0) {
        init_dir(d, len, now);
        return;
    }

    __sync_fetch_and_add(&d->packets, 1);
    __sync_fetch_and_add(&d->bytes, len);

    if (len <= 128)
        __sync_fetch_and_add(&d->small_128, 1);

    if (len <= 256)
        __sync_fetch_and_add(&d->small_256, 1);

    if (len >= 1000)
        __sync_fetch_and_add(&d->large_1000, 1);

    if (len < d->min_len)
        d->min_len = len;

    if (len > d->max_len)
        d->max_len = len;

    if (previous && now > previous) {
        __u64 gap = now - previous;

        __sync_fetch_and_add(&d->gap_sum_ns, gap);
        __sync_fetch_and_add(&d->gap_count, 1);

        if (gap < 20000000ULL)
            __sync_fetch_and_add(&d->gap_lt_20ms, 1);
        else if (gap < 50000000ULL)
            __sync_fetch_and_add(&d->gap_20_50ms, 1);
        else if (gap < 250000000ULL)
            __sync_fetch_and_add(&d->gap_50_250ms, 1);
        else
            __sync_fetch_and_add(&d->gap_ge_250ms, 1);
    }

    d->last_ns = now;
}

SEC("classifier")
int observe_udp_v2(struct __sk_buff *skb)
{
    void *data = (void *)(long)skb->data;
    void *data_end = (void *)(long)skb->data_end;

    struct ethhdr *eth = data;

    if ((void *)(eth + 1) > data_end)
        return TC_ACT_OK;

    if (eth->h_proto != bpf_htons(ETH_P_IP))
        return TC_ACT_OK;

    struct iphdr *ip = (void *)(eth + 1);

    if ((void *)(ip + 1) > data_end)
        return TC_ACT_OK;

    if (ip->protocol != IPPROTO_UDP)
        return TC_ACT_OK;

    __u32 ihl = ip->ihl * 4;

    if (ihl < sizeof(*ip))
        return TC_ACT_OK;

    struct udphdr *udp = (void *)ip + ihl;

    if ((void *)(udp + 1) > data_end)
        return TC_ACT_OK;

    __u32 src = bpf_ntohl(ip->saddr);
    __u32 dst = bpf_ntohl(ip->daddr);

    int src_lan = is_lan(src);
    int dst_lan = is_lan(dst);

    /*
     * Observe only traffic crossing between our LAN and something
     * outside 192.0.2.0/24 (documentation example).
     *
     * LAN-local UDP (DNS to router, broadcasts, etc.) is ignored.
     */
    if (src_lan == dst_lan)
        return TC_ACT_OK;

    struct flow_key key = {};

    int outbound;

    if (src_lan) {
        outbound = 1;

        key.client_ip   = src;
        key.remote_ip   = dst;
        key.client_port = bpf_ntohs(udp->source);
        key.remote_port = bpf_ntohs(udp->dest);
    } else {
        outbound = 0;

        key.client_ip   = dst;
        key.remote_ip   = src;
        key.client_port = bpf_ntohs(udp->dest);
        key.remote_port = bpf_ntohs(udp->source);
    }

    __u32 len = skb->len;
    __u64 now = bpf_ktime_get_ns();

    struct flow_stats *s =
        bpf_map_lookup_elem(&udp_flows_v2, &key);

    if (!s) {
        struct flow_stats initial = {};

        if (outbound)
            init_dir(&initial.up, len, now);
        else
            init_dir(&initial.down, len, now);

        bpf_map_update_elem(
            &udp_flows_v2,
            &key,
            &initial,
            BPF_ANY
        );

        return TC_ACT_OK;
    }

    if (outbound)
        update_dir(&s->up, len, now);
    else
        update_dir(&s->down, len, now);

    return TC_ACT_OK;
}

char LICENSE[] SEC("license") = "GPL";
