#!/usr/bin/env python3
"""Package completed LVGL captures into the complete, offline skin gallery."""
import argparse
import hashlib
import json
import re
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

ROOT = Path(__file__).resolve().parents[1]
FAMILIES = ['夜航电台', '旷野来信', '宇宙邮局', '口袋花园', '海底漫游', '云端小岛', '街角咖啡', '复古游园', '微光工坊', '冬日慢邮']
SCENES = len(FAMILIES) * 8
SKINS = SCENES * 4
APPEARANCES = SKINS * 6
SUBJECTS = ['猫与灯', '旅人与灯笼', '机器人与电台', '兔子与花']
HTML = r'''<!doctype html>
<html lang="zh-CN"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>一签 · 全部320款像素皮肤</title>
<style>
:root{color-scheme:light;--ink:#253c3b;--muted:#6c7970;--paper:#f4f0e5;--line:#dadccf;--green:#345d50}
*{box-sizing:border-box}body{margin:0;background:var(--paper);color:var(--ink);font:15px/1.65 system-ui,-apple-system,sans-serif}
header,main{max-width:1280px;margin:auto;padding:30px 32px}header{padding-bottom:10px}.eyebrow{font:12px/1.5 monospace;letter-spacing:2px;color:var(--green)}h1{font-size:clamp(30px,4vw,48px);margin:10px 0 8px;letter-spacing:-1px}h1 span{color:#a06443}p{margin:5px 0;color:var(--muted)}
.toolbar{position:sticky;top:0;z-index:2;background:rgba(244,240,229,.97);padding:14px 0;border-bottom:1px solid var(--line);display:flex;flex-wrap:wrap;gap:9px;align-items:center}
button,select{font:inherit;border:1px solid var(--line);border-radius:8px;background:#fffdf5;color:var(--ink);padding:7px 12px;cursor:pointer}button:hover{border-color:var(--green)}button[aria-pressed=true]{background:var(--green);color:white;border-color:var(--green)}button:focus-visible,select:focus-visible{outline:3px solid #c99b67;outline-offset:2px}.count{margin:14px 0;color:var(--muted)}
.grid{display:grid;grid-template-columns:repeat(auto-fill,minmax(178px,1fr));gap:20px}.card{padding:10px;background:#fffdf5;border:1px solid var(--line);text-align:left;box-shadow:0 5px 16px #3049360b;transition:transform .15s}.card:hover{transform:translateY(-3px)}.card img{display:block;width:100%;height:auto;image-rendering:pixelated;border-radius:6px}.card strong{display:block;margin-top:9px;font-size:15px}.card small{color:var(--muted)}
dialog{max-width:760px;width:calc(100% - 32px);border:1px solid var(--line);padding:24px;border-radius:16px;background:var(--paper);color:var(--ink)}dialog::backdrop{background:#101f28b3;backdrop-filter:blur(4px)}.detail{display:flex;align-items:center;justify-content:center;gap:30px}.detail img{width:min(320px,45vw);height:auto;image-rendering:pixelated;border-radius:10px}.detail h2{font-size:25px}.detail small{display:block;color:var(--muted)}.close{float:right}.nav{display:flex;gap:8px;margin-top:20px}.note{font-size:12px;margin:24px 0}a{color:var(--green)}
@media(max-width:600px){header,main{padding:22px 16px}.grid{grid-template-columns:repeat(2,minmax(0,1fr));gap:12px}.detail{flex-direction:column;gap:12px}.detail img{width:240px;max-width:80vw}.detail h2{margin:5px 0}}
</style></head><body>
<header><div class="eyebrow">ONE FORTUNE · PIXEL COLLECTION</div><h1>320款风景，<span>随手换一种心情。</span></h1>
<p>80处风景 × 4位小伙伴，组合成320款皮肤。每款都有6组配色。</p><p>全部来自程序实际渲染。点击卡片放大看，也可以筛选系列、小伙伴和配色。</p></header>
<main><div class="toolbar" id="toolbar"><button data-family="all" aria-pressed="true">全部320款</button>
<button data-family="0" aria-pressed="false">夜航电台</button><button data-family="1" aria-pressed="false">旷野来信</button><button data-family="2" aria-pressed="false">宇宙邮局</button><button data-family="3" aria-pressed="false">口袋花园</button>
<button data-family="4" aria-pressed="false">海底漫游</button><button data-family="5" aria-pressed="false">云端小岛</button><button data-family="6" aria-pressed="false">街角咖啡</button>
<button data-family="7" aria-pressed="false">复古游园</button><button data-family="8" aria-pressed="false">微光工坊</button><button data-family="9" aria-pressed="false">冬日慢邮</button>
<select id="subject" aria-label="小伙伴"><option value="all">所有小伙伴</option><option value="0">猫与灯</option><option value="1">旅人与灯笼</option><option value="2">机器人与电台</option><option value="3">兔子与花</option></select>
<select id="tone" aria-label="配色"><option value="0">配色一</option><option value="1">配色二</option><option value="2">配色三</option><option value="3">配色四</option><option value="4">配色五</option><option value="5">配色六</option></select><button id="random">随机看一张</button></div>
<div class="count" id="count"></div><div class="grid" id="grid"></div>
<p class="note">已完成320款 × 6组配色，共1920张签名卡渲染。文字固定为同一句，方便比较外观。主机预览，尚未完成新版真机验收。</p></main>
<dialog id="detail"><button class="close" id="close" aria-label="关闭">关闭 ×</button><div class="detail"><img id="large" alt=""><div><small id="series"></small><h2 id="name"></h2><p id="companion"></p><p id="number"></p><div class="nav"><button id="prev">← 上一款</button><button id="next">下一款 →</button></div></div></div></dialog>
<script type="application/json" id="catalog">__DATA__</script>
<script>
const skins=JSON.parse(document.getElementById('catalog').textContent).skins;
const families=JSON.parse(document.getElementById('catalog').textContent).families,subjects=['猫与灯','旅人与灯笼','机器人与电台','兔子与花'];
let family='all',subject='all',tone=0,selected=0,visible=[];
const grid=document.getElementById('grid'),detail=document.getElementById('detail');
function url(s){return '../images/skins/'+s.images[tone]}
function show(index){selected=(index+visible.length)%visible.length;const s=visible[selected];document.getElementById('large').src=url(s);document.getElementById('large').alt=s.name+' · '+subjects[s.subject];document.getElementById('series').textContent=families[s.family];document.getElementById('name').textContent=s.name;document.getElementById('companion').textContent=subjects[s.subject];document.getElementById('number').textContent='皮肤 '+String(s.skin+1).padStart(3,'0')+' / 320 · 配色 '+(tone+1);if(!detail.open)detail.showModal()}
function render(){visible=skins.filter(s=>(family==='all'||s.family===Number(family))&&(subject==='all'||s.subject===Number(subject)));grid.replaceChildren();document.getElementById('count').textContent='显示 '+visible.length+' 款 · 配色 '+(tone+1);visible.forEach((s,i)=>{const b=document.createElement('button');b.className='card';b.setAttribute('aria-label',s.name+'，'+subjects[s.subject]+'，点击放大');const img=document.createElement('img');img.src=url(s);img.alt=s.name+' · '+subjects[s.subject];img.loading='lazy';img.width=240;img.height=320;const t=document.createElement('strong');t.textContent=s.name;const sub=document.createElement('small');sub.textContent=subjects[s.subject]+' · '+String(s.skin+1).padStart(3,'0');b.append(img,t,sub);b.addEventListener('click',()=>show(i));grid.append(b)})}
document.querySelectorAll('[data-family]').forEach(b=>b.addEventListener('click',()=>{family=b.dataset.family;document.querySelectorAll('[data-family]').forEach(x=>x.setAttribute('aria-pressed',String(x===b)));render()}));
document.getElementById('subject').addEventListener('change',e=>{subject=e.target.value;render()});document.getElementById('tone').addEventListener('change',e=>{tone=Number(e.target.value);render()});document.getElementById('random').addEventListener('click',()=>show(Math.floor(Math.random()*visible.length)));document.getElementById('close').addEventListener('click',()=>detail.close());document.getElementById('prev').addEventListener('click',()=>show(selected-1));document.getElementById('next').addEventListener('click',()=>show(selected+1));detail.addEventListener('keydown',e=>{if(e.key==='ArrowLeft')show(selected-1);if(e.key==='ArrowRight')show(selected+1)});render();
</script></body></html>'''


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--captures', type=Path, default=ROOT / 'build/fortune-preview')
    args = parser.parse_args()
    catalog = json.loads((args.captures / 'skin-catalog.json').read_text())
    records = catalog['skins']
    assert [s['skin'] for s in records] == list(range(SKINS))
    output = ROOT / 'assets/images/skins'
    output.mkdir(parents=True, exist_ok=True)
    hashes = {}
    for s in records:
        assert s['family'] == s['skin'] % len(FAMILIES) and s['subject'] == s['skin'] // SCENES
        assert len(s['images']) == 6
        for name in s['images']:
            assert re.fullmatch(r'skin-\d{3}-[0-5]\.png', name)
            with Image.open(args.captures / Path(name).with_suffix('.ppm')) as image:
                assert image.size == (240, 320) and image.mode == 'RGB'
                image.load()
                image.save(output / name, optimize=True)
            hashes[name] = hashlib.sha256((output / name).read_bytes()).hexdigest()
    assert len(hashes) == APPEARANCES and len(set(hashes.values())) == APPEARANCES
    font = ROOT / 'assets/fonts/NotoSansCJKsc-Regular.otf'
    title = ImageFont.truetype(str(font), 42)
    body = ImageFont.truetype(str(font), 21)
    label = ImageFont.truetype(str(font), 16)
    panels = []
    for family in range(len(FAMILIES)):
        panel = Image.new('RGB', (1086, 1448), '#f4f0e5')
        draw = ImageDraw.Draw(panel)
        draw.text((40, 30), FAMILIES[family] + ' · 32款', font=title, fill='#253c3b')
        draw.text((40, 93), '8处风景 × 4位小伙伴 · 实际程序渲染', font=body, fill='#64786b')
        for layout in range(8):
            for subject in range(4):
                skin = family + layout * len(FAMILIES) + subject * SCENES
                s = records[skin]
                x, y = 34 + subject * 258, 143 + layout * 151
                draw.rounded_rectangle((x, y, x+244, y+141), 6, fill='#fffdf5', outline='#d5d9ce')
                with Image.open(output / s['images'][0]) as image:
                    panel.paste(image.crop((12, 38, 228, 154)), (x+14, y+7))
                draw.text((x+10, y+121), s['name']+' · '+SUBJECTS[subject], font=label, fill='#253c3b')
        draw.text((40, 1380), '一签 / 320款皮肤总览 · 主机预览，非真机照片', font=body, fill='#64786b')
        panel.save(output / ('overview-'+str(family)+'.png'), optimize=True)
        panels.append(panel)
    overview = Image.new('RGB', (2172, 7240), '#f4f0e5')
    for i, panel in enumerate(panels):
        overview.paste(panel, ((i % 2)*1086, (i // 2)*1448))
    overview.save(ROOT / 'assets/images/fortune-skins-overview.png', optimize=True)
    contact = Image.new('RGB', (1200, 700), '#f4f0e5')
    contact_draw = ImageDraw.Draw(contact)
    for family in range(len(FAMILIES)):
        skin = family + (family % 4) * SCENES
        with Image.open(output / records[skin]['images'][family % 6]) as card:
            contact.paste(card, ((family % 5) * 240, (family // 5) * 350))
        contact_draw.text(((family % 5) * 240 + 12, (family // 5) * 350 + 324), FAMILIES[family], font=label, fill='#253c3b')
    contact.save(ROOT / 'assets/images/fortune-ui-pixel-collection.png', optimize=True)
    series_cover = Image.new('RGB', (1086, 1448), '#f4f0e5')
    series_draw = ImageDraw.Draw(series_cover)
    series_draw.text((40, 24), '10个系列 · 320款皮肤', font=title, fill='#253c3b')
    series_draw.text((40, 87), '给这句话换一处风景 · 6组配色 / 1920种外观', font=body, fill='#64786b')
    for family in range(len(FAMILIES)):
        skin = family + (family % 4) * SCENES
        x, y = 34 + (family % 2) * 520, 136 + (family // 2) * 252
        with Image.open(output / records[skin]['images'][0]) as image:
            art = image.crop((12, 38, 228, 154)).resize((432, 232), Image.Resampling.NEAREST)
            series_cover.paste(art, (x, y))
        series_draw.text((x, y+232), FAMILIES[family], font=label, fill='#253c3b')
    series_draw.text((40, 1410), '一签 · 全部来自程序实际渲染 / 主机预览，非真机照片', font=body, fill='#64786b')
    series_cover.save(ROOT / 'assets/images/fortune-ten-collections.png', optimize=True)
    catalog['capture_source'] = 'actual LVGL application host renderer'
    catalog['render_status'] = f'completed: {SKINS} skins, all {APPEARANCES} palette variants'
    catalog['sha256'] = hashes
    destination = ROOT / 'assets/fortune/skin-gallery.html'
    destination.write_text(HTML.replace('__DATA__', json.dumps({'families': FAMILIES, 'skins': records}, ensure_ascii=False)), encoding='utf-8')
    (ROOT / 'assets/fortune/skin-catalog.json').write_text(json.dumps(catalog, ensure_ascii=False, indent=2)+'\n', encoding='utf-8')
    print(f'Gallery: COMPLETE — {SKINS} skins, {APPEARANCES} full-card PNGs, ten series panels, one complete overview.')
    print(destination.relative_to(ROOT))


if __name__ == '__main__':
    main()
