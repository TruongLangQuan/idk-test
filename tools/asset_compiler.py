#!/usr/bin/env python3
"""Compile image/txt/gif/ir assets into built-in C++ headers for idk-test firmware."""

from __future__ import annotations

import argparse
import math
import re
from dataclasses import dataclass, field
from pathlib import Path
from typing import Iterable, List, Sequence, Tuple

from PIL import Image, ImageOps, ImageSequence

TARGET_W = 240
TARGET_H = 135
GIF_MAX_ASSET_BYTES = 700_000


@dataclass
class Paths:
    repo_root: Path
    data_root: Path
    idk_test_root: Path


@dataclass
class IrCommandData:
    name: str = ""
    type: str = "parsed"
    protocol: str = ""
    address: int = 0
    command: int = 0
    frequency: int = 38000
    duty_cycle: float = 0.33
    raw_data: List[int] = field(default_factory=list)


@dataclass
class IrFileData:
    name: str
    commands: List[IrCommandData]


def sanitize_identifier(name: str) -> str:
    ident = re.sub(r"[^a-zA-Z0-9_]", "_", name)
    if ident and ident[0].isdigit():
        ident = f"_{ident}"
    return ident.upper()


def c_escape(text: str) -> str:
    return (
        text.replace("\\", "\\\\")
        .replace("\"", "\\\"")
        .replace("\n", "\\n")
        .replace("\r", "\\r")
        .replace("\t", "\\t")
    )


def to_rgb565_bytes(img: Image.Image) -> bytes:
    rgb = img.convert("RGB")
    out = bytearray()
    raw = rgb.tobytes()
    for i in range(0, len(raw), 3):
        r = raw[i]
        g = raw[i + 1]
        b = raw[i + 2]
        v = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)
        out.append((v >> 8) & 0xFF)
        out.append(v & 0xFF)
    return bytes(out)


def fit_image(img: Image.Image, w: int, h: int, mode: str, bg: Tuple[int, int, int]) -> Image.Image:
    if mode == "stretch":
        return img.resize((w, h), Image.Resampling.LANCZOS)
    if mode == "cover":
        return ImageOps.fit(img, (w, h), Image.Resampling.LANCZOS)

    scaled = ImageOps.contain(img, (w, h), Image.Resampling.LANCZOS)
    canvas = Image.new("RGB", (w, h), bg)
    x = (w - scaled.width) // 2
    y = (h - scaled.height) // 2
    canvas.paste(scaled, (x, y))
    return canvas


def as_c_array(data: bytes, per_line: int = 16) -> str:
    chunks = []
    for i in range(0, len(data), per_line):
        line = ", ".join(f"0x{b:02X}" for b in data[i : i + per_line])
        chunks.append(f"  {line}")
    return "{\n" + ",\n".join(chunks) + "\n}"


def as_u16_array(data: List[int], per_line: int = 14) -> str:
    chunks = []
    for i in range(0, len(data), per_line):
        line = ", ".join(str(v) for v in data[i : i + per_line])
        chunks.append(f"  {line}")
    return "{\n" + ",\n".join(chunks) + "\n}"


def list_files(folder: Path, exts: Sequence[str]) -> List[Path]:
    if not folder.exists():
        return []
    norm = {e.lower() for e in exts}
    return sorted([p for p in folder.iterdir() if p.is_file() and p.suffix.lower() in norm])


def write_text(path: Path, content: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(content, encoding="utf-8")


def parse_hex_bytes(value: str) -> int:
    parts = [p for p in value.strip().split() if p]
    out = 0
    for i, token in enumerate(parts):
        out |= (int(token, 16) & 0xFF) << (8 * i)
    return out


def parse_ir_file(path: Path, display_name: str | None = None) -> IrFileData:
    commands: List[IrCommandData] = []
    cur: IrCommandData | None = None

    def flush() -> None:
        nonlocal cur
        if cur and cur.name:
            commands.append(cur)
        cur = None

    for raw_line in path.read_text(encoding="utf-8", errors="replace").splitlines():
        line = raw_line.strip()
        if not line:
            continue
        if line.startswith("Filetype:") or line.startswith("Version:"):
            continue
        if line.startswith("#"):
            continue
        if ":" not in line:
            continue

        key, value = line.split(":", 1)
        key = key.strip().lower()
        value = value.strip()

        if key == "name":
            flush()
            cur = IrCommandData(name=value)
            continue

        if cur is None:
            continue

        if key == "type":
            cur.type = value.lower()
        elif key == "protocol":
            cur.protocol = value
        elif key == "address":
            cur.address = parse_hex_bytes(value)
        elif key == "command":
            cur.command = parse_hex_bytes(value)
        elif key == "frequency":
            try:
                cur.frequency = int(float(value))
            except ValueError:
                cur.frequency = 38000
        elif key == "duty_cycle":
            try:
                cur.duty_cycle = float(value)
            except ValueError:
                cur.duty_cycle = 0.33
        elif key == "data":
            nums = []
            for token in value.split():
                try:
                    n = int(token)
                except ValueError:
                    continue
                nums.append(max(0, min(65535, n)))
            cur.raw_data = nums

    flush()
    return IrFileData(name=display_name or path.name, commands=commands)


def parse_ir_tree_manifest(manifest: Path, src_dir: Path) -> List[Path]:
    """Parse `tree`-style text and return ordered `.ir` files under src_dir.

    Supports lines like:
      │   ├── A/B/C.ir
      │   └── Folder
    """
    if not manifest.exists():
        return []

    lines = manifest.read_text(encoding="utf-8", errors="replace").splitlines()
    stack: List[str] = []
    ordered: List[Path] = []
    seen: set[Path] = set()

    entry_re = re.compile(r"^(?P<prefix>[│ ]*)(?:├──|└──)\s(?P<name>.+)$")

    for raw in lines:
        m = entry_re.match(raw)
        if not m:
            continue
        prefix = m.group("prefix")
        name = m.group("name").strip()
        if not name:
            continue

        depth = len(prefix.replace("│", " ").expandtabs(4)) // 4
        if len(stack) <= depth:
            stack.extend([""] * (depth - len(stack) + 1))

        if name.lower().endswith(".ir"):
            rel_parts = [p for p in stack[:depth] if p] + [name]
            rel = Path(*rel_parts)
            full = (src_dir / rel).resolve()
            if full.exists() and full.is_file() and full.suffix.lower() == ".ir" and full not in seen:
                seen.add(full)
                ordered.append(full)
        else:
            stack[depth] = name
            del stack[depth + 1 :]

    return ordered


def compile_images(
    src_dir: Path,
    output_headers: Iterable[Path],
    export_dir: Path,
    export_format: str,
    fit_mode: str,
    rotate_left: bool,
    bg_color: Tuple[int, int, int],
) -> int:
    files = list_files(src_dir, [".png", ".jpg", ".jpeg", ".bmp", ".webp"])
    export_dir.mkdir(parents=True, exist_ok=True)

    blocks = []
    table_entries = []

    for p in files:
        ident = sanitize_identifier(p.stem)
        img = Image.open(p)
        if rotate_left:
            img = img.transpose(Image.Transpose.ROTATE_90)
        fitted = fit_image(img, TARGET_W, TARGET_H, fit_mode, bg_color)

        out_img = export_dir / f"{p.stem}.{export_format}"
        fitted.save(out_img, format=export_format.upper())

        data = to_rgb565_bytes(fitted)
        arr_name = f"IMG_{ident}_RGB565"
        blocks.append(f"static const uint8_t {arr_name}[] PROGMEM = {as_c_array(data)};\n")
        table_entries.append(
            f'  {{"{p.name}", {TARGET_W}, {TARGET_H}, {arr_name}, sizeof({arr_name})}}'
        )

    if not table_entries:
        table_blob = ""
        count_expr = "0"
    else:
        table_blob = "\n".join(table_entries)
        count_expr = "sizeof(IMAGE_TABLE) / sizeof(IMAGE_TABLE[0])"

    header = f"""#pragma once
#include <stdint.h>
#include <stddef.h>

#ifndef PROGMEM
#define PROGMEM
#endif

struct BuiltinImage {{
  const char* name;
  uint16_t width;
  uint16_t height;
  const uint8_t* rgb565;
  size_t bytes;
}};

{'\n'.join(blocks)}
static const BuiltinImage IMAGE_TABLE[] = {{
{",\n".join(table_entries)}
}};

static constexpr size_t IMAGE_COUNT = {count_expr};
"""

    for h in output_headers:
        write_text(h, header)

    return len(files)


def compile_gif(
    src_dir: Path,
    output_headers: Iterable[Path],
    fit_mode: str,
    rotate_left: bool,
    bg_color: Tuple[int, int, int],
) -> int:
    files = list_files(src_dir, [".gif"])
    if not files:
        empty = """#pragma once
#include <stdint.h>

#ifndef PROGMEM
#define PROGMEM
#endif

struct GifFrame {
  const uint8_t* rgb565;
  uint32_t delay_ms;
};

struct GifAsset {
  const char* name;
  const GifFrame* frames;
  uint16_t frame_count;
};

static constexpr uint16_t GIF_WIDTH = 240;
static constexpr uint16_t GIF_HEIGHT = 135;
static constexpr uint16_t GIF_ASSET_COUNT = 0;
static constexpr GifFrame GIF_EMPTY_FRAMES[] = {};
static constexpr GifAsset GIF_ASSETS[] = {};
static const uint16_t GIF_FRAME_COUNT = 0;
static const GifFrame* GIF_FRAMES = GIF_EMPTY_FRAMES;
"""
        for h in output_headers:
            write_text(h, empty)
        return 0

    frame_bytes = TARGET_W * TARGET_H * 2
    max_total_frames = max(1, GIF_MAX_ASSET_BYTES // frame_bytes)
    max_per_gif = max(1, max_total_frames // len(files))

    frame_blocks: List[str] = []
    gif_blocks: List[str] = []
    asset_entries: List[str] = []
    total_out_frames = 0

    for gif_idx, gif_path in enumerate(files):
        gif = Image.open(gif_path)
        all_frames = [f.copy() for f in ImageSequence.Iterator(gif)]
        total_frames = len(all_frames)
        stride = max(1, math.ceil(total_frames / max_per_gif))

        gif_frame_entries: List[str] = []
        out_idx = 0
        for src_idx in range(0, total_frames, stride):
            frame = all_frames[src_idx]
            base = frame.convert("RGB")
            if rotate_left:
                base = base.transpose(Image.Transpose.ROTATE_90)
            fitted = fit_image(base, TARGET_W, TARGET_H, fit_mode, bg_color)

            data = to_rgb565_bytes(fitted)
            arr_name = f"GIF_{gif_idx}_FRAME_{out_idx}_RGB565"
            delay_sum = 0
            chunk_len = min(src_idx + stride, total_frames) - src_idx
            for i in range(src_idx, min(src_idx + stride, total_frames)):
                info = all_frames[i].info
                delay_sum += int(info.get("duration", gif.info.get("duration", 100)))
            if delay_sum <= 0:
                delay_sum = 100 * max(1, chunk_len)

            # Keep non-ellen GIF timing unchanged (sum delay of skipped source frames).
            if gif_path.name.lower() == "ellen-joe.gif":
                avg_delay = delay_sum / max(1, chunk_len)
                delay = max(90, min(260, int(avg_delay * 1.6)))
            else:
                delay = max(1, int(delay_sum))

            frame_blocks.append(f"static const uint8_t {arr_name}[] PROGMEM = {as_c_array(data)};\n")
            gif_frame_entries.append(f"  {{{arr_name}, {delay}}}")
            out_idx += 1

        total_out_frames += out_idx
        frame_list_name = f"GIF_{gif_idx}_FRAMES"
        gif_blocks.append(
            f"static const GifFrame {frame_list_name}[] = {{\n"
            + ",\n".join(gif_frame_entries)
            + "\n};\n"
        )
        asset_entries.append(
            f"  {{\"{c_escape(gif_path.name)}\", {frame_list_name}, {out_idx}}}"
        )

    header = f"""#pragma once
#include <stdint.h>

#ifndef PROGMEM
#define PROGMEM
#endif

struct GifFrame {{
  const uint8_t* rgb565;
  uint32_t delay_ms;
}};

struct GifAsset {{
  const char* name;
  const GifFrame* frames;
  uint16_t frame_count;
}};

{'\n'.join(frame_blocks)}
{'\n'.join(gif_blocks)}

static constexpr uint16_t GIF_WIDTH = {TARGET_W};
static constexpr uint16_t GIF_HEIGHT = {TARGET_H};
static constexpr uint16_t GIF_ASSET_COUNT = {len(asset_entries)};
static const GifAsset GIF_ASSETS[] = {{
{",\n".join(asset_entries)}
}};
static const uint16_t GIF_FRAME_COUNT = GIF_ASSET_COUNT ? GIF_ASSETS[0].frame_count : 0;
static const GifFrame* GIF_FRAMES = GIF_ASSET_COUNT ? GIF_ASSETS[0].frames : nullptr;
"""

    for h in output_headers:
        write_text(h, header)
    return total_out_frames


def wrap_text_lines(content: str, max_chars: int) -> List[str]:
    lines: List[str] = []
    for raw in content.splitlines():
        raw = raw.rstrip()
        if not raw:
            lines.append("")
            continue
        words = raw.split(" ")
        cur = ""
        for w in words:
            candidate = w if not cur else f"{cur} {w}"
            if len(candidate) <= max_chars:
                cur = candidate
            else:
                if cur:
                    lines.append(cur)
                cur = w
        if cur:
            lines.append(cur)
    return lines


def compile_txt(src_dir: Path, output_header: Path, max_chars: int = 34) -> int:
    files = list_files(src_dir, [".txt"])
    if not files:
        empty = """#pragma once
#include <stddef.h>

static constexpr const char* TXT_LINES[] = {
  "No TXT compiled yet.",
};
static constexpr size_t TXT_LINE_COUNT = sizeof(TXT_LINES) / sizeof(TXT_LINES[0]);
"""
        write_text(output_header, empty)
        return 0

    txt_path = files[0]
    content = txt_path.read_text(encoding="utf-8", errors="replace")
    lines = wrap_text_lines(content, max_chars=max_chars)

    entries = []
    for line in lines:
        esc = line.replace("\\", "\\\\").replace('"', '\\"')
        entries.append(f'  "{esc}",')

    header = f"""#pragma once
#include <stddef.h>

// UTF-8 lines generated from: {txt_path.name}
static constexpr const char* TXT_LINES[] = {{
{chr(10).join(entries)}
}};
static constexpr size_t TXT_LINE_COUNT = sizeof(TXT_LINES) / sizeof(TXT_LINES[0]);
"""

    write_text(output_header, header)
    return len(lines)



def compile_ir(src_dir: Path, output_header: Path) -> Tuple[int, int]:
    # Prefer the user-provided tree manifest order, fallback to recursive scan.
    manifest = src_dir.parent.parent / "ir.txt"
    files = parse_ir_tree_manifest(manifest, src_dir)
    if not files:
        files = sorted([p for p in src_dir.rglob("*") if p.is_file() and p.suffix.lower() == ".ir"])
    if not files:
        empty = """#pragma once
#include <stddef.h>
#include <stdint.h>

struct IrCommand {
  const char* name;
  const char* type;
  const char* protocol;
  uint32_t address;
  uint32_t command;
  uint32_t frequency;
  float duty_cycle;
  const uint16_t* raw_data;
  size_t raw_len;
};

struct IrFile {
  const char* name;
  const IrCommand* commands;
  size_t command_count;
};

static constexpr IrFile IR_FILES[] = {};
static constexpr size_t IR_FILE_COUNT = 0;
"""
        write_text(output_header, empty)
        return 0, 0

    # Keep output within practical firmware size.
    max_files = 120
    max_cmds = 2500
    max_raw_items = 220_000

    parsed: List[IrFileData] = []
    cmd_budget = 0
    raw_budget = 0
    for p in files:
        if len(parsed) >= max_files:
            break
        rel_name = str(p.relative_to(src_dir)).replace("\\", "/")
        pf = parse_ir_file(p, display_name=rel_name)
        if not pf.commands:
            continue
        next_cmds = len(pf.commands)
        next_raw = sum(len(c.raw_data) for c in pf.commands if c.raw_data)
        if cmd_budget + next_cmds > max_cmds:
            break
        if raw_budget + next_raw > max_raw_items:
            break
        parsed.append(pf)
        cmd_budget += next_cmds
        raw_budget += next_raw

    if not parsed:
        write_text(
            output_header,
            "#pragma once\n#include <stddef.h>\n#include <stdint.h>\n"
            "struct IrCommand{const char* name;const char* type;const char* protocol;uint32_t address;"
            "uint32_t command;uint32_t frequency;float duty_cycle;const uint16_t* raw_data;size_t raw_len;};\n"
            "struct IrFile{const char* name;const IrCommand* commands;size_t command_count;};\n"
            "static constexpr IrFile IR_FILES[] = {};\nstatic constexpr size_t IR_FILE_COUNT = 0;\n",
        )
        return 0, 0
    file_blocks: List[str] = []
    file_entries: List[str] = []
    cmd_count = 0

    for fi, ir_file in enumerate(parsed):
        cmd_entries: List[str] = []
        for ci, cmd in enumerate(ir_file.commands):
            cmd_count += 1
            cmd_id = sanitize_identifier(f"{ir_file.name}_{ci}_{cmd.name}")
            raw_name = f"IR_RAW_{cmd_id}"
            if cmd.raw_data:
                file_blocks.append(
                    f"static const uint16_t {raw_name}[] PROGMEM = {as_u16_array(cmd.raw_data)};\n"
                )
                raw_ref = raw_name
                raw_len = str(len(cmd.raw_data))
            else:
                raw_ref = "nullptr"
                raw_len = "0"

            esc_name = cmd.name.replace("\\", "\\\\").replace('"', '\\"')
            esc_type = cmd.type.replace("\\", "\\\\").replace('"', '\\"')
            esc_proto = cmd.protocol.replace("\\", "\\\\").replace('"', '\\"')
            cmd_entries.append(
                f'  {{"{esc_name}", "{esc_type}", "{esc_proto}", '
                f"0x{cmd.address:08X}u, 0x{cmd.command:08X}u, {cmd.frequency}u, "
                f"{cmd.duty_cycle:.6f}f, {raw_ref}, {raw_len}}},"
            )

        arr_name = f"IR_FILE_{fi}_CMDS"
        file_blocks.append(
            f"static const IrCommand {arr_name}[] PROGMEM = {{\n{chr(10).join(cmd_entries)}\n}};\n"
        )
        esc_file_name = ir_file.name.replace("\\", "\\\\").replace('"', '\\"')
        file_entries.append(
            f'  {{"{esc_file_name}", {arr_name}, sizeof({arr_name}) / sizeof({arr_name}[0])}}'
        )

    header = f"""#pragma once
#include <stddef.h>
#include <stdint.h>

#ifndef PROGMEM
#define PROGMEM
#endif

struct IrCommand {{
  const char* name;
  const char* type;
  const char* protocol;
  uint32_t address;
  uint32_t command;
  uint32_t frequency;
  float duty_cycle;
  const uint16_t* raw_data;
  size_t raw_len;
}};

struct IrFile {{
  const char* name;
  const IrCommand* commands;
  size_t command_count;
}};

{chr(10).join(file_blocks)}
static const IrFile IR_FILES[] = {{
{",\n".join(file_entries)}
}};
static constexpr size_t IR_FILE_COUNT = sizeof(IR_FILES) / sizeof(IR_FILES[0]);
"""

    write_text(output_header, header)
    return len(parsed), cmd_count


def _portal_html_is_safe(html: str) -> bool:
    # Heuristic safety gate: refuse obvious credential capture pages.
    # This keeps idk-portal templates limited to non-sensitive forms.
    lowered = html.lower()
    if 'type="password"' in lowered or "type='password'" in lowered:
        return False
    for bad in ("name=\"password\"", "name='password'", "name=\"email\"", "name='email'", "gmail"):
        if bad in lowered:
            return False
    return True


def compile_portal_html(src_dir: Path, output_header: Path) -> int:
    files = list_files(src_dir, [".html", ".htm"])
    if not files:
        empty = """#pragma once
#include <stddef.h>
#include <stdint.h>

#ifndef PROGMEM
#define PROGMEM
#endif

struct PortalPage {
  const char* name;
  const char* html;
  size_t bytes;
};

static constexpr PortalPage PORTAL_PAGES[] = {};
static constexpr size_t PORTAL_PAGE_COUNT = 0;
"""
        write_text(output_header, empty)
        return 0

    blocks: List[str] = []
    entries: List[str] = []
    kept = 0

    for p in files:
        html = p.read_text(encoding="utf-8", errors="replace")
        if not _portal_html_is_safe(html):
            # Skip unsafe templates rather than embedding them.
            continue

        kept += 1
        ident = sanitize_identifier(p.stem)
        var_name = f"PORTAL_HTML_{ident}"
        # Raw literal keeps HTML readable and avoids escaping.
        blocks.append(f'static const char {var_name}[] PROGMEM = R\"rawliteral({html})rawliteral\";\n')
        entries.append(f'  {{"{p.name}", {var_name}, sizeof({var_name}) - 1}}')

    header = f"""#pragma once
#include <stddef.h>
#include <stdint.h>

#ifndef PROGMEM
#define PROGMEM
#endif

struct PortalPage {{
  const char* name;
  const char* html;
  size_t bytes;
}};

{chr(10).join(blocks)}
static constexpr PortalPage PORTAL_PAGES[] = {{
{",\n".join(entries)}
}};
static constexpr size_t PORTAL_PAGE_COUNT = sizeof(PORTAL_PAGES) / sizeof(PORTAL_PAGES[0]);
"""

    write_text(output_header, header)
    return kept


def resolve_paths(script_path: Path) -> Paths:
    idk_test_root = script_path.resolve().parent.parent
    repo_root = idk_test_root.parent
    data_root = repo_root / "data"
    return Paths(repo_root=repo_root, data_root=data_root, idk_test_root=idk_test_root)


def parse_color(s: str) -> Tuple[int, int, int]:
    parts = [p.strip() for p in s.split(",")]
    if len(parts) != 3:
        raise ValueError("bg color must be 'R,G,B'")
    vals = tuple(max(0, min(255, int(p))) for p in parts)
    return vals  # type: ignore[return-value]


def build_parser() -> argparse.ArgumentParser:
    p = argparse.ArgumentParser(description="Compile data assets into built-in C++ headers")
    p.add_argument("command", choices=["all", "img", "gif", "txt", "ir", "portalhtml"], help="task to run")
    p.add_argument("--fit", choices=["contain", "cover", "stretch"], default="contain")
    p.add_argument("--rotate-left", action="store_true", default=False)
    p.add_argument("--bg", default="0,0,0", help="background color for contain mode, e.g. 0,0,0")
    p.add_argument("--export-format", choices=["png", "bmp"], default="png")
    return p


def main() -> int:
    args = build_parser().parse_args()
    paths = resolve_paths(Path(__file__))
    bg = parse_color(args.bg)

    img_count = gif_frames = txt_lines = ir_files = ir_cmds = -1
    portal_pages = -1

    if args.command in ("all", "img"):
        img_count = compile_images(
            src_dir=paths.data_root / "img",
            output_headers=[
                paths.idk_test_root / "idk-img/include/generated/img_assets.h",
                paths.idk_test_root / "idk-cyd/include/generated/cyd_img_assets.h",
            ],
            export_dir=paths.idk_test_root / "tools/out/normalized_img",
            export_format=args.export_format,
            fit_mode=args.fit,
            rotate_left=args.rotate_left,
            bg_color=bg,
        )

    if args.command in ("all", "gif"):
        gif_frames = compile_gif(
            src_dir=paths.data_root / "gifs",
            output_headers=[
                paths.idk_test_root / "idk-gif/include/generated/gif_asset.h",
                paths.idk_test_root / "idk-cyd/include/generated/cyd_gif_asset.h",
            ],
            fit_mode=args.fit,
            rotate_left=args.rotate_left,
            bg_color=bg,
        )

    if args.command in ("all", "txt"):
        txt_lines = compile_txt(
            src_dir=paths.data_root / "txt",
            output_header=paths.idk_test_root / "idk-txt/include/generated/txt_asset.h",
        )

    if args.command in ("all", "ir"):
        ir_files, ir_cmds = compile_ir(
            src_dir=paths.data_root / "ir",
            output_header=paths.idk_test_root / "idk-ir/include/generated/ir_assets.h",
        )

    if args.command in ("all", "portalhtml"):
        portal_pages = compile_portal_html(
            src_dir=paths.data_root / "portal_html",
            output_header=paths.idk_test_root / "idk-portal/include/generated/portal_pages.h",
        )

    print("Asset compilation done")
    if img_count >= 0:
        print(f"- Images compiled: {img_count}")
        print(f"- Normalized image exports: {paths.idk_test_root / 'tools/out/normalized_img'}")
    if gif_frames >= 0:
        print(f"- GIF frames compiled: {gif_frames}")
    if txt_lines >= 0:
        print(f"- TXT wrapped lines: {txt_lines}")
    if ir_files >= 0:
        print(f"- IR files compiled: {ir_files}")
        print(f"- IR commands compiled: {ir_cmds}")
    if portal_pages >= 0:
        print(f"- Portal HTML templates compiled: {portal_pages}")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
