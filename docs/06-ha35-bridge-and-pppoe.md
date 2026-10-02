# HA35 bridge and PPPoE

The later architecture moves PPPoE termination onto the TX3, with the HA35 acting as a DSL bridge. The USB interface carries PPPoE; the resulting `ppp0` is the routed WAN interface.

PPPoE reconnects recreate `ppp0`. Qdiscs and filters attached to the previous interface disappear even if a systemd oneshot service still appears active. A PPP `ip-up.d` hook was introduced to refresh queue management.

The notes' existing hook restarted both `router-cake.service` and `tx3-qos-v4.service`. That conflicts with a temporary V5D-only experiment. Persistent configuration, reconnect hooks, and the live classifier must agree before claiming reboot persistence.

Credentials belong in private PPP configuration with restrictive permissions. No ISP usernames, passwords, PAP/CHAP secrets, or live peer files are included here.
