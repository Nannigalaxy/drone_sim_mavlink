#!/usr/bin/env bash

set -e

# ============================================================
# RTSP streaming helper
#
# Supports:
#   1. USB webcam streaming
#   2. Local video file streaming
#
# Usage:
#
#   Webcam mode:
#       ./video2rtsp.sh webcam
#
#   Local video file mode:
#       ./video2rtsp.sh file sample.mp4
#
# Test stream:
#       ffplay rtsp://127.0.0.1:8554/live
#
# VLC:
#       Media -> Open Network Stream
#       rtsp://127.0.0.1:8554/live
#
# Requirements:
#   - ffmpeg
#   - mediamtx
#
# Install ffmpeg:
#       sudo apt install ffmpeg
#
# ============================================================

MEDIA_MTX_BIN="./mediamtx"

RTSP_URL="rtsp://127.0.0.1:8554/live"

VIDEO_DEVICE="/dev/video0"

WIDTH="1280"
HEIGHT="720"
FPS="30"

show_usage() {
    echo
    echo "Usage:"
    echo
    echo "  Webcam mode:"
    echo "    ./run_rtsp.sh webcam"
    echo
    echo "  Local video file mode:"
    echo "    ./run_rtsp.sh file <video_file>"
    echo
}

cleanup() {
    echo
    echo "Stopping..."

    if [[ -n "${STREAM_PID:-}" ]]; then
        kill "$STREAM_PID" 2>/dev/null || true
    fi

    if [[ -n "${MEDIAMTX_PID:-}" ]]; then
        kill "$MEDIAMTX_PID" 2>/dev/null || true
    fi
}

trap cleanup EXIT INT TERM

MODE="${1:-}"

if [[ -z "$MODE" ]]; then
    show_usage
    exit 1
fi

echo "[1/2] Starting MediaMTX..."

$MEDIA_MTX_BIN &
MEDIAMTX_PID=$!

sleep 2

echo "[2/2] Starting RTSP publisher..."

if [[ "$MODE" == "webcam" ]]; then

    echo "Mode: webcam"
    echo "Device: $VIDEO_DEVICE"

    ffmpeg \
        -f v4l2 \
        -framerate "$FPS" \
        -video_size "${WIDTH}x${HEIGHT}" \
        -i "$VIDEO_DEVICE" \
        -c:v libx264 \
        -preset ultrafast \
        -tune zerolatency \
        -pix_fmt yuv420p \
        -f rtsp \
        -rtsp_transport tcp \
        "$RTSP_URL" &

    STREAM_PID=$!


elif [[ "$MODE" == "file" ]]; then

    INPUT_FILE="${2:-}"

    if [[ -z "$INPUT_FILE" ]]; then
        echo
        echo "Error: missing video file"
        show_usage
        exit 1
    fi

    if [[ ! -f "$INPUT_FILE" ]]; then
        echo
        echo "Error: file not found: $INPUT_FILE"
        exit 1
    fi

    echo "Mode: file"
    echo "Input: $INPUT_FILE"

    ffmpeg \
        -re \
        -stream_loop -1 \
        -i "$INPUT_FILE" \
        -c:v libx264 \
        -preset ultrafast \
        -tune zerolatency \
        -pix_fmt yuv420p \
        -f rtsp \
        -rtsp_transport tcp \
        "$RTSP_URL" &

    STREAM_PID=$!

else

    echo
    echo "Error: unknown mode: $MODE"
    show_usage
    exit 1

fi

echo
echo "RTSP stream running:"
echo "  $RTSP_URL"
echo
echo "Test with:"
echo "  ffplay $RTSP_URL"
echo

wait