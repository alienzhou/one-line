#!/usr/bin/env python3
"""Package actual host-rendered rare cards and their opening/ambient animation."""
from pathlib import Path
import re
from PIL import Image, ImageDraw, ImageFont

ROOT = Path(__file__).resolve().parents[1]


def main():
    source = ROOT / 'build/fortune-preview'
    output = ROOT / 'assets/images'
    font = str(ROOT / 'assets/fonts/NotoSansCJKsc-Regular.otf')
    count = int(re.search(r'FORTUNE_RARE_COUNT (\d+)U', (ROOT / 'main/fortune_art.h').read_text())[1])
    rules = (ROOT / 'main/fortune_rare.h').read_text()
    chance = int(re.search(r'FORTUNE_RARE_PERCENT (\d+)U', rules)[1])
    pity = int(re.search(r'FORTUNE_RARE_PITY (\d+)U', rules)[1])
    columns = 5 if count > 20 else 4
    rows = (count + columns - 1) // columns
    height = 106 + rows * 336 + 44
    poster = Image.new('RGB', (32 + columns * 256, height), '#0d1022')
    draw = ImageDraw.Draw(poster)
    draw.text((28, 17), f'一签 · {count} 封奇遇', font=ImageFont.truetype(font, 32), fill='#f2d299')
    draw.text((28, 65), f'首次十抽内必得 · 之后 {chance}% 概率 / {pity} 抽保底',
              font=ImageFont.truetype(font, 18), fill='#c4b69b')
    for i in range(count):
        with Image.open(source / f'rare-{i}-0.ppm') as card:
            assert card.size == (240, 320)
            poster.paste(card, (24 + i % columns * 256, 106 + i // columns * 336))
    draw.text((28, height - 34), '实际程序渲染 · 非设备照片 · 原版自动永久珍藏',
              font=ImageFont.truetype(font, 16), fill='#a9a38f')
    poster.save(output / 'fortune-rares.png', optimize=True)
    frames = [Image.open(source / f'rare-opening-{i:02}.ppm').convert('RGB') for i in range(10)]
    frames += [Image.open(source / f'rare-motion-0-{i:02}.ppm').convert('RGB') for i in range(16)]
    frames[0].save(output / 'fortune-rare-reveal.gif', save_all=True, append_images=frames[1:],
                   duration=[125] * 10 + [250] * 15 + [1200], loop=0)
    community = Image.new('RGB', (1152, 1536), '#0d1022')
    draw = ImageDraw.Draw(community)
    for pos, words, size, color in [
        ((48, 36), '一签 · ONE FORTUNE', 24, '#c4b69b'),
        ((48, 88), f'{count}封奇遇，偶尔拆到星光', 50, '#f2d299'),
        ((48, 172), '独立场景与原创短句 · 三种配色 · 发现即永久珍藏', 27, '#c4b69b'),
        ((48, 224), f'首次十抽内必得 · 之后{chance}%基础概率 / {pity}抽保底', 25, '#c4b69b'),
        ((48, 1450), '金色拆信与庆祝铃音 · 珍藏原版不占普通签册', 27, '#f2d299'),
        ((48, 1496), '实际程序主机渲染 · 非设备照片 / APPLICATION RENDERS', 21, '#a9a38f'),
    ]:
        draw.text(pos, words, font=ImageFont.truetype(font, size), fill=color)
    for i, rare in enumerate((0, 4, 7, 24)):
        with Image.open(source / f'rare-{rare}-0.ppm') as card:
            assert card.size == (240, 320)
            community.paste(card.resize((390, 520), Image.Resampling.NEAREST),
                            (92 + i % 2 * 578, 285 + i // 2 * 570))
    (output / 'community').mkdir(exist_ok=True)
    community.save(output / 'community/rare-encounters.png', optimize=True)
    print('Rare previews: all actual cards and opening/ambient animation packaged.')
    print('Community rare panel: COMPLETE — final 1152x1536 PNG from completed application renders.')


if __name__ == '__main__':
    main()
