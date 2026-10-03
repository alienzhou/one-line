[简体中文](README.zh_CN.md) · **English**

# One Fortune

**Open a letter. Find a line that gets you. Keep it as your signature.**

One Fortune turns AI Passport into a pocket collection of encouraging, witty,
and quietly poetic notes. When a day feels draining, choose a mood and open an
unknown letter. Keep a line you like on a pixel-art card, ready to display as
your personal signature. It works entirely offline, with a Chinese interface
and Chinese fortunes.

<p align="center">
  <img src="assets/images/fortune-cover.png" alt="One Fortune illustrated cover: an open letter surrounded by pixel-art stamps" width="360">
</p>

| Complete fortunes | Voices | Pixel-art collections | Distinct generated scenes |
| --- | --- | --- | --- |
| **2,112** independently authored lines | Warm and assertive, playful internet humor, restrained poetry | Night Radio, Wildland Letters, Cosmic Post, Pocket Garden | **24,576** base images |

The current release contains 2,112 complete fortunes. The original goal of more
than 10,000 independently authored lines remains a future expansion. Scene
variations are counted separately from written content.

## A small ritual for a difficult day

Choose a mood, or leave it to chance. A sealed envelope shakes, opens, and lifts
its letter before the note appears. The reveal takes about 1.25 seconds; press
OK to skip it. Drawing again keeps your existing saved signature until you
explicitly save a replacement.

Three voices cover moments when you want reassurance, a funny line that says
what you are thinking, or a little quiet poetry. Fortunes do not repeat within
a draw cycle, even when you change filters. There are no daily quotas,
streaks, rarity ranks, accounts, or payment steps.

<table>
  <tr>
    <td><img src="assets/images/fortune-letters.png" alt="Illustration of choosing a mood, opening a letter, and revealing a fortune" width="300"></td>
    <td><img src="assets/images/fortune-voices.png" alt="Illustration of the three writing voices and eight moods" width="300"></td>
  </tr>
  <tr><td>Choose a mood and unwrap a note.</td><td>Find a voice that fits the moment.</td></tr>
  <tr>
    <td><img src="assets/images/fortune-collections.png" alt="Illustration of the four pixel-art collections" width="300"></td>
    <td><img src="assets/images/fortune-signature.png" alt="Illustration of keeping a fortune as a personal signature" width="300"></td>
  </tr>
  <tr><td>Four scenes, with small ambient movements.</td><td>Keep the words; change their setting.</td></tr>
</table>

*The cover and four posters are AI-generated gameplay illustrations. Their
Chinese copy describes the implemented experience. They are labeled as
illustrations in the images.*

## Start in five steps

1. Turn on the device. It works offline without Wi-Fi setup. If the display is off, press any button once to wake it.
2. On the mood page, use UP/DOWN to choose a mood. Hold DOWN to change the voice, then press OK to draw.
3. Watch the envelope open and the note appear. Press OK during the opening animation to reveal it immediately.
4. On the result page, press UP to draw again or DOWN to change only the artwork. Press OK to save this note and show it as your signature.
5. Hold OK to return to the mood page; hold OK there to view your saved signature. Restarting restores it. Drawing again keeps it until you save another note.

Fortunes offer encouragement and entertainment. They make no predictions or
promises about what will happen.

## See the actual interface

These previews come from the application's host renderer, including its fonts,
layout, and rounded display mask. They are not on-device photographs.

![Actual rendered cards from all four collections](assets/images/fortune-ui-pixel-collection.png)

<p align="center">
  <img src="assets/images/fortune-ui-unwrap.gif" alt="Actual rendered letter-opening sequence" width="240">
  <img src="assets/images/fortune-ui-pixel-motion.gif" alt="Actual rendered ambient pixel-art animation" width="240">
</p>

*Letter opening and ambient scene motion. Text remains still and readable.*

## Build and verification

Read [AGENTS.md](AGENTS.md), the [environment guide](docs/development/engineering/environment-setup.md),
and the [build guide](docs/development/engineering/build-and-test.md). This
application reuses the upstream BSP and provides its own pages and interaction
flow. Activate ESP-IDF 5.5.3, then run:

```bash
./tools/validate.sh
./tools/test_fortune_ui.sh
```

The complete gate builds and verifies a merged image at
`build/FoloToy-AI-Passport-full.bin` for flashing from **0x0**, with its matching
ELF, MAP, and manifest archived under `build/firmware/`. Generated firmware and
private device logs are excluded from Git. Follow the [flashing and data policy](docs/development/engineering/firmware-layout.md#flashing-and-stored-data)
before writing a device.

| Check | Current result |
| --- | --- |
| Build | **PASS** — complete gate and merged-image verification |
| Host tests | **PASS** — corpus decoding, draw history, persistence, fonts, all 2,112 text layouts, controls, and animations; all 24,576 rendered base scenes have distinct image hashes |
| Device tests | **PASS, limited** — authorized flashing with write verification and a 15-second clean startup observation on 2026-10-03 |
| Unverified | Physical button interactions, on-device Chinese legibility and clipping, animation smoothness, saved-signature recovery after restart or interrupted writes, idle/wake behavior, battery accuracy, power consumption, and endurance |

The complete encoded text bank takes **57.3 KiB**. Artwork is drawn from scene
rules rather than a library of stored backgrounds; the canvas uses **12,528
bytes** of RAM. These measurements and test boundaries are detailed in the
[product and engineering design](docs/fortune-card.md).

## Explore or extend

- [Product design, controls, storage, and acceptance](docs/fortune-card.md)
- [Complete authored corpus](assets/fortune/corpus.json)
- [Application source](main/main.c) and [pixel-art renderer](main/fortune_pixels.c)
- [Asset sources, licenses, and image provenance](assets/README.md)
- [AI illustration prompts](assets/fortune/publication-image-prompts.json)
- [Upstream hardware and development documentation](docs/README.md)

Application code and original project assets follow the repository's
[MIT license](LICENSE). Noto Sans CJK font assets retain their
[SIL Open Font License](assets/fonts/OFL.txt). The upstream platform is
[FoloToy AI Passport](https://github.com/FoloToy/ai-passport).
