# Failures and lessons

| Attempt | What changed the design |
| --- | --- |
| One-interface routing | Bypass and topology enforcement were awkward. |
| TX3 internal Wi-Fi as the household AP | Capacity and client handling were insufficient. |
| Assume a similarly named in-tree Ethernet driver is enough | This device revision required investigation across USB selection and link detection. |
| Prioritize the whole gaming computer | Its large transfers also gained priority. |
| Treat all small packets or all UDP as interactive | Unrelated traffic gained priority. |
| Infer calls from symmetric small packets | Observed calls were asymmetric. |
| Remove realtime status whenever downstream cadence degrades | Congestion itself made genuine games fail classification. |
| Put every detected call into CAKE Voice | Delay increased on the tiny uplink. |
| Trust an active oneshot after PPP reconnect | The recreated interface had lost queue state. |
| Treat first-hop ICMP spikes as forwarding proof | ICMP control-plane behavior can differ from forwarded traffic. |

The most useful tests changed one component at a time and included an explicit rollback. Successful temporary changes still need lifecycle and persistence work.
