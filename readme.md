# drone_sim_mavlink

Drone telemetry replay and MAVLink analysis framework written in C++.

## Features

- Replay `.tlog` telemetry logs
- Analyze MAVLink telemetry packets
- Simulate live telemetry over UDP
- RTSP video stream viewer
- MAVLink v1/v2 support
- PX4 and ArduPilot compatible

---

## Build

```bash
mkdir build
cd build
cmake ..
make -j$(nproc)
```

---

## Telemetry Commands

### Analyze Summary

```bash
./telemetry_app \
  --analyze-summary \
  --file flight.tlog
```

### Full Analysis

```bash
./telemetry_app \
  --analyze-full \
  --file flight.tlog
```

### Replay `.tlog`

```bash
./telemetry_app \
  --replay \
  --file flight.tlog
```

### Replay Over UDP

```bash
./telemetry_app \
  --replay \
  --udp 14550 [bind_ip]
```

---

## RTSP Video Viewer

```bash
./video_app
```

Default stream:

```text
rtsp://127.0.0.1:8554/live
```

---

## Docker

```bash
docker build -t drone_sim_mavlink .
```

---

## Supported

- MAVLink v1/v2
- PX4
- ArduPilot
- UDP telemetry
- RTSP video streams