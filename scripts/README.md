# Recovered deployment scripts

These are sanitized copies read from the live router on 2026-10-02. `router-cake` installs the observed 450/10000 Kbit/s settings; `tx3-qos-v4` implements the observed V4 lifecycle. The PPP refresh hook still restarts V4.

**These scripts change networking. They are reference material, not a portable installer.** Review hardcoded interfaces, TC preferences, BPF paths, nftables ownership, and bandwidth before use. `router-cake` removes qdiscs and recreates the IFB. The monitor's target addresses are documentation placeholders and must be changed before use. No script was run on the live router during publication preparation.

Build the appropriate source with your actual LAN subnet before deploying an object. The source archive defaults to a documentation network. Service units refer to installed `/usr/local` paths; this repository does not install them automatically.
