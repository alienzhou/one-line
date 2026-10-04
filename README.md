[简体中文](README.zh_CN.md) · **English**

# One Fortune

**Open a line. See another side of life. Keep it as your signature.**

One Fortune is a pocket text surprise box. Choose reassurance when you need it,
or explore everyday observations, dry humor, surreal ideas, personal attitudes,
relationship moments, small outings, and attributed classical Chinese poetry.
A sealed letter shakes, opens, and reveals its words. Keep a line you like on a
pixel-art card for display. The interface and texts are Chinese; it works
entirely offline without Wi-Fi setup.

![Eight themes, their actual interfaces and corpus proportions](assets/images/fortune-themes.png)

## A different subject in each letter

| Theme | Count and proportion | What it brings |
| --- | --- | --- |
| Gentle recharge | 440 · **20%** | Support and reassurance |
| Classical echoes | 220 · **10%** | Classical poetry with author and work title |
| Everyday observations | 330 · 15% | Details in streets, food, and weather |
| Dry humor | 330 · 15% | A small everyday laugh |
| Surreal ideas | 330 · 15% | An odd new angle on familiar things |
| Personal attitudes | 220 · 10% | Taste, judgment, and individual style |
| Relationship moments | 220 · 10% | Specific moments between people |
| Small outings | 110 · 5% | A small action to try yourself |

There are **2,200 complete records**: 1,980 original lines and 220 classical
excerpts. This update adds 1,540 original lines and retains 440 reassurance
notes. Every record is stored whole; sentences are never assembled from parts.
Chance mode draws from every theme. Records do not repeat within a cycle,
even after changing filters. The percentages describe the whole corpus;
a small set of draws need not have the same proportions.
The original 10,001+ independently authored target remains future work.
Visual combinations are counted separately from written content.

## Start in five steps

1. Turn on the device; no Wi-Fi setup is needed. Press any button once to wake a dark display.
2. Use UP/DOWN on the home page to select a theme, or leave it to chance. Hold DOWN to change voice, then press OK to draw.
3. Wait for the letter to open and reveal its words. Press OK during opening to skip the animation.
4. On the result, press UP to draw again or DOWN for random artwork. Press OK to keep this line as your displayed signature.
5. Hold OK to return home; hold OK there to view your saved signature. On a card, hold UP for volume or DOWN to mute. Your signature and settings survive restarts.

Voices are straightforward, playful, and poetic; unavailable voices are skipped.
Drawing keeps your existing signature until you explicitly save a replacement.
Changing artwork preserves the words. Previously saved signatures and artwork
remain readable after this upgrade.

<p align="center">
  <img src="assets/images/fortune-home.png" alt="Actual rendered theme-selection page" width="240">
  <img src="assets/images/fortune-theme-2.png" alt="Actual rendered poetry card with attribution" width="240">
</p>

## Give the words a different setting

Ten pixel-art collections combine **80 scenes × four companions = 320 skins**,
each in six palettes: **1,920 appearances**. Each shuffled cycle covers every
combination, with different adjacent scenes. Text stays still while the scenery
has small ambient movements.

![Ten pixel-art collections](assets/images/fortune-ten-collections.png)

[See all 320 skins](assets/images/fortune-skins-overview.png), or open the
[interactive gallery](assets/fortune/skin-gallery.html) locally to filter by
collection, companion, and palette and click to enlarge. Keep the adjacent
image directory when downloading it.

<p align="center">
  <img src="assets/images/fortune-ui-unwrap.gif" alt="Host-rendered letter-opening animation" width="240">
  <img src="assets/images/fortune-ui-pixel-motion.gif" alt="Host-rendered ambient pixel animation" width="240">
</p>

These are application host renders, not device photographs. The animations
retain prior-version text as motion examples.

## Choose your sound

Short original music-box cues accompany opening, reveal, keeping, and artwork
changes. Hold UP on a card for volume. UP/DOWN changes it from 10% to 100% in
10% steps, with an audition. OK saves and enables sound; hold OK to cancel and
restore the old volume and mute setting. Default volume is 80%.
Hold DOWN on a card to mute or restore sound without losing the chosen volume.

<p align="center"><img src="assets/images/fortune-volume.png" alt="Actual rendered volume panel" width="240"></p>

[Listen to the audition](assets/music/fortune-audition.wav). It is exported by
the firmware sound generator and excluded from firmware; speaker loudness
still needs device listening. The lines offer reading, expression, and
entertainment, without predictions.

## Development, builds, and tests

Read [AGENTS.md](AGENTS.md) and the [build guide](docs/development/engineering/build-and-test.md).
The application reuses BSP and owns its pages. Activate ESP-IDF 5.5.3, then run:

```bash
./tools/validate.sh
./tools/test_fortune_ui.sh --skins
# Use a Python with Pillow 11.3.0 installed
python3 tools/render_fortune_gallery.py
python3 tools/render_fortune_topics.py
```

The complete gate produces `build/FoloToy-AI-Passport-full.bin` for **0x0**.
Matching images, ELF, MAP, and manifest are archived under
`build/firmware/<full-image-sha256>/`. Firmware and private logs stay out of Git.
A merged write can overwrite stored data; this upgrade uses compatible segmented
writing to preserve NVS. See the [flashing and data policy](docs/development/engineering/firmware-layout.md#flashing-and-stored-data).

| Check | This update |
| --- | --- |
| Build | NOT RUN — complete build and merged-image verification pending |
| Host tests | PASS — exact decoding, proportions and poetry sources, legacy migration, input and storage; active fonts and all text layouts, 1,920 artwork renders |
| Device tests | NOT RUN — flashing and startup observation pending |
| Unverified | Physical theme selection and Chinese readability, old-signature recovery, volume/mute recovery, sound quality, animation smoothness, interrupted writes, idle/wake, battery reporting and endurance; perceived text repetition |

The active encoded text bank takes **66.3 KiB**, plus attribution and legacy
compatibility data. The **57.3 KiB** old bank serves saved signatures only and is
excluded from new draws. Both font sizes cover **2,285 characters**; the whole
bank is never loaded into RAM. The card save is **323 bytes**, with separate
one-byte volume and mute preferences. Procedural artwork retains its
12,528-byte canvas; gallery images stay out of firmware.
See the [product and engineering design](docs/fortune-card.md) for measurements
and compatibility boundaries.

## Explore or extend

- [Product, storage, controls, and acceptance](docs/fortune-card.md)
- [Complete themed corpus](assets/fortune/corpus.json) and [poetry sources](assets/fortune/poetry-sources.json)
- [Application entry](main/main.c) and [pixel renderer](main/fortune_pixels.c)
- [Asset sources and licenses](assets/README.md)
- [Upstream hardware and development documents](docs/README.md)

Original code and assets use the [MIT license](LICENSE). Classical texts are
public-domain works checked against the [chinese-poetry collection](https://github.com/chinese-poetry/chinese-poetry).
Its [MIT license](assets/fortune/sources/chinese-poetry-LICENSE.txt) and pinned
source records are retained. Long titles are abbreviated on cards; source data
retains full titles. Noto Sans CJK keeps its [SIL Open Font License](assets/fonts/OFL.txt).
The upstream platform is [FoloToy AI Passport](https://github.com/FoloToy/ai-passport).
