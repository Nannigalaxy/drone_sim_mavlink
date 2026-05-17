#!/usr/bin/env python3

"""
Simple MAVLink .tlog replay simulator over UDP.

Usage:
    python3 tlog2udp.py ./flight.tlog

Optional:
    python3 tlog2udp.py ./flight.tlog 127.0.0.1 14550

Default target:
    127.0.0.1:14550
"""

import socket
import struct
import sys
import time


MAVLINK_V1_MAGIC = 0xFE
MAVLINK_V2_MAGIC = 0xFD


def read_timestamp_us(f):
    """
    .tlog format:
        uint64 timestamp (microseconds)
        followed by MAVLink packet
    """
    data = f.read(8)

    if len(data) != 8:
        return None

    return struct.unpack(">Q", data)[0]


def read_mavlink_packet(f):
    """
    Read one MAVLink packet from stream.
    Supports MAVLink v1 and v2.
    """

    magic = f.read(1)

    if not magic:
        return None

    magic = magic[0]

    if magic == MAVLINK_V1_MAGIC:
        header = f.read(5)

        if len(header) != 5:
            return None

        payload_len = header[0]

        payload = f.read(payload_len)
        checksum = f.read(2)

        if len(payload) != payload_len or len(checksum) != 2:
            return None

        packet = bytes([magic]) + header + payload + checksum
        return packet

    elif magic == MAVLINK_V2_MAGIC:
        header = f.read(9)

        if len(header) != 9:
            return None

        payload_len = header[0]
        incompat_flags = header[1]

        payload = f.read(payload_len)
        checksum = f.read(2)

        if len(payload) != payload_len or len(checksum) != 2:
            return None

        packet = bytes([magic]) + header + payload + checksum

        # MAVLink v2 signature
        if incompat_flags & 0x01:
            signature = f.read(13)

            if len(signature) != 13:
                return None

            packet += signature

        return packet

    else:
        # Skip unknown byte
        return None


def replay_tlog(tlog_path, udp_ip, udp_port):

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    with open(tlog_path, "rb") as f:

        first_timestamp = None
        replay_start = None

        packet_count = 0

        while packet_count < 500:

            timestamp_us = read_timestamp_us(f)

            if timestamp_us is None:
                break

            packet = read_mavlink_packet(f)

            if packet is None:
                continue

            if first_timestamp is None:
                first_timestamp = timestamp_us
                replay_start = time.time()

            # Maintain original timing
            elapsed_log_time = (timestamp_us - first_timestamp) / 1_000_000
            elapsed_real_time = time.time() - replay_start

            sleep_time = elapsed_log_time - elapsed_real_time

            if sleep_time > 0:
                time.sleep(sleep_time)

            sock.sendto(packet, (udp_ip, udp_port))

            packet_count += 1

            # Basic info
            msg_id = get_message_id(packet)

            print(
                f"Packet #{packet_count:06d} | "
                f"MsgID={msg_id:<5} | "
                f"Size={len(packet):<4} bytes"
            )

    print("\nReplay completed.")


def get_message_id(packet):

    magic = packet[0]

    try:

        if magic == MAVLINK_V1_MAGIC:
            return packet[5]

        elif magic == MAVLINK_V2_MAGIC:
            return (
                packet[7]
                | (packet[8] << 8)
                | (packet[9] << 16)
            )

    except Exception:
        pass

    return -1


def main():

    if len(sys.argv) < 2:
        print(
            "Usage:\n"
            "python3 replay_tlog.py flight.tlog [ip] [port]"
        )
        sys.exit(1)

    tlog_path = sys.argv[1]

    udp_ip = "127.0.0.1"
    udp_port = 14550

    if len(sys.argv) >= 3:
        udp_ip = sys.argv[2]

    if len(sys.argv) >= 4:
        udp_port = int(sys.argv[3])

    print("\n========== TLOG REPLAY ==========")
    print(f"TLog File : {tlog_path}")
    print(f"Target IP : {udp_ip}")
    print(f"Target Port : {udp_port}")
    print("=================================\n")

    replay_tlog(tlog_path, udp_ip, udp_port)


if __name__ == "__main__":
    main()