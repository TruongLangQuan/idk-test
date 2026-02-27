#!/usr/bin/env fish
set ROOT (cd (dirname (status -f))/..; pwd)

if test (count $argv) -lt 2
  echo "Usage: ./scripts/flash.fish <project> <port>"
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

pio run --project-dir "$ROOT/$rel" -t upload --upload-port "$port"
