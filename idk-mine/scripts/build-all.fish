#!/usr/bin/env fish
set ROOT (cd (dirname (status -f))/..; pwd)
set PROJECTS \
  cyd/idk-cyd-lottery \
  cyd/idk-cyd-mine \
  esp32-wroom-32u/idk-esp32-lottery \
  esp32-wroom-32u/idk-esp32-mine

for p in $PROJECTS
  echo "==> Building $p"
  pio run --project-dir "$ROOT/$p"
  or begin
    echo "Build failed: $p"
    exit 1
  end
end

echo "All builds completed"
