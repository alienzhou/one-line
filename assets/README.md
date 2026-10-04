<p align="right">
  <a href="README.zh_CN.md">简体中文</a> · <strong>English</strong>
</p>

# Assets

This directory stores reusable fonts, images, music, and sound effects, organized by asset type.

Keep each asset in the matching subdirectory and document its destination, naming, integration method, and source/license. Do not mix binary assets with Markdown documentation.

## Fonts

The fortune-card application uses Noto Sans CJK SC Regular from the official
[Noto CJK repository](https://github.com/notofonts/noto-cjk/blob/main/Sans/OTF/SimplifiedChinese/NotoSansCJKsc-Regular.otf),
under the SIL Open Font License in `fonts/OFL.txt`. Source SHA-256:
`2c76254f6fc379fddfce0a7e84fb5385bb135d3e399294f6eeb6680d0365b74b`.
Only generated 12/20 px, 4 bpp, uncompressed subsets are linked into the app;
the 16 MB source OTF is a development asset, not firmware payload. Regenerate
with `python3 tools/generate_fortune_fonts.py --converter /path/to/lv_font_conv`
using converter 1.5.3. The script collects current and legacy records, poetry attribution, and
displayed UI literals. `fonts/fortune-characters.txt` is the printable inventory;
host LVGL tests check actual fonts, widget bindings, a missing-glyph negative
case, and all 2200 current and 2112 legacy text layouts. On-device Chinese output remains unverified.

## Fortune content

`fortune/corpus.json` holds 2200 complete records: 1980 original lines and 220
classical Chinese poetry excerpts. Eight themes use 20/10/15/15/15/10/10/5 percent
proportions, without runtime phrase assembly. The 2000+ milestone is met; 10001+
remains future work. `fortune/legacy-corpus.json` freezes the old 2112-record bank
solely to preserve saved signatures. `fortune/previous-corpus.json` freezes the
preceding 2200 records to generate migration IDs and a compact bank of 312
retired records. Neither compatibility bank participates in new draws. `fortune/poetry-sources.json` retains each
excerpt's author, full title, source paragraphs, and pinned source link, checked
against [chinese-poetry](https://github.com/chinese-poetry/chinese-poetry).
Its MIT notice is in `fortune/sources/chinese-poetry-LICENSE.txt`. Ancient works
are public-domain; opencc-python-reimplemented 0.1.7 converts Traditional to Simplified Chinese, and
excerpt punctuation is resegmented. They are attributed quotations, not claimed
original work. Long displayed titles are abbreviated; data retains full titles.
The active text stream, dictionary, and index total 68600 bytes, plus attribution,
legacy bank, and migration mapping. `tools/pack_fortunes.py --release` enforces
2001 records. See [the product design](../docs/fortune-card.md).

The ten original pixel-scene families are drawn by `main/fortune_pixels.c`;
no downloaded artwork or stored scene images enter the firmware. Previews under
ignored `build/fortune-preview/` come from the actual LVGL host renderer.
The extension combines 80 scenes and four subjects into 320 skins, each with six
palettes. All 1920 complete card captures live in `images/skins/`. After
`./tools/test_fortune_ui.sh --skins` completes, run
`python3 tools/render_fortune_gallery.py` (requires Pillow) to package
[`fortune/skin-gallery.html`](fortune/skin-gallery.html) and the per-image SHA-256
manifest [`fortune/skin-catalog.json`](fortune/skin-catalog.json). These are completed
application renders, not device screenshots, and are excluded from firmware.
The previous `fortune-collections.png` poster is retained as first-release history.

Store reusable font files and generated font sources in `fonts/`.

- Use descriptive names that include the family, weight, size, and format when relevant.
- Document the source, license, character range, conversion command, and expected destination.
- Check Flash and internal-RAM impact before adding a font; the ESP32-C3 has no PSRAM.
- Do not commit fonts whose license does not permit redistribution.

## Images

Store reusable source images and generated display assets in `images/`.

| File | Dimensions and format | Use and source |
| --- | --- | --- |
| [`images/home.jpg`](images/home.jpg) | 3840 × 2160, JPEG | Product hero image embedded in both project README files to foreground AI Passport and its open, maker-oriented identity. |
| [`images/readme-hardware-specs.png`](images/readme-hardware-specs.png) | 2172 × 724, PNG RGBA | Optional technical infographic retained as a reference asset; it is no longer used as the homepage hero. Generated for this repository with the built-in image generation tool on 2026-09-17; the six labels and values were checked against the documented hardware contract. |
| [`images/logo-wordmark.png`](images/logo-wordmark.png) | 1648 × 336, PNG RGBA | Transparent black wordmark extracted from the repository's original `images/logo.png`; embedded in both project README files for light backgrounds. |
| [`images/logo-wordmark-dark.png`](images/logo-wordmark-dark.png) | 1648 × 336, PNG RGBA | White version of the extracted wordmark, used by the README `<picture>` element when GitHub is in dark mode. |
| `images/fortune-themes.png` | 1160 × 960, PNG | Eight-theme overview from completed actual LVGL captures; MIT. |
| `images/fortune-home.png` and `fortune-theme-1.png` through `fortune-theme-8.png` | 240 × 320, PNG | Theme selection and representative cards, rendered by the actual app; MIT. |
| [`images/fortune-cover.png`](images/fortune-cover.png) | 1086 × 1448, PNG | One Fortune community cover; original AI-generated gameplay illustration. |
| [`images/fortune-letters.png`](images/fortune-letters.png) | 1086 × 1448, PNG | Letter-opening gameplay illustration. |
| [`images/fortune-voices.png`](images/fortune-voices.png) | 1086 × 1448, PNG | Historical voices and moods illustration; the current app retires voice filtering. |
| [`images/fortune-collections.png`](images/fortune-collections.png) | 1086 × 1448, PNG | Four pixel-art collections illustration. |
| [`images/fortune-signature.png`](images/fortune-signature.png) | 1086 × 1448, PNG | Saved personal-signature illustration. |
| [`images/fortune-ten-collections.png`](images/fortune-ten-collections.png) | 1086 × 1448, PNG | Ten collections sampled from finished application renders; MIT. |
| [`images/fortune-skins-overview.png`](images/fortune-skins-overview.png) | 2172 × 7240, PNG | All 320 artwork regions from actual application renders; MIT. |
| `images/skins/overview-0.png` through `overview-9.png` | Each 1086 × 1448, PNG | Ten series overviews with 32 skins each; actual renders, MIT. |
| `images/skins/skin-000-0.png` through `skin-319-5.png` | Each 240 × 320, PNG | All 320 × 6 complete LVGL card captures; MIT. |
| [`images/fortune-ui-pixel-collection.png`](images/fortune-ui-pixel-collection.png) | 1200 × 700, PNG | Actual LVGL host-rendered contact sheet of ten cards. |
| [`images/fortune-volume.png`](images/fortune-volume.png) | 240 × 320, PNG | Actual LVGL host-rendered volume panel; MIT, generated by `tools/test_fortune_ui.sh`, not a device photo. |
| [`images/fortune-ui-unwrap.gif`](images/fortune-ui-unwrap.gif) | 240 × 320, GIF | Actual host-rendered letter-opening sequence. |
| [`images/fortune-ui-pixel-motion.gif`](images/fortune-ui-pixel-motion.gif) | 240 × 320, GIF | Actual host-rendered scene animation. |

The five portrait illustrations were generated with Codex's built-in image
generation tool on 2026-10-03. All five tasks explicitly completed, and the exact
local upload files were visually inspected for finished artwork. They are labeled
as gameplay illustrations, not device photographs. Prompts are retained in
[`fortune/publication-image-prompts.json`](fortune/publication-image-prompts.json).
They are retained as prior-version community illustrations; the current README uses actual theme renders. they are not linked into
firmware. These original project images follow the repository MIT license.
The three UI previews come from the actual application renderer and carry the
same license. The generated illustrations and host previews have different roles;
on-device visual acceptance is still pending.

- Use descriptive names and document dimensions, pixel format, conversion steps, and destination.
- Prefer formats suitable for the 240 × 320 RGB565 display and account for Flash and internal RAM.
- Preserve editable sources where licensing permits, and record the source and license.
- Never commit device QR secrets, credentials, or personal data in images.

## Music and sound effects

The original music-box cues are synthesized by `main/fortune_sound.c` under the
repository MIT license. `tools/render_fortune_audio.py` exports completed,
16 kHz, signed 16-bit mono WAV previews into `music/`: `fortune-draw-1.wav` through
`fortune-draw-4.wav`, `fortune-keep.wav`, `fortune-skin.wav`, and the combined
`fortune-audition.wav`. `fortune-audio.json` records completion, lengths and hashes.
There are no downloaded samples or third-party compositions. The firmware links
the compact scores and sine table; these preview WAVs are development/publication
assets only and are not embedded. Listening previews omit physical codec and
speaker coloration.

Store reusable music and sound-effect sources in `music/`.

- Document the source, license, sample rate, bit depth, channels, conversion command, and destination.
- Prefer 16 kHz, 16-bit mono PCM when it matches the current BSP audio path.
- Check Flash and internal-RAM cost before embedding audio; stream or chunk long recordings.
- Do not commit media without redistribution permission.
