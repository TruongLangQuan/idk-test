#!/usr/bin/env bash
set -euo pipefail

IN_DEFAULT="/home/truonglangquan/Videos/ellen.mp4"
OUT_DEFAULT="../data/video.mjpg"

IN="${1:-$IN_DEFAULT}"
OUT="${2:-$OUT_DEFAULT}"
DURATION="${DURATION:-}"
FPS="${FPS:-10}"
QUALITY="${QUALITY:-15}"

if [ -n "$DURATION" ]; then
  DUR_ARGS=(-t "$DURATION")
else
  DUR_ARGS=()
fi

ffmpeg -y -ss 0 "${DUR_ARGS[@]}" -i "$IN" \
  -vf "scale=240:135" \
  -r "$FPS" -q:v "$QUALITY" -an -f mjpeg "$OUT"

echo "Wrote $OUT"
