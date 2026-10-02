# Reported experiments

These are sanitized summaries from session notes. They are not freshly measured results, and the underlying raw logs are not published.

| Experiment | Observation | Interpretation and limit |
| --- | --- | --- |
| Initial household use | Roughly 30 ms idle; hundreds of ms under load, sometimes about 900 ms | Historical owner report; endpoints and sample counts unavailable |
| V4 isolated detachment | Video improved while CAKE and routing remained enabled | Evidence of classifier collateral effects on that workload |
| V5D shadow saturation | UNKNOWN 32, CANDIDATE 3, REALTIME 1, BULK 5, VOICE 2 | A single reported map snapshot, not an accuracy rate |
| CS6 Voice at 400 Kbit/s upload | Voice peak/average delay about 630/414 ms; Video about 33.4/11.9 ms | Reported CAKE counters from a failed experiment; policy rolled back |
| 430/9800 Kbit/s upload/download | Owner reported normal browsing, video, and gaming | Last subjectively validated combination |
| 450/10000 Kbit/s | Reported live qdisc rates | Experience validation unfinished |
| V5D verifier/JIT | Reported successful load and removal on the TX3 | Historical target-kernel check, not repeated by a local compile |

## Repeatable next test

Record kernel, source revision, DSL sync, framing, qdisc parameters, classifier policy, topology, test endpoint, duration, and background load. Measure idle and saturated upload/download separately, then mixed game/call/transfer use. Retain loss and p50/p95/p99 latency alongside throughput. Change one variable at a time and keep private addresses and payloads out of public artifacts.
