# Built by .github/workflows/deploy.yml and pushed to Artifact Registry.
#
# Drogon 1.9 on Debian trixie. Multi-stage: the build stage has the compiler
# and libdrogon-dev; the runtime stage installs only the Drogon shared library
# (libdrogon1t64 pulls trantor, jsoncpp and the database client libraries it
# was built with) and runs as a non-root user. The port is read from $PORT
# when the container starts, not at build time (see main.cc).
FROM debian:trixie AS build
RUN apt-get update \
 && apt-get install -y --no-install-recommends build-essential cmake ninja-build libdrogon-dev \
      libjsoncpp-dev uuid-dev zlib1g-dev libssl-dev libbrotli-dev libc-ares-dev \
      libpq-dev libsqlite3-dev default-libmysqlclient-dev libhiredis-dev libyaml-cpp-dev \
 && rm -rf /var/lib/apt/lists/*
WORKDIR /src
COPY . .
RUN cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release \
 && cmake --build build --target qode-drogon-template-v1 \
 && install -D build/qode-drogon-template-v1 /out/app

FROM debian:trixie-slim AS runtime
RUN apt-get update \
 && apt-get install -y --no-install-recommends libdrogon1t64 \
 && rm -rf /var/lib/apt/lists/* \
 && useradd -r -u 10001 app
WORKDIR /app
ARG BUILD_ID=""
ENV PORT=8080 BUILD_ID=$BUILD_ID
COPY --from=build /out/app /app/app
# Drogon creates ./uploads/tmp/00..FF under its working dir at startup; the app user must
# own it (WORKDIR is created by root).
RUN chown app:app /app
EXPOSE 8080
USER app
ENTRYPOINT ["/app/app"]
