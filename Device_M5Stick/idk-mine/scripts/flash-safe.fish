#!/usr/bin/env fish
set ROOT (cd (dirname (status -f))/..; pwd)

if test (count $argv) -lt 2
  echo "Usage: ./scripts/flash-safe.fish <project> <port>"
  echo "Projects: cyd-lottery | cyd-mine | esp32-lottery | esp32-mine"
  exit 1
end

set key $argv[1]
set port $argv[2]

switch $key
  case cyd-lottery
    set rel cyd/idk-cyd-lottery
  case cyd-mine
    set rel cyd/idk-cyd-mine
  case esp32-lottery
    set rel esp32-wroom-32u/idk-esp32-lottery
  case esp32-mine
    set rel esp32-wroom-32u/idk-esp32-mine
  case '*'
    echo "Unknown project key: $key"
    exit 1
end

set project_dir "$ROOT/$rel"
set base_ini "$project_dir/platformio.ini"

if not test -f "$base_ini"
  echo "Missing $base_ini"
  exit 1
end

set speeds 460800 230400 115200

for speed in $speeds
  echo "==> Trying upload_speed=$speed"
  set temp_ini (mktemp)
  sed -E "s/^upload_speed[[:space:]]*=.*/upload_speed = $speed/" "$base_ini" > "$temp_ini"

  pio run --project-dir "$project_dir" -c "$temp_ini" -t upload --upload-port "$port"
  set rc $status
  rm -f "$temp_ini"

  if test $rc -eq 0
    echo "Upload succeeded at $speed"
    exit 0
  end

  echo "Upload failed at $speed"
end

echo "All retry speeds failed"
exit 1
