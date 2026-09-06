path = "/home/truonglangquan/idk-code/idk-test/Launcher-ws-1.47/boards/waveshare-esp32-s3-lcd-147/interface.cpp"
with open(path, "r") as f:
    content = f.read()

content = content.replace("if (isUp)    PrevPress = true;", "if (isUp)    UpPress = true;")
content = content.replace("if (isDown)  NextPress = true;", "if (isDown)  DownPress = true;")
content = content.replace("if (isLeft)  EscPress = true;", "if (isLeft)  PrevPress = true;")
content = content.replace("if (isRight) SelPress = true;", "if (isRight) NextPress = true;")

with open(path, "w") as f:
    f.write(content)
