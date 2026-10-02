# Wi-Fi access-point experiments

Using the TX3's internal Wi-Fi as the household access point was an intermediate experiment. The notes report that capacity and client handling were insufficient. An external router was then used as an access point.

The access point should bridge household devices onto the LAN while the TX3 provides routing. Disable competing DHCP services. Windows Internet Connection Sharing previously introduced a rogue DHCP server and misleading connectivity symptoms.

Good ICMP latency to the access point only tests one path and one type of traffic; it does not establish that every wireless client has reliable airtime or forwarding.
