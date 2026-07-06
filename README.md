# tsf-systemd-ts

A Test Environment suite that exercises
[tsf-systemd](https://github.com/interpretica-io/tsf-systemd)
(`tapi_systemd`) against the agent it runs on — reading its systemd
units and service-hardening posture over libsystemd's sd-bus.

| Test | What it checks |
|---|---|
| `list` | `tapi_systemd_list()` returns a non-empty, self-consistent snapshot of units, and `tapi_systemd_hardening()` reads one active service's exec-context properties |
| `audit` | `tapi_systemd_audit()` produces a well-formed hardening report over the loaded services; the posture is logged (informational — a stock host is mostly unconfined) |

What units exist is the host's to decide, so **the suite asserts no
particular unit**: a host without systemd skips cleanly, and the audit
does not fail on findings (a stock host runs many services with the
systemd defaults). Reading runs on the agent, in its RPC server, over
libsystemd's sd-bus — read-only, no `systemctl`.

## Running it

Needs Docker and `test-environment` as a sibling directory:

```bash
./scripts/run.sh docker guess --cfg=localhost
```

Or natively (agent on the host, so it sees the real systemd):

```bash
./scripts/run.sh guess --cfg=localhost
```

The agent must carry **libsystemd with its headers**
(`libsystemd-dev`, in the suite's Dockerfile for the container flow) and
a running systemd. `tapi_systemd`'s posture reports through
tsf-cybersec, so the suite also builds the tsf-cybersec chain
(tsf-cybersec → tsf-kernel → tsf-devtool); their refs are in
`conf/external.yml`.

`conf/builder.conf.lock` pins the commits actually built (empty until
the first build). The Builder clones the tsf-* repositories itself.

## Status

**Not yet run.** This suite was written alongside tsf-systemd but has
not been built or executed here — there was no TE toolchain, and
tsf-systemd's sd-bus code is Linux-only (not compilable on the
development host). The first run should expect the ordinary first-build
fixes.
