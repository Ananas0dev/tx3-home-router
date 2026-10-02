# Roadmap

- Reconcile live V5D policy with systemd, PPP reconnect hooks, and saved CAKE rates.
- Repeat mixed game/call/upload tests with recorded duration, rates, and latency distributions.
- Validate classifier behavior for fragmented packets, VLANs, IPv6, concurrent map updates, and long-lived flows before broader deployment.
- Investigate call recognition when downstream cadence is delayed without broadening false positives.
- Reproduce the AX88179B failure against a current supported kernel and separate configuration selection from link polling.
- Produce a minimal, correctly attributed upstream report if the failure remains reproducible.
- Document recovery and reboot validation against the actual storage layout.
