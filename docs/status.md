# Evidence and operational status

Prepared on 2026-10-02. Sources are the owner's session notes and the recovered WSL files. See the [live observation](live-observation.md): V4 is active and saved CAKE rates are 450/10000 Kbit/s. The table below describes historical evidence, not the current deployment.

| Item | Evidence | Limit |
| --- | --- | --- |
| V5D source | Recovered C files, including shadow variant | Source availability does not prove it is currently attached |
| USB workarounds | Recovered Linux and ASIX patches | Not accepted upstream; hardware regression testing remains necessary |
| Armbian + PPPoE | Later session account | Earlier notes describe a different deployment |
| 430/9800 Kbit/s | Positive owner report | Subjective household test |
| 450/10000 Kbit/s | Last recorded experiment | No completed experience report in the notes |
| Persistence | Notes explicitly identify V4/rate restoration risk | Do not claim reboot-safe V5D deployment |

The public eBPF sources substitute `192.0.2.0/24` for the private LAN and allow build-time overrides. This deliberately changes the source hashes from the private originals. The classifier logic otherwise remains as recovered. Older revisions are retained for comparison, not installation.
