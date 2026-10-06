#!/usr/bin/env python3
"""Package a portrait release panel and README previews from completed letter captures."""
from PIL import Image
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
    for story in range(4):
        with Image.open(preview / f'letters-preview-{story}.ppm') as card:
            if card.size != (240, 320):
                raise ValueError(f'Unexpected story capture: {story}')
            card.save(ROOT / f'assets/images/fortune-letter-preview-{story}.png', optimize=True)
    print(f'Continuous-letter panel: COMPLETE — final 1152x1536 PNG at {path}')
    print('README letter previews: COMPLETE — four final 240x320 PNGs from completed application renders.')


if __name__ == '__main__':
    main()
