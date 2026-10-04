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
    image, draw = panel('生活，不止一种说法', '2200张完整签文 · 默认全库随机 · 也可确认主题后抽取')
    text(draw, (48, 222), '慢慢回血 / 诗词留声 / 生活观察 / 冷幽默', 25, MUTED)
    text(draw, (48, 264), '荒诞脑洞 / 小见解 / 关系切片 / 小小出走', 25, MUTED)
    for i, theme in enumerate((2, 4, 3, 6)):
        capture(image, preview/f'theme-{theme}.ppm', 96+(i%2)*536, 326+(i//2)*560, 396)
    image.save(OUT/'themes.png', optimize=True)

    image, draw = panel('抽到一首诗，也知道它来自哪里', '220条古诗词摘句 · 作者与篇名随签显示')
    capture(image, preview/'theme-2.ppm', 56, 264, 480)
    capture(image, preview/'selector-2.ppm', 616, 264, 480)
    text(draw, (48, 978), '诗词占完整签库的10%，一起随机洗牌。', 36)
    text(draw, (48, 1068), '短期可能没遇见，也可能抽到好几首。', 32)
    text(draw, (48, 1180), '想专门读诗？首页上 / 下打开主题页，', 32)
    text(draw, (48, 1250), '选「诗词留声」，按确定再开始抽。', 32)
    text(draw, (48, 1372), '回首页或重启恢复全库随机；已读与签名保留。', 27, MUTED)
    image.save(OUT/'poetry.png', optimize=True)

    image, draw = panel('同一句话，换一处风景', '10个像素系列 · 320款皮肤 · 1920种外观')
    for i, skin in enumerate((0, 3, 24, 45, 67, 99)):
        capture(image, preview/f'release-skin-{skin:03}.ppm',
                56+(i%3)*364, 268+(i//3)*474, 312)
    text(draw, (48, 1216), '下键随机换肤，文字保持原样。', 38)
    text(draw, (48, 1288), '确定留下，重启继续展示。', 34)
    text(draw, (48, 1352), '再抽保留原签名，只有留下一张新签才会替换。', 29, MUTED)
    image.save(OUT/'signature-skins.png', optimize=True)

    image, draw = panel('三个按键，就能开始', '无需配网 · 喜欢就留下 · 声音大小自己选')
    capture(image, preview/'home.ppm', 56, 260, 480)
    capture(image, preview/'sound-volume.ppm', 616, 260, 480)
    lines = [
        '抽签：首页确定全库随机；上下打开主题页。',
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
