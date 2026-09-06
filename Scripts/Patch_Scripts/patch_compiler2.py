import re

with open("tools/asset_compiler.py", "r") as f:
    code = f.read()

# Wrap parse_ir_file in try/except
replacement = """
        try:
            pf = parse_ir_file(p, display_name=rel_name)
        except Exception as e:
            print(f"Skipping {p}: {e}")
            continue
"""

code = code.replace("        pf = parse_ir_file(p, display_name=rel_name)", replacement)

with open("tools/asset_compiler.py", "w") as f:
    f.write(code)

