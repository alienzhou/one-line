#!/usr/bin/env python3
"""Render a portrait release panel from completed continuous-letter UI captures."""
from render_fortune_release import ROOT, OUT, panel, capture, text


def main():
    preview = ROOT / 'build/fortune-preview'
    image, draw = panel('连续来信，喜欢就接着读', '四组原创故事 · 短句、像素配图与留白')
    names = ('letters-preview-0', 'letters-preview-1', 'letters-2-5', 'letters-3-20')
    for i, name in enumerate(names):
        capture(image, preview / (name + '.ppm'), 120 + (i % 2) * 612,
                264 + (i // 2) * 478, 300)
    text(draw, (48, 1210), '先看看：上键换一组，下键换肤，确定选定。', 32)
    text(draw, (48, 1280), '选定后：确定下一页，上键回看，下键仍换肤。', 32)
    text(draw, (48, 1350), '长按确定回首页；再选这组，继续上次那一页。', 30)
    text(draw, (48, 1420), '每组分别记住进度，随时停下，也随时回来。', 28)
    OUT.mkdir(parents=True, exist_ok=True)
    path = OUT / 'continuous-letters.png'
    image.save(path, optimize=True)
    print(f'Continuous-letter panel: COMPLETE — final 1152x1536 PNG at {path}')


if __name__ == '__main__':
    main()
