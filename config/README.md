# Configuration references

These files were read from the router on 2026-10-02 and sanitized. LAN and modem-side addresses use documentation ranges. The PPP peer name in the public service units is `wan`; create its private credentials separately if adapting the deployment.

`nftables/router.nft.example` is the persistent file, not a dump of the complete active ruleset. It contains older port-based QoS rules. Runtime services install additional classifier/sanitizer rules; the V4 lifecycle script contains its own nftables logic. The persistent file begins with `flush ruleset`, which would remove those runtime tables if applied. Do not load it onto a live router without reconciling service ordering and policy.

The reference firewall is not a complete general-purpose IPv6 policy and is not offered as a portable hardening template. The networkd examples similarly retain the original interface assumptions. DHCP leases, hostname inventories, private DNS configuration, PPP credentials, and TLS/SSH files are deliberately absent.
