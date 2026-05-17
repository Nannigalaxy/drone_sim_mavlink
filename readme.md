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

#### Example:
summary.json
```json
{
"autopilot": "ArduPilot",
"messages": [
    {
        "count": 7137,
        "fields": [
            {
                "datatype": "uint8_t",
                "max": 0.0,
                "message_count": 7137,
                "min": 0.0,
                "name": "autopilot",
                "null_count": 0,
                "observed_count": 7137
                ...
```

### Full Analysis

```bash
./telemetry_app \
  --analyze-full \
  --file flight.tlog
```

#### Example:
full_log.csv
```
| timestamp        | delta_from_previous_us | msg_ids | ATTITUDE.pitch      | ATTITUDE.pitchspeed   |
| ---------------- | ---------------------- | ------- | ------------------- | --------------------- |
| 1762777055108076 | 0                      | 30      | -0.041307058185339  | -0.00083394767716527  |
| 1762777055108957 | 881                    | 74      | -0.041307058185339  | -0.00083394767716527  |
| 1762777055466178 | 357221                 | 30*74   | -0.0412115380167961 | -0.000211425824090838 |

```

> Note: ^ analyzer also takes UDP as input 

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

#### Example:
On terminal
```
...
=========================
MSG: SYSTEM_TIME (2)
SYS: 1 COMP: 1
TIME: 40455236112
  time_unix_usec = 1.75301e+15 [uint64_t]
  time_boot_ms = 714470 [uint32_t]

=========================
MSG: TERRAIN_REPORT (136)
SYS: 1 COMP: 1
TIME: 40455329840
  lat = 1.30309e+08 [int32_t]
  lon = 7.75655e+08 [int32_t]
  spacing = 55598 [uint16_t]
  terrain_height = 4.66939e-35 [float]
  current_height = 9.25136e-39 [float]
  pending = 0 [uint16_t]
  loaded = 336 [uint16_t]

=========================
MSG: LOCAL_POSITION_NED (32)
SYS: 1 COMP: 1
TIME: 40455376743
  time_boot_ms = 714470 [uint32_t]
  x = 2.53704 [float]
  y = 0.377936 [float]
  z = 1.36678 [float]
  vx = -0.000103231 [float]
  vy = -0.0118041 [float]
  vz = 0.0235051 [float]
  ...
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