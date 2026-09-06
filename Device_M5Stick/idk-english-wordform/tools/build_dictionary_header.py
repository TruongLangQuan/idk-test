#!/usr/bin/env python3
from pathlib import Path
import json
import re

OED = Path('/home/truonglangquan/Downloads/Oxford English Dictionary.txt')
ENGLISH_WORDS = Path('/home/truonglangquan/english-words')
OUT = Path(__file__).resolve().parents[1] / 'src' / 'generated_word_hashes.h'

WORD_RE = re.compile(r"[A-Za-z][A-Za-z'-]*")


def normalize(word: str) -> str:
    w = word.strip().lower()
    w = w.replace('’', "'")
    if not w:
        return ''
    if not all(ch.isalpha() or ch in "'-" for ch in w):
        return ''
    return w


def fnv1a(text: str) -> int:
    h = 2166136261
    for b in text.encode('utf-8'):
        h ^= b
        h = (h * 16777619) & 0xFFFFFFFF
    return h


def collect_from_oed(path: Path):
    words = set()
    if not path.exists():
        return words
    for raw in path.read_text(encoding='utf-8', errors='ignore').splitlines():
        line = raw.strip()
        if not line:
            continue
        head = line.split('	', 1)[0].split('  ', 1)[0].split(' — ', 1)[0].split(' - ', 1)[0]
        m = WORD_RE.match(head)
        if not m:
            continue
        word = normalize(m.group(0))
        if word:
            words.add(word)
    return words


def collect_from_english_words(path: Path):
    words = set()
    txt = path / 'words_alpha.txt'
    if txt.exists():
        for raw in txt.read_text(encoding='utf-8', errors='ignore').splitlines():
            word = normalize(raw)
            if word:
                words.add(word)
    js = path / 'words_dictionary.json'
    if js.exists():
        data = json.loads(js.read_text(encoding='utf-8', errors='ignore'))
        for key in data.keys():
            word = normalize(key)
            if word:
                words.add(word)
    return words


def main():
    words = collect_from_oed(OED)
    if not words:
        words = collect_from_english_words(ENGLISH_WORDS)
    hashes = sorted({fnv1a(w) for w in words})
    lines = [
        '#pragma once',
        '#include <stdint.h>',
        '',
        'static const uint32_t kWordHashes[] = {'
    ]
    for i in range(0, len(hashes), 8):
        chunk = ', '.join(f'0x{h:08X}u' for h in hashes[i:i+8])
        lines.append(f'    {chunk},')
    lines += [
        '};',
        '',
        f'static const size_t kWordHashCount = {len(hashes)};',
        ''
    ]
    OUT.write_text('\n'.join(lines), encoding='utf-8')
    print(f'Wrote {OUT} with {len(words)} words / {len(hashes)} hashes')


if __name__ == '__main__':
    main()
