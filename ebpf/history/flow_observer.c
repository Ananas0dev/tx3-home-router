#include <linux/bpf.h>
#include <linux/pkt_cls.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <linux/in.h>
#include <linux/udp.h>
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_endian.h>

struct flow_key {
    __u32 saddr;
    __u32 daddr;
    __u16 sport;
    __u16 dport;
};

struct flow_stats {
    __u64 packets;
    __u64 bytes;

    __u64 small_128;
    __u64 small_256;
    __u64 large_1000;

    __u64 first_ns;
    __u64 last_ns;

    __u32 min_len;
    __u32 max_len;
};

struct {
    __uint(type, BPF_MAP_TYPE_LRU_HASH);
    __uint(max_entries, 4096);
    __type(key, struct flow_key);
    __type(value, struct flow_stats);
} udp_flows SEC(".maps");

SEC("classifier")
int observe_udp(struct __sk_buff *skb)
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

    struct flow_key key = {
        .saddr = ip->saddr,
        .daddr = ip->daddr,
        .sport = udp->source,
        .dport = udp->dest,
    };

    /*
     * skb->len is useful here because we're interested in the actual
     * queued packet size seen by TC, not application payload contents.
     */
    __u32 len = skb->len;
    __u64 now = bpf_ktime_get_ns();

    struct flow_stats *s = bpf_map_lookup_elem(&udp_flows, &key);

    if (!s) {
        struct flow_stats initial = {
            .packets = 1,
            .bytes = len,
            .small_128 = len <= 128,
            .small_256 = len <= 256,
            .large_1000 = len >= 1000,
            .first_ns = now,
            .last_ns = now,
            .min_len = len,
            .max_len = len,
        };

        bpf_map_update_elem(&udp_flows, &key, &initial, BPF_ANY);
        return TC_ACT_OK;
    }

    __sync_fetch_and_add(&s->packets, 1);
    __sync_fetch_and_add(&s->bytes, len);

    if (len <= 128)
        __sync_fetch_and_add(&s->small_128, 1);

    if (len <= 256)
        __sync_fetch_and_add(&s->small_256, 1);

    if (len >= 1000)
        __sync_fetch_and_add(&s->large_1000, 1);

    /*
     * One TC instance is processing this router's LAN interface.
     * Slightly approximate min/max values are perfectly adequate for
     * this observational experiment.
     */
    if (len < s->min_len)
        s->min_len = len;

    if (len > s->max_len)
        s->max_len = len;

    s->last_ns = now;

    return TC_ACT_OK;
}

char LICENSE[] SEC("license") = "GPL";
