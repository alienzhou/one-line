#!/usr/bin/env python3
"""Generate reproducible lv_font_conv 1.5.3 subsets for every complete record."""
import argparse
import hashlib
import re
import subprocess
from pack_fortunes import ROOT, load_records


def inventory():
    document, records = load_records(ROOT / 'assets/fortune/corpus.json')
    _, legacy = load_records(ROOT / 'assets/fortune/legacy-corpus.json')
    _, previous = load_records(ROOT / 'assets/fortune/previous-corpus.json')
    chars = set(range(32, 127))
    for _, _, text in records + legacy + previous:
        chars.update(map(ord, text))
    for citation in document.get('citations', {}).values():
        chars.update(map(ord, citation['display']))
    for path in (ROOT / 'main').glob('fortune_*.[ch]'):
        if path.name in ('fortune_data.c', 'fortune_data.h'):
            continue
        for string in re.findall(r'"([^"\\]*(?:\\.[^"\\]*)*)"', path.read_text()):
            chars.update(ord(c) for c in string if ord(c) > 127)
    for string in re.findall(r'"([^"\\]*(?:\\.[^"\\]*)*)"', (ROOT/'main/main.c').read_text()):
        chars.update(ord(c) for c in string if ord(c) > 127)
    return sorted(chars)


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--converter', required=True)
    args = p.parse_args()
    if subprocess.check_output([args.converter, '--version'], text=True).strip() != '1.5.3':
        raise SystemExit('Require lv_font_conv 1.5.3')
    chars = inventory()
    font_dir = ROOT / 'assets/fonts'
    (font_dir / 'fortune-characters.txt').write_text(''.join(map(chr, chars)) + '\n')
    header = '#pragma once\n#include <stdint.h>\n'
    header += f'#define FORTUNE_CHARACTER_COUNT {len(chars)}U\n'
    header += 'static const uint32_t FORTUNE_CHARACTERS[] = {\n'
    header += ''.join('    '+','.join(str(v) for v in chars[i:i+16])+',\n' for i in range(0,len(chars),16))+'};\n'
    (font_dir / 'fortune_characters.h').write_text(header)
    for size in (12, 20):
        output = font_dir / f'fortune_font_{size}.c'
        subprocess.run([args.converter, '--font', 'assets/fonts/NotoSansCJKsc-Regular.otf',
                        '--range', ','.join(f'0x{c:X}' for c in chars), '--size', str(size),
                        '--bpp','4','--format','lvgl','--no-compress','--no-kerning',
                        '--lv-font-name',f'fortune_font_{size}','--lv-include','lvgl.h',
                        '--output',str(output.relative_to(ROOT))], cwd=ROOT, check=True)
        output.write_text(output.read_text().rstrip() + '\n')
    print(f'{len(chars)} printable glyphs at 12/20 px; source SHA256 '+hashlib.sha256(
        (font_dir / 'NotoSansCJKsc-Regular.otf').read_bytes()).hexdigest())


if __name__ == '__main__':
    main()
