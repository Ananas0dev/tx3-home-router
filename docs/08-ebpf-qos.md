# From device priority to flow behavior

A gaming computer can generate both latency-sensitive game packets and large transfers. Giving the whole device priority cannot distinguish them. The classifier instead observes bidirectional UDP flows, packet sizes, cadence, and accumulated history.

V4 helped the game but affected ordinary video. Detaching only V4 while leaving CAKE and routing intact improved the reported video behavior. V5 introduced a Voice class. V5B relaxed inaccurate assumptions about symmetric calls. V5C rejected flows with too many large upstream packets. V5D added hysteresis: a flow must first earn REALTIME under strict conditions, then may retain it during downstream congestion if upstream behavior remains suitable.

A shadow classifier attached after an existing direct-action classifier initially saw no useful traffic. Moving it before the earlier classifier restored observation. TC preference and filter handles matter; replacing one filter does not guarantee that every old handle is gone.

The nftables experiment also exposed rule-ordering problems: a stale-mark cleanup rule erased a newly assigned Voice conntrack mark. The repository preserves that lesson and does not claim a finalized production V5D lifecycle.

See [the source guide](../ebpf/README.md) for limitations and build instructions.
