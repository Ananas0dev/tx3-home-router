# Read-only live observation — 2026-10-02

The server was inspected over SSH without restarting services or changing network state.

- Kernel: `6.12.110-ophub`, AArch64.
- Calendar, LAN Share, router-cake, and tx3-qos-v4 services: active.
- Upload CAKE: 450 Kbit/s, diffserv4, dual-srchost, NAT, ACK filtering, ATM overhead 40.
- Download CAKE: 10 Mbit/s, diffserv4, dual-dsthost, NAT, ingress, ATM overhead 40.
- LAN ingress and egress: BPF preference 12348; same program tag `d616f31ac6581d68`.
- The V4 service points to `flow_classifier_v4.o`; the PPP reconnect hook restarts V4.
- Saved router-cake script now also specifies 450/10000 Kbit/s.

**The observed deployment is V4, not the V5D-only canary described in the historical notes.** The inspection did not establish why it changed or prove a reboot occurred. No load test, reconnect, or reboot was performed. V5D remains a recovered experiment with reported historical results.
