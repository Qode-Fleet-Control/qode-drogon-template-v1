# Drogon template

Provisioned from [`Qode-Fleet-Control/fleet-template-v1`](https://github.com/Qode-Fleet-Control/fleet-template-v1) — the fleet
lifecycle contract (`bin/`, `fleet.conf`, `compose.yaml`, deploy workflows) with a Drogon starter laid on top.

An HTTP service on Drogon 1.9 (Debian trixie), the C++17/20 asynchronous web framework, generated with `drogon_ctl`. Routes: `GET /` (plain-text greeting, `controllers/RootCtrl`) and `GET /health` (`{"status":"ok"}`, `controllers/HealthCtrl`), both `HttpSimpleController`s.

## Origin

    drogon_ctl create project qode-drogon-template-v1 && cd qode-drogon-template-v1/controllers && drogon_ctl create controller HealthCtrl && drogon_ctl create controller RootCtrl

Run with drogon_ctl 1.9.0 from Debian trixie's `drogon` package, inside a container: `docker run --rm -v "$PWD":/w -w /w debian:trixie bash -c 'apt-get update && apt-get install -y --no-install-recommends drogon && drogon_ctl create project ...'`.

## Run it

### On the fleet

The fleet runs it as containers (the docker runtime): `bin/run` builds the image with
`docker compose build` and then starts it with `docker compose up` in the foreground, publishing `$PORT`.

It listens on `0.0.0.0:$PORT` (default `8080`), read from the environment when the container starts,
and serves at the root of its own hostname (`https://<hash>.<FLEET_APP_DOMAIN>/`). The health check hits `/health`.

### With docker

```sh
PORT=8080 bin/run                 # build + run through compose, Ctrl-C to stop
docker compose up --build             # the same, by hand
curl localhost:8080/health
```

### Without docker

```sh
# Debian/Ubuntu: sudo apt install build-essential cmake libdrogon-dev
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build -j
PORT=8080 ./build/qode-drogon-template-v1
# or: FLEET_RUNTIME=process PORT=8080 bin/run
```

`fleet.conf` drives every script in `bin/`:

| step | docker runtime (fleet) | `FLEET_RUNTIME=process` |
|---|---|---|
| install | — | `(none)` |
| build | `docker compose build` | `cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build -j` |
| start | `docker compose up --remove-orphans` | `env PORT="$PORT" ./build/qode-drogon-template-v1` |

## Layout

Stock `drogon_ctl` layout: `main.cc`, `CMakeLists.txt`, `config.json` / `config.yaml` (sample configs, not loaded), `controllers/`, `filters/`, `plugins/`, `models/model.json`, `views/`, `test/` (a `DROGON_TEST` runner).
- `Dockerfile` — `debian:trixie` build stage with `libdrogon-dev`, builds only the app target; `debian:trixie-slim` runtime with `libdrogon1t64`; non-root user `app`.
- `compose.yaml` — service `app`, publishes `${PORT:-8080}:${PORT:-8080}`, fleet variables passed through by name.

## Deviations from stock, and why

- `main.cc`: the generated `addListener("0.0.0.0", 5555)` now reads `$PORT` at runtime (default 8080), and `setThreadNum(0)` uses one IO thread per core.
- `controllers/RootCtrl` and `controllers/HealthCtrl` are generated stubs with a `PATH_ADD` and a response body filled in.
- The generated empty `build/` directory is removed; `filters/`, `plugins/`, `views/` keep a `.gitkeep` so git keeps the generated layout.
- The generated `.gitignore` is kept and the fleet entries (`.env`, `.fleet/`, `.fleet-deploy.log`, `*.log`, `build/`) are appended.

## Verified

**Not verified.** The image was never built: the shared docker host's disk stayed under 1 GB for hours while this was cut, and the one build attempt was cancelled by a disk guard (146 MB left) during `apt-get install`. `drogon_ctl` itself ran (output above). Build and boot it once before trusting it:

    /workspace/claude-workspace/agent-fleet/.claude/skills/migrate-docker-runtime/scripts/verify.sh . <port>

See `docs/fleet-lifecycle.md` for the lifecycle contract.
