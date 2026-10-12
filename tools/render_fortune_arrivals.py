#!/usr/bin/env python3
"""Build an offline review of all arrivals from actual LVGL frames and PCM."""
import ctypes
import json
import os
from pathlib import Path
import re
import subprocess
import tempfile
import wave

from PIL import Image

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / 'build/fortune-preview'
OUTPUT = SOURCE / 'arrivals'
HTML = r'''<!doctype html><html lang="zh-CN"><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>一签 · 三十封奇遇</title><style>
:root{color-scheme:dark;font-family:system-ui,-apple-system,sans-serif;color:#f4e6c7;background:#101526}*{box-sizing:border-box}body{margin:0}main{max-width:1080px;margin:auto;padding:40px 28px}header{margin-bottom:28px}header p{color:#b5afa2;max-width:650px;line-height:1.8}h1{font-size:32px;font-weight:550;letter-spacing:2px;margin:8px 0}.eyebrow{font-size:12px;color:#d6b773;letter-spacing:3px}.layout{display:grid;grid-template-columns:320px 1fr;gap:44px}.player{position:sticky;top:24px;align-self:start}canvas{width:288px;height:384px;image-rendering:pixelated;display:block;margin:0 auto 22px;border-radius:28px;box-shadow:0 18px 60px #0005}button{font:inherit;background:#20283b;color:#ead9b7;border:1px solid #495066;border-radius:9px;padding:10px 16px;cursor:pointer}button:hover,button[aria-pressed=true]{border-color:#e9c479;background:#393745}button:focus-visible{outline:2px solid #e9c479;outline-offset:3px}.controls{display:flex;gap:10px;justify-content:center}.sound{font-size:13px;display:block;text-align:center;margin:16px}#phase{font-size:13px;color:#a7aaa9;text-align:center;min-height:24px}.grid{display:grid;grid-template-columns:repeat(3,minmax(0,1fr));gap:12px}.card{padding:9px 5px;font-size:13px}.card img{display:block;width:100%;max-width:120px;height:auto;margin:0 auto 8px;image-rendering:pixelated}.foot{font-size:12px;color:#8e98aa;line-height:1.8;margin-top:22px}h2{font-weight:500;margin:0 0 18px;font-size:19px}@media(max-width:750px){.layout{grid-template-columns:1fr}.player{position:static}.grid{grid-template-columns:repeat(3,1fr)}main{padding:26px 18px}h1{font-size:27px}}@media(prefers-reduced-motion:reduce){canvas{box-shadow:none}}
</style><main><header><div class="eyebrow">ONE FORTUNE / 奇遇来信</div><h1>每一封，都有自己的到来方式。</h1><p>鲸浮出海面，末班车驶入月台，水洼先映出彩虹。看一小段奇遇发生，再把那句话留下来。</p></header><div class="layout"><section class="player"><canvas id="screen" width="240" height="320" aria-label="隐藏签实际界面动画"></canvas><div class="controls"><button id="replay">重新拆信</button><button id="skip">直接看签</button></div><label class="sound"><input type="checkbox" id="sound"> 开启声音</label><p id="phase" aria-live="polite"></p><p class="foot">使用当前程序的实际界面帧与合成音效。<br>此页仅供预览，不改变你的抽签和收藏记录。</p></section><section><h2 id="selected"></h2><div class="grid" id="grid"></div></section></div></main><script>
const cards=__CARDS__, canvas=document.querySelector('canvas'), ctx=canvas.getContext('2d'), phase=document.querySelector('#phase'), reduced=matchMedia('(prefers-reduced-motion: reduce)').matches;
let current=0,sprite,started=0,skipped=false,playing=false,sound=null,ready=false,requestId=0;
function status(t){if(phase.textContent!==t)phase.textContent=t}
function stopSound(){if(sound){sound.pause();sound=null}}
function startSound(){stopSound();if(document.querySelector('#sound').checked){sound=new Audio(`${String(current).padStart(2,'0')}.wav`);sound.volume=.8;sound.play().catch(()=>status('点选后可播放声音'))}}
function drawFrame(frame){ctx.clearRect(0,0,240,320);ctx.drawImage(sprite,0,frame*320,240,320,0,0,240,320)}
function replay(){if(!ready)return;started=performance.now();skipped=false;playing=true;startSound();status('拆开一封特别来信')}
async function select(index,animate=true){let ticket=++requestId;current=index;ready=false;stopSound();const next=new Image();next.src=`${String(index).padStart(2,'0')}-frames.png`;await next.decode();if(ticket!==requestId)return;sprite=next;ready=true;document.querySelector('#selected').textContent=cards[index];document.querySelectorAll('.card').forEach((b,i)=>b.setAttribute('aria-pressed',String(i===index)));drawFrame(26);if(animate)replay();else{playing=false;status('静态预览 · 可点击重新拆信')}}
function tick(now){if(ready&&playing){const elapsed=skipped?3500+now-started:now-started;let frame;if(elapsed<3250){frame=Math.min(25,Math.floor(elapsed/125));status(elapsed<1250?'拆开一封特别来信':elapsed<2750?'奇遇正在发生':'把这一句，留给自己')}else{const ambient=Math.floor((elapsed-3250)/250);frame=reduced||ambient===0?26:27+(ambient-1)%16;status('已留下 · 安静陪伴')}drawFrame(frame)}requestAnimationFrame(tick)}
cards.forEach((name,i)=>{const b=document.createElement('button');b.className='card';b.setAttribute('aria-pressed','false');b.innerHTML=`<img src="${String(i).padStart(2,'0')}.png" alt="" loading="lazy"><span></span>`;b.querySelector('span').textContent=name;b.onclick=()=>select(i);document.querySelector('#grid').append(b)});
document.querySelector('#replay').onclick=replay;document.querySelector('#skip').onclick=()=>{skipped=true;playing=true;started=performance.now();stopSound()};document.querySelector('#sound').onchange=e=>{if(e.target.checked)replay();else stopSound()};select(0,!reduced);requestAnimationFrame(tick);
</script></html>'''


def main():
    OUTPUT.mkdir(parents=True, exist_ok=True)
    header = (ROOT / 'main/fortune_rare.h').read_text()
    titles = re.findall(r'"([^"\n]+)"', header.split('titles[] = {', 1)[1].split('};', 1)[0])
    with tempfile.TemporaryDirectory(prefix='fortune-arrivals-') as temp:
        wrapper = Path(temp) / 'ids.c'
        wrapper.write_text('#include "fortune_rare.h"\nunsigned preview_art(unsigned n) { return fortune_rare_art_id(n,0); }\n')
        library = Path(temp) / 'renderer.so'
        subprocess.run([os.environ.get('CC', 'cc'), '-O2', '-shared', '-fPIC', '-I', str(ROOT/'main'),
                        str(ROOT/'main/fortune_sound.c'), str(wrapper), '-o', str(library)], check=True)
        renderer = ctypes.CDLL(str(library))
        renderer.preview_art.argtypes = [ctypes.c_uint]
        renderer.preview_art.restype = ctypes.c_uint32
        renderer.fortune_sound_samples.argtypes = [ctypes.c_int]
        renderer.fortune_sound_samples.restype = ctypes.c_uint32
        renderer.fortune_sound_render.argtypes = [ctypes.c_int, ctypes.c_uint, ctypes.c_uint32,
                                                 ctypes.POINTER(ctypes.c_int16), ctypes.c_size_t]
        renderer.fortune_sound_render.restype = ctypes.c_size_t
        def cue(sound, art):
            n = renderer.fortune_sound_samples(sound)
            pcm = (ctypes.c_int16*n)()
            assert renderer.fortune_sound_render(sound,art,0,pcm,n)==n
            return bytes(pcm)
        for kind, title in enumerate(titles):
            frames = [Image.open(SOURCE/f'rare-arrival-{kind:02}-{step:02}.ppm').convert('RGB') for step in range(27)]
            frames += [Image.open(SOURCE/f'rare-motion-{kind}-{step:02}.ppm').convert('RGB') for step in range(16)]
            sprite = Image.new('RGB', (240, 320*len(frames)))
            for i, frame in enumerate(frames):
                assert frame.size == (240,320)
                sprite.paste(frame, (0,320*i))
            sprite.save(OUTPUT/f'{kind:02}-frames.png', optimize=True)
            frames[26].save(OUTPUT/f'{kind:02}.png')
            if kind<3:
                frames[0].save(OUTPUT/f'{kind:02}.gif', save_all=True, append_images=frames[1:],
                               duration=[125]*26+[250]*16+[1500], loop=0)
            art = renderer.preview_art(kind)
            with wave.open(str(OUTPUT/f'{kind:02}.wav'), 'wb') as audio:
                audio.setnchannels(1); audio.setsampwidth(2); audio.setframerate(16000)
                audio.writeframes(cue(1,art)+bytes(3200)+cue(5,art))
    (OUTPUT/'index.html').write_text(HTML.replace('__CARDS__', json.dumps(titles,ensure_ascii=False)))
    print(f'Arrivals: {len(titles)} completed scene previews, actual LVGL frames and PCM: {OUTPUT / "index.html"}')


if __name__ == '__main__':
    main()
