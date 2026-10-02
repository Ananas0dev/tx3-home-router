#include <linux/bpf.h>
#include <linux/pkt_cls.h>
#include <bpf/bpf_helpers.h>

SEC("classifier")
int hello_tc(struct __sk_buff *skb)
{
    return TC_ACT_OK;
}

char LICENSE[] SEC("license") = "GPL";
