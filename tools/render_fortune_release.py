#!/usr/bin/env python3
"""Render completed application captures as portrait community release panels."""
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / 'assets/images/community'
FONT = ROOT / 'assets/fonts/NotoSansCJKsc-Regular.otf'
BG, INK, MUTED = '#f4f0e5', '#253c3b', '#64786b'


def text(draw, pos, value, size=32, color=INK):
    draw.text(pos, value, font=ImageFont.truetype(str(FONT), size), fill=color)


def panel(title, subtitle):
    image = Image.new('RGB', (1152, 1536), BG)
    draw = ImageDraw.Draw(image)
    text(draw, (48, 36), '一签 · ONE FORTUNE', 24, MUTED)
    text(draw, (48, 88), title, 54)
    text(draw, (48, 172), subtitle, 27, MUTED)
    text(draw, (48, 1490), '实际程序主机渲染 · 非设备照片 / APPLICATION RENDERS', 21, MUTED)
    return image, draw


def capture(image, source, x, y, width):
    with Image.open(source) as screenshot:
        screenshot.load()
        if screenshot.size != (240, 320):
            raise ValueError(f'Unexpected capture: {source}')
        image.paste(screenshot.resize((width, width*4//3), Image.Resampling.NEAREST), (x, y))


def main():
    OUT.mkdir(parents=True, exist_ok=True)
    preview = ROOT / 'build/fortune-preview'
    image, draw = panel('生活，不止一种说法', '2200张完整签文 · 八种主题 · 随缘混抽或按主题选')
    text(draw, (48, 222), '慢慢回血 / 诗词留声 / 生活观察 / 冷幽默', 25, MUTED)
    text(draw, (48, 264), '荒诞脑洞 / 小见解 / 关系切片 / 小小出走', 25, MUTED)
    for i, theme in enumerate((2, 4, 3, 6)):
        capture(image, preview/f'theme-{theme}.ppm', 96+(i%2)*536, 326+(i//2)*560, 396)
    image.save(OUT/'themes.png', optimize=True)

    image, draw = panel('抽到一首诗，也知道它来自哪里', '220条古诗词摘句 · 作者与篇名随签显示')
    capture(image, preview/'theme-2.ppm', 336, 264, 480)
    text(draw, (48, 956), '全新随缘一轮：每20张中有2张诗词', 36)
    for i in range(20):
        x, y = 48+(i%10)*106, 1056+(i//10)*108
        poetry = i in (3, 14)
        draw.rectangle((x,y,x+90,y+86), fill='#b98d5a' if poetry else '#dce2d6')
        text(draw, (x+15,y+22), '诗' if poetry else '签', 36, '#ffffff' if poetry else INK)
    text(draw, (48, 1305), '顺序会打散，每次拆信都有一点期待。', 31)
    text(draw, (48, 1380), '分组示意；切换主题后跳过已读，短期比例可能变化。', 25, MUTED)
    image.save(OUT/'poetry.png', optimize=True)

    image, draw = panel('同一句话，换一处风景', '10个像素系列 · 320款皮肤 · 1920种外观')
    for i, skin in enumerate((0, 3, 24, 45, 67, 99)):
        capture(image, ROOT/f'assets/images/skins/skin-{skin:03}-0.png',
                56+(i%3)*364, 268+(i//3)*474, 312)
    text(draw, (48, 1216), '下键随机换肤，文字保持原样。', 38)
    text(draw, (48, 1288), '确定留下，重启继续展示。', 34)
    text(draw, (48, 1352), '再抽保留原签名，只有留下一张新签才会替换。', 29, MUTED)
    image.save(OUT/'signature-skins.png', optimize=True)

    image, draw = panel('三个按键，就能开始', '无需配网 · 喜欢就留下 · 声音大小自己选')
    capture(image, preview/'home.ppm', 56, 260, 480)
    capture(image, preview/'sound-volume.ppm', 616, 260, 480)
    lines = [
        '抽签：首页上下选主题，确定拆信。',
        '续抽 / 换肤：结果页上再抽、下换肤。',
        '留签：确定留下；长按确定回首页。',
        '音量：卡片长按上，上下调节，确定保存。',
        '静音：卡片长按下开关音效，重启记住。']
    for i, line in enumerate(lines):
        text(draw, (56, 978+i*83), f'{i+1}. {line}', 32)
    image.save(OUT/'controls-volume.png', optimize=True)
    print('Community panels: COMPLETE — four final 1152x1536 PNGs from completed application renders.')


if __name__ == '__main__':
    main()
