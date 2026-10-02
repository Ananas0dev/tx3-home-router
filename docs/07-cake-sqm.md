# CAKE queue management

The later setup shaped upload on `ppp0` and download on `ifb-wan`. Both used `diffserv4`, NAT awareness, `nowash`, split GSO, an RTT setting of 100 ms, and ATM overhead 40.

Upload used `dual-srchost` and ACK filtering. Download used `dual-dsthost`, `ingress`, and no ACK filtering. Endpoint-provided DSCP needed sanitization so applications could not consume priority tins arbitrarily.

Rates evolved through 400/9500, 430/9800, and 450/10000 Kbit/s upload/download. The middle setting received a positive household-use report; the last was still awaiting validation. Older persistent scripts reportedly retained 480/9500.

The failed CS6 Voice experiment produced high delay on a very small uplink. The final noted policy kept VOICE at CS0 and REALTIME at AF41. Classification names, DSCP values, and CAKE tins are different layers; naming a flow VOICE does not require putting it in the Voice tin.

For CAKE option semantics see the [iproute2 manual](https://man7.org/linux/man-pages/man8/tc-cake.8.html).
