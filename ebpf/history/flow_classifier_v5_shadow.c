#include <linux/bpf.h>
#include <linux/pkt_cls.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <linux/in.h>
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_endian.h>

/*
 * V5 BEHAVIORAL UDP REALTIME CLASSIFIER
 *
 * OBSERVATION ONLY:
 *   - no DSCP modification
 *   - no skb priority modification
 *   - no dropping
 *   - no redirect
 *
 * The key is the complete LAN-client <-> Internet UDP flow.
 */

#define CLASS_UNKNOWN       0
#define CLASS_CANDIDATE     1
#define CLASS_REALTIME      2
#define CLASS_BULK          3
#define CLASS_VOICE         4

/*
 * Temporary skb mark used only between tc/eBPF and nftables.
 * nftables will consume/clear this before the packet leaves the router.
 */
#define V5_REALTIME_SKB_MARK 0x40000000U
#define V5_VOICE_SKB_MARK    0x20000000U

#define REASON_SUSTAINED    (1U << 0)
#define REASON_BIDIR        (1U << 1)
#define REASON_SMALL_UP     (1U << 2)
#define REASON_SMALL_DOWN   (1U << 3)
#define REASON_FAST_UP      (1U << 4)
#define REASON_FAST_DOWN    (1U << 5)
#define REASON_BULK_DOWN    (1U << 6)
#define REASON_TOO_FEW      (1U << 7)

/* 192.0.2.0/24 (documentation example), represented after bpf_ntohl(). */
#ifndef LAN_NET
#define LAN_NET  0xC0000200U
#endif
#ifndef LAN_MASK
#define LAN_MASK 0xFFFFFF00U
#endif

struct flow_key {
    __u32 client_ip;
    __u32 remote_ip;
    __u16 client_port;
    __u16 remote_port;
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

struct flow_state {
    struct dir_stats up;
    struct dir_stats down;

    __u32 classification;
    __u32 confidence;
    __u32 reasons;
    __u32 reserved;

    __u64 first_seen_ns;
    __u64 last_seen_ns;
};

/*
 * LRU means abandoned UDP flows eventually disappear instead of filling
 * the table forever.
 */
struct {
    __uint(type, BPF_MAP_TYPE_LRU_HASH);
    __uint(max_entries, 8192);
    __type(key, struct flow_key);
    __type(value, struct flow_state);
} flow_states SEC(".maps");


static __always_inline int is_lan(__u32 host_ip)
{
    return (host_ip & LAN_MASK) == LAN_NET;
}


static __always_inline void update_dir(struct dir_stats *d,
                                       __u32 len,
                                       __u64 now)
{
    if (d->packets == 0) {
        d->first_ns = now;
        d->min_len = len;
        d->max_len = len;
    } else {
        __u64 gap = now - d->last_ns;

        d->gap_sum_ns += gap;
        d->gap_count++;

        if (gap < 20000000ULL)
            d->gap_lt_20ms++;
        else if (gap < 50000000ULL)
            d->gap_20_50ms++;
        else if (gap < 250000000ULL)
            d->gap_50_250ms++;
        else
            d->gap_ge_250ms++;

        if (len < d->min_len)
            d->min_len = len;

        if (len > d->max_len)
            d->max_len = len;
    }

    d->packets++;
    d->bytes += len;

    if (len <= 128)
        d->small_128++;

    if (len <= 256)
        d->small_256++;

    if (len >= 1000)
        d->large_1000++;

    d->last_ns = now;
}


/*
 * Conservative shadow classifier.
 *
 * Deliberately does NOT know:
 *   - Overwatch ports
 *   - Blizzard IP ranges
 *   - device identity
 *
 * It only looks at flow behavior.
 */
static __always_inline void classify(struct flow_state *s, __u64 now)
{
    __u64 up = s->up.packets;
    __u64 down = s->down.packets;
    __u64 total = up + down;
    __u64 age_ns;
    __u32 reasons = 0;
    __u32 confidence = 0;

    if (s->first_seen_ns == 0)
        return;

    age_ns = now - s->first_seen_ns;

    /*
     * Absolutely refuse to call tiny transient exchanges realtime.
     * This is aimed directly at the 2-up/2-down noise seen in the
     * no-game capture.
     */
    if (total < 20 || age_ns < 2000000000ULL) {
        s->classification = CLASS_UNKNOWN;
        s->confidence = 0;
        s->reasons = REASON_TOO_FEW;
        return;
    }

    reasons |= REASON_SUSTAINED;
    confidence += 2;

    /*
     * Require meaningful traffic in BOTH directions.
     */
    if (up >= 8 && down >= 8) {
        reasons |= REASON_BIDIR;
        confidence += 2;
    }

    /*
     * Fraction tests are written without floating point:
     *
     * small_256 / packets >= 80%
     */
    if (up && s->up.small_256 * 100 >= up * 80) {
        reasons |= REASON_SMALL_UP;
        confidence += 2;
    }

    if (down && s->down.small_256 * 100 >= down * 80) {
        reasons |= REASON_SMALL_DOWN;
        confidence += 1;
    }

    /*
     * Average cadence.
     *
     * <= 100 ms upstream = >= ~10pps
     * <= 150 ms downstream = >= ~6.7pps
     */
    if (s->up.gap_count &&
        s->up.gap_sum_ns / s->up.gap_count <= 100000000ULL) {
        reasons |= REASON_FAST_UP;
        confidence += 2;
    }

    if (s->down.gap_count &&
        s->down.gap_sum_ns / s->down.gap_count <= 150000000ULL) {
        reasons |= REASON_FAST_DOWN;
        confidence += 1;
    }

    /*
     * Strong anti-QUIC/video/download signal.
     *
     * If >=40% of downstream packets are >=1000 B, classify BULK.
     *
     * Our same-PC test had obvious false positives with downstream
     * averages around 1290-1328 B. This should kill those quickly.
     */
    if (down >= 20 &&
        s->down.large_1000 * 100 >= down * 40) {
        reasons |= REASON_BULK_DOWN;

        s->classification = CLASS_BULK;
        s->confidence = 0;
        s->reasons = reasons;
        return;
    }

    /*
     * V5 VOICE:
     *
     * A deliberately conservative subset of realtime intended for
     * voice-like media flows.
     *
     * Require:
     *   - at least 40 observed packets
     *   - at least 3 seconds of history
     *   - meaningful traffic in both directions
     *   - >= 90% <=256-byte packets in BOTH directions
     *   - packet counts within roughly 2.2x of each other
     *   - useful cadence in BOTH directions
     *
     * This is intentionally stricter than generic REALTIME so a
     * high-rate/asymmetric game flow should normally remain REALTIME.
     */
    if (total >= 40 &&
        age_ns >= 3000000000ULL &&
        up >= 20 &&
        down >= 20 &&
        s->up.small_256 * 100 >= up * 90 &&
        s->down.small_256 * 100 >= down * 90 &&
        up * 100 <= down * 220 &&
        down * 100 <= up * 220 &&
        (reasons & REASON_FAST_UP) &&
        (reasons & REASON_FAST_DOWN)) {
        s->classification = CLASS_VOICE;
        s->confidence = confidence;
        s->reasons = reasons;
        return;
    }

    /*
     * REALTIME is intentionally strict.
     *
     * Require:
     *   sustained flow
     *   bidirectional traffic
     *   mostly-small upstream
     *   at least one meaningful cadence signal
     *
     * This should catch the high-rate gameplay family while avoiding
     * classifying every tiny keepalive as realtime.
     */
    if ((reasons & REASON_BIDIR) &&
        (reasons & REASON_SMALL_UP) &&
        (reasons & (REASON_FAST_UP | REASON_FAST_DOWN)) &&
        confidence >= 7) {
        s->classification = CLASS_REALTIME;
    }
    else if ((reasons & REASON_BIDIR) &&
             (reasons & REASON_SMALL_UP) &&
             confidence >= 5) {
        s->classification = CLASS_CANDIDATE;
    }
    else {
        s->classification = CLASS_UNKNOWN;
    }

    s->confidence = confidence;
    s->reasons = reasons;
}


SEC("classifier")
int classify_v5_shadow(struct __sk_buff *skb)
{
    void *data = (void *)(long)skb->data;
    void *data_end = (void *)(long)skb->data_end;

    struct ethhdr *eth = data;
    struct iphdr *ip;
    struct udphdr *udp;

    struct flow_key key = {};
    struct flow_state zero = {};
    struct flow_state *state;

    __u32 ihl;
    __u32 src;
    __u32 dst;
    __u32 pkt_len;
    __u64 now;

    int upstream;

    if ((void *)(eth + 1) > data_end)
        return TC_ACT_OK;

    if (eth->h_proto != bpf_htons(ETH_P_IP))
        return TC_ACT_OK;

    ip = (void *)(eth + 1);

    if ((void *)(ip + 1) > data_end)
        return TC_ACT_OK;

    if (ip->protocol != IPPROTO_UDP)
        return TC_ACT_OK;

    ihl = ip->ihl * 4;

    if (ihl < sizeof(*ip))
        return TC_ACT_OK;

    udp = (void *)ip + ihl;

    if ((void *)(udp + 1) > data_end)
        return TC_ACT_OK;

    src = bpf_ntohl(ip->saddr);
    dst = bpf_ntohl(ip->daddr);

    /*
     * Normalize both directions into:
     *
     * client_ip:client_port -> remote_ip:remote_port
     */
    if (is_lan(src) && !is_lan(dst)) {
        key.client_ip = src;
        key.remote_ip = dst;
        key.client_port = bpf_ntohs(udp->source);
        key.remote_port = bpf_ntohs(udp->dest);
        upstream = 1;
    }
    else if (!is_lan(src) && is_lan(dst)) {
        key.client_ip = dst;
        key.remote_ip = src;
        key.client_port = bpf_ntohs(udp->dest);
        key.remote_port = bpf_ntohs(udp->source);
        upstream = 0;
    }
    else {
        return TC_ACT_OK;
    }

    now = bpf_ktime_get_ns();
    pkt_len = skb->len;

    state = bpf_map_lookup_elem(&flow_states, &key);

    if (!state) {
        zero.first_seen_ns = now;
        zero.last_seen_ns = now;

        bpf_map_update_elem(&flow_states, &key, &zero, BPF_NOEXIST);

        state = bpf_map_lookup_elem(&flow_states, &key);

        if (!state)
            return TC_ACT_OK;
    }

    if (upstream)
        update_dir(&state->up, pkt_len, now);
    else
        update_dir(&state->down, pkt_len, now);

    state->last_seen_ns = now;

    /*
     * Re-evaluating every packet is fine for this shadow experiment.
     * Later we can reduce this if desired.
     */
    classify(state, now);

    /*
     * V5 action:
     *
     * Only LAN -> WAN packets receive temporary skb markers.
     *
     * VOICE:
     *   separate marker for later mapping to CAKE Voice
     *
     * REALTIME:
     *   separate marker for later mapping to AF41 / CAKE Video
     *
     * CANDIDATE, UNKNOWN and BULK remain untouched.
     *
     * These are NOT DSCP values. nftables consumes/clears the markers.
     */
    /*
     * SHADOW BUILD:
     * Keep the complete V5 classifier and flow-state updates, but never
     * modify skb->mark. This object is observation-only.
     */
    (void)upstream;

    return TC_ACT_OK;
}

char LICENSE[] SEC("license") = "GPL";
