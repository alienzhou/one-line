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
using converter 1.5.3. The script collects every complete corpus record and
displayed UI literal. `fonts/fortune-characters.txt` is the printable inventory;
host LVGL tests check actual fonts, widget bindings, a missing-glyph negative
case, and all 2112 text layouts. On-device Chinese output remains unverified.

## Fortune content

`fortune/corpus.json` holds 2112 individually written complete sentences across
eight moods and three voices (88 per pair). This meets the later 2000+ milestone;
the original 10001+ aspiration remains a separate unfinished expansion. No runtime
templates or phrase combinations are used. `tools/pack_fortunes.py` preserves each
full string through lossless code-point encoding. The measured bank, dictionary
and index occupy 58686 bytes. `--release` enforces the explicit current minimum
of 2001. See [the product design](../docs/fortune-card.md).

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
| [`images/fortune-cover.png`](images/fortune-cover.png) | 1086 × 1448, PNG | One Fortune community cover; original AI-generated gameplay illustration. |
| [`images/fortune-letters.png`](images/fortune-letters.png) | 1086 × 1448, PNG | Letter-opening gameplay illustration. |
| [`images/fortune-voices.png`](images/fortune-voices.png) | 1086 × 1448, PNG | Writing voices and moods illustration. |
| [`images/fortune-collections.png`](images/fortune-collections.png) | 1086 × 1448, PNG | Four pixel-art collections illustration. |
| [`images/fortune-signature.png`](images/fortune-signature.png) | 1086 × 1448, PNG | Saved personal-signature illustration. |
| [`images/fortune-ten-collections.png`](images/fortune-ten-collections.png) | 1086 × 1448, PNG | Ten collections sampled from finished application renders; MIT. |
| [`images/fortune-skins-overview.png`](images/fortune-skins-overview.png) | 2172 × 7240, PNG | All 320 artwork regions from actual application renders; MIT. |
| `images/skins/overview-0.png` through `overview-9.png` | Each 1086 × 1448, PNG | Ten series overviews with 32 skins each; actual renders, MIT. |
| `images/skins/skin-000-0.png` through `skin-319-5.png` | Each 240 × 320, PNG | All 320 × 6 complete LVGL card captures; MIT. |
| [`images/fortune-ui-pixel-collection.png`](images/fortune-ui-pixel-collection.png) | 1200 × 700, PNG | Actual LVGL host-rendered contact sheet of ten cards. |
| [`images/fortune-ui-unwrap.gif`](images/fortune-ui-unwrap.gif) | 240 × 320, GIF | Actual host-rendered letter-opening sequence. |
| [`images/fortune-ui-pixel-motion.gif`](images/fortune-ui-pixel-motion.gif) | 240 × 320, GIF | Actual host-rendered scene animation. |

The five portrait illustrations were generated with Codex's built-in image
generation tool on 2026-10-03. All five tasks explicitly completed, and the exact
local upload files were visually inspected for finished artwork. They are labeled
as gameplay illustrations, not device photographs. Prompts are retained in
[`fortune/publication-image-prompts.json`](fortune/publication-image-prompts.json).
They appear in the fork README and community submission; they are not linked into
firmware. These original project images follow the repository MIT license.
The three UI previews come from the actual application renderer and carry the
same license. The generated illustrations and host previews have different roles;
on-device visual acceptance is still pending.

- Use descriptive names and document dimensions, pixel format, conversion steps, and destination.
- Prefer formats suitable for the 240 × 320 RGB565 display and account for Flash and internal RAM.
- Preserve editable sources where licensing permits, and record the source and license.
- Never commit device QR secrets, credentials, or personal data in images.

## Music and sound effects

Store reusable music and sound-effect sources in `music/`.

- Document the source, license, sample rate, bit depth, channels, conversion command, and destination.
- Prefer 16 kHz, 16-bit mono PCM when it matches the current BSP audio path.
- Check Flash and internal-RAM cost before embedding audio; stream or chunk long recordings.
- Do not commit media without redistribution permission.
