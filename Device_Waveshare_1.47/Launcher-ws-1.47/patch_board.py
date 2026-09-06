import re

ini_path = "/home/truonglangquan/idk-code/idk-test/Launcher-ws-1.47/boards/waveshare-esp32-s3-lcd-147/platformio.ini"
with open(ini_path, "r") as f:
    ini = f.read()

# Change TFT_BL from 48 to 46
ini = re.sub(r'-DTFT_BL=48', '-DTFT_BL=46', ini)

# Add definitions for 5-way switch K1
ini = re.sub(r'-DHAS_1_BUTTON=1', '-DHAS_3_BUTTONS=1\n\t-DHAS_UP_DOWN_BUTTONS=1\n\t-DPIN_BTN_UP=0\n\t-DPIN_BTN_DOWN=2\n\t-DPIN_BTN_LEFT=3\n\t-DPIN_BTN_RIGHT=4\n\t-DPIN_BTN_PRESS=5', ini)

with open(ini_path, "w") as f:
    f.write(ini)
