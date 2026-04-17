#!/usr/bin/env fish

set OUTDIR "/home/truonglangquan/idk-code/idk-test/idk-bin"
mkdir -p "$OUTDIR"

set items \
  "idk-clock:firmware" \
  "idk-screensaver:m5stickc_plus2" \
  "idk-worldgen-2d:m5stickc_plus2" \
  "idk-worldgen-3d:m5stickc_plus2" \
  "idk-video:firmware" \
  "idk-geogen-2d:m5stickc_plus2" \
  "idk-geogen-multidim:m5stickc_plus2"

for item in $items
    set proj (string split ":" $item)[1]
    set env  (string split ":" $item)[2]

    echo "===== BUILD $proj ($env) ====="
    cd "/home/truonglangquan/idk-code/idk-test/$proj"
    or exit 1

    pio run -e "$env"
    or exit 1

    set builddir "/home/truonglangquan/idk-code/idk-test/$proj/.pio/build/$env"
    set out "$OUTDIR/$proj.bin"

    echo "===== MERGE $proj ====="
    esptool.py --chip esp32 merge_bin -o "$out" \
        0x1000 "$builddir/bootloader.bin" \
        0x8000 "$builddir/partitions.bin" \
        0x10000 "$builddir/firmware.bin"
    or exit 1
end

echo "Done. Files are in $OUTDIR"
