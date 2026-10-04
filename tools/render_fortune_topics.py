#!/usr/bin/env python3
"""Package completed LVGL topic captures for the illustrated application README."""
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont

ROOT = Path(__file__).resolve().parents[1]


def main():
    captures = ROOT / 'build/fortune-preview'
    output = ROOT / 'assets/images'
    font = str(ROOT / 'assets/fonts/NotoSansCJKsc-Regular.otf')
    head = ImageFont.truetype(font, 38)
    body = ImageFont.truetype(font, 21)
    small = ImageFont.truetype(font, 16)
    topics = [('慢慢回血','20% · 440张'),('诗词留声','10% · 220张'),
              ('生活观察','15% · 330张'),('冷幽默','15% · 330张'),
              ('荒诞脑洞','15% · 330张'),('小见解','10% · 220张'),
              ('关系切片','10% · 220张'),('小小出走','5% · 110张')]
    page = Image.new('RGB', (1160, 960), '#f4f0e5')
    draw = ImageDraw.Draw(page)
    draw.text((36,18),'一签 · 8种话题，拆开另一面',font=head,fill='#253c3b')
    draw.text((36,74),'2200张完整签文 · 随缘混抽 / 主题筛选 · 喜欢就留作签名',font=body,fill='#64786b')
    for i, (name, count) in enumerate(topics):
        x, y = 36+(i%4)*280, 132+(i//4)*390
        with Image.open(captures / f'theme-{i+1}.ppm') as image:
            image.load()
            assert image.size == (240,320) and image.mode == 'RGB'
            page.paste(image,(x,y))
            image.save(output / f'fortune-theme-{i+1}.png',optimize=True)
        draw.text((x,y+326),name,font=body,fill='#253c3b')
        draw.text((x,y+355),count,font=small,fill='#64786b')
    draw.text((36,924),'实际程序主机渲染 · 非设备照片',font=small,fill='#64786b')
    page.save(output / 'fortune-themes.png',optimize=True)
    with Image.open(captures / 'home.ppm') as image:
        image.load()
        image.save(output / 'fortune-home.png',optimize=True)
    print('Topic renders: COMPLETE — eight cards, overview and selection page.')


if __name__ == '__main__':
    main()
