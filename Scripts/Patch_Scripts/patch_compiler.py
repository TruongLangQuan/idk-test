import re

with open("tools/asset_compiler.py", "r") as f:
    code = f.read()

code = code.replace("max_files = 120", "max_files = 20000")
code = code.replace("max_cmds = 2500", "max_cmds = 500000")
code = code.replace("max_raw_items = 220_000", "max_raw_items = 50000000")

with open("tools/asset_compiler.py", "w") as f:
    f.write(code)

