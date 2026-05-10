# Drone Sim Mavlink
> Project is currently under development.


![TLog path validation](docs/screenshot/uavlog_validation.png)
^ validating tlog

![SITL+Gazebo setup](docs/screenshot/sim1.png)
^ SITL and Gazebo setup



# TLog Parser and Replay
> (!) GPS path replay in development

## Build Instructions

```bash
mkdir build
cd build
cmake ..
make
```

Usage:
```bash
./drone_sim_mavlink --analyze input/test_flight_tail1.tlog
```


## Configuration

Telemetry message fields are configured through:

```text
config/message_types_subcat_id.json
```
Source ref: https://mavlink.io/en/messages/common.html