# The one-interface router

The initial experiment routed through a Linux box with only one usable Ethernet interface. It demonstrated queue management, but shared upstream and downstream connectivity made enforcement and bypass prevention harder to reason about.

The resulting design uses separate LAN and WAN interfaces, with clients behind the LAN interface and the modem on the WAN side. A working ping or a default-route setting on one client is not proof that every household device uses the intended forwarding path. Verify the physical topology, DHCP gateway, and actual forwarding counters.
