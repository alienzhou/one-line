[简体中文](fortune-card.zh_CN.md) · **English**

# One Fortune: eight-theme pixel collection

There are **2200 complete records**: 1980 original lines and 220 classical
Chinese poetry excerpts. This update adds 1540 original lines and retains 440
reassurance notes. Poetry displays its author and work title. Home selects a
theme, with chance mode mixing the entire corpus. The original 10001+ target
remains future work. Sources are `assets/fortune/corpus.json` and the checked
`assets/fortune/poetry-sources.json`.

## Product

Give someone under everyday pressure a sentence they can keep as a personal
signature. Choose a theme, draw a letter, change its appearance if desired, and
keep one card. Drawing does not replace the pinned signature; confirming does.
Reboot opens the saved signature. The public card shows its collection and text,
not the mood that led the user to draw it.

No account, network, payment, daily quota, streak, rarity ranking, bad-luck draw,
or guaranteed prediction is used. Radio stacks remain off; audio plays on demand. Text is
written as complete sentences; no sentence parts are combined at runtime.

### 320 combinatorial pixel skins

| Collection | Composition | Subtle motion |
| --- | --- | --- |
| Night Radio | Window room, roof terrace, bookshop, last train, harbor, balcony, library, rainy lane | Sparse rain and occasional blinking |
| Wild Letter | Mountain cabin, canyon bridge, lighthouse, treehouse, desert camp, alpine lake, windmill, flower meadow | Small firefly and occasional blinking |
| Space Post | Ringed planet, cockpit, moon base, observatory, launchpad, orbital cafe, station garden, comet field | A few stars twinkle |
| Pocket Garden | Plant window, greenhouse, terrarium, rose arch, lotus pond, flower cart, mushroom cottage, courtyard | A little steam and occasional blinking |

Art is rendered at **108 × 58 RGB565**, scaled exactly 2× with antialiasing off.
The illustration occupies (12,38)–(227,153) on the 240 × 320 panel. Chinese text
uses separate 20 px type; labels use 12 px. The type remains still. Explicit
wrapping uses measured glyph widths, uses bounded dynamic programming to favor balanced clauses, avoid
one-character tails, and keep closing punctuation off line starts without changing source bytes.

There are **80 scene compositions × 4 subjects = 320 skins**. The four subjects
are a cat with a lamp, a traveler with a lantern, a robot with a radio, and a rabbit
with flowers. Each has **6 palettes**, giving **1920 selectable appearances**.
This counts visible composition and subject combinations separately from color
variants. Shapes and colors are drawn by code; no background-image bank is embedded.

A keyed shuffle visits each scene once per 80 changes and each scene/subject skin
once per 320 changes. Six batches cover all 1920 appearances before repeating.
Adjacent scenes differ, including batch boundaries and cycle wrap. The sequence
is deterministic from the stored device seed and cursor and survives a restart.
These are pseudorandom entertainment choices, not cryptographic randomness.


| Collection | Eight scenes |
| --- | --- |
| Undersea Wandering | Coral post office, sea station, jellyfish lamp, whale garden, submarine window, pearl house, shipwreck books, bubble tea room |
| Cloud Islands | Cloud windmill, floating house, rainbow bridge, balloon harbor, sky garden, cloud station, kite hill, cloud bed |
| Corner Cafe | Morning pour-over, bakery kitchen, window dessert, cafe flowers, record afternoon, newsstand, roof tea, alley bread |
| Vintage Fair | Carousel, Ferris wheel, arcade, ticket booth, candy cart, park train, puppet theatre, balloon stand |
| Glow Workshop | Gear workshop, watch repair, pottery, dye garden, glass lamps, star charts, stamp carving, paper crafts |
| Winter Post | Snow post office, fireplace, frozen lake cabin, snowman garden, winter station, pine camp, warm town, snow bookshop |

The former 24576 IDs are retained only to render existing saved cards unchanged.
New draws and appearance changes use the new 1920-ID bank. This is why a saved
signature can initially show its old artwork until the user explicitly changes it.
This update changes the card save from 312 to 323 bytes and imports the old
corpus fingerprint 3420887641. Existing current/pinned records use a separate
legacy ID namespace, preserving their exact words and art. The 440 retained
records import their old seen bits; new content starts unread. Old mood filters
become reassurance, chance remains chance, and the artwork cursor is retained.
The old bank serves existing cards only, never new draws. Separate NVS mute and
volume keys are unchanged. Compatibility covers the identified old and current
banks, not arbitrary future banks or downgrades. A merged 0x0 write can still
reset NVS; this upgrade should use verified components with the same layout.

The [complete interactive gallery](../assets/fortune/skin-gallery.html) contains
all 320 skins in all six palettes, captured from the actual application renderer.
The [full overview](../assets/images/fortune-skins-overview.png) shows all 320 in
one image. Preview images live in the repository and are excluded from firmware.

### Anticipation and transitions

A new draw first shows a sealed letter: a tiny shake, seal, opening flap, and
rising paper. After ten 125 ms steps (about **1.25 s**), the complete sentence
and its scene appear. OK skips to the result. Other keys during this phase do
not draw or pin another card. The app persists the draw before starting the
animation; animation frames do not write Flash. Battery refresh does not end
an opening sequence early.

Changing only the appearance uses a three-step **375 ms pixel wipe**. The sentence
stays unchanged. Ambient animation is **4 fps**, and stops drawing frames after
45 idle seconds, when the display dims. Input restores it. The host test sampled
96 legacy scenes and all 320 new skins over 32 phases; at most 42 of 6264 canvas pixels changed from
the base frame (0.67%). Real-device frame timing and power are not yet measured.

## References and design conclusions

| Primary reference | What informed the design |
| --- | --- |
| [Pixel Joint pixel-art tutorial](https://pixeljoint.com/forum/forum_posts.asp?TID=11299) | Pixel clusters, silhouettes, intentional palettes, avoiding noisy isolated pixels |
| [Lospec ENDESGA 16](https://lospec.com/palette-list/endesga-16) and [Vanilla Milkshake](https://lospec.com/palette-list/vanilla-milkshake) | Strong value contrast and soft limited palettes as different art directions |
| [Aseprite animation documentation](https://www.aseprite.org/docs/animation/) | Frame-based animation vocabulary and timing |
| [I am](https://iam.monkeytaps.app/) and [Monkey Taps themes](https://faq.monkeytaps.app/en/articles/3542914) | Keeping a sentence on a reusable display surface |
| [Finch home](https://help.finchcare.com/hc/en-us/articles/37780000231309-Exploring-the-Finch-Home-Page) and [Good Vibes](https://help.finchcare.com/hc/en-us/articles/37780369483533-Sending-Good-Vibes) | A companion and brief encouragement as interaction references |

Our design conclusion is a letter-opening ritual with quiet collectible scenes.
These sources are references for design, not copied illustrations or text, and
not evidence of therapeutic efficacy. Every drawing is local procedural code.

## Content and memory

| Theme | Records | Proportion |
| --- | ---: | ---: |
| Gentle recharge | 440 | 20% |
| Classical echoes | 220 | 10% |
| Everyday observations | 330 | 15% |
| Dry humor | 330 | 15% |
| Surreal ideas | 330 | 15% |
| Personal attitudes | 220 | 10% |
| Relationship moments | 220 | 10% |
| Small outings | 110 | 5% |

These are whole-corpus proportions, not guarantees for a small run of draws.
Voices are straightforward, playful, and poetic. Voice cycling skips unavailable
voices; selecting a theme falls back to any voice when the old voice is absent.
Exhaustion requires changing filters or explicitly reshuffling.

Original records are authored whole, without assembling sentence fragments.
Classical excerpts use public-domain ancient works, checked against
chinese-poetry commit `b8594f81a89752241442f2ce267d6f66f96704ee` for words,
author, and title. opencc-python-reimplemented 0.1.7 converts Traditional to Simplified Chinese;
punctuation is segmented for the excerpt. Normalized words must match the
source. Full titles, context, pinned paths, and the collection's MIT license
are retained. Long titles are abbreviated in the 12 px attribution label.

There are no exact duplicate strings; the longest record is 24 characters.
Bigram Jaccard ≥0.48 and sequence similarity >0.72 find no close-wording pairs.
This does not prove semantic novelty; reader-perceived repetition remains open.
`python3 tools/pack_fortunes.py --release` enforces the current 2001 minimum.
Host checks roundtrip 2200 current and 2112 legacy strings, theme proportions,
and all 220 poetry sources. The 10001 `long_term_target` is a target, not content.

| Resource | Encoded size |
| --- | ---: |
| Current UTF-8 including terminators | 103669 bytes |
| Current 11-bit character stream | 46507 bytes |
| Current dictionary | 3808 bytes |
| Current index and metadata | 17600 bytes |
| Current encoded text subtotal | **67915 bytes / 66.3 KiB** |
| Attribution strings and 32-bit pointers | 3744 bytes |
| Old seen-record mapping | 4400 bytes |
| Legacy text compatibility bank | **58686 bytes / 57.3 KiB** |
| Read-only content total, before linker alignment | **134745 bytes / 131.6 KiB** |
| Internal-RAM pixel canvas | **12528 bytes** |
| NVS state including seen bitset and CRC | **323 bytes** |
| Separate sound preferences | **Two one-byte values (mute and volume)** |
| Current application image | **1492416 bytes** |
| Verified merged image | **1557952 bytes** |

Only one record is decoded into a 128-byte buffer; the entire bank is never
loaded into RAM. The limit is 16384 records. Noto Sans CJK SC and
`lv_font_conv 1.5.3` generate 12/20 px, 4 bpp fonts with **2285 characters per
size**, covering current/legacy text, attribution, and UI literals. The OTF,
1920 gallery PNGs, and audition WAVs are excluded from firmware. Sources and
regeneration are in `assets/README.md`.

## Firmware and controls

This update uses `codex/fortune-themes`, based on adjustable-volume `4da25a0`,
preserving earlier changes. BSP is reused unchanged. Demo screens
are not linked. Target remains ESP32-C3, 8 MB Flash, no PSRAM, ESP-IDF 5.5.3,
LVGL 9.5.0 and the default minimal NVS/PHY/factory partition layout.

| Page | UP | DOWN | OK | Long press |
| --- | --- | --- | --- | --- |
| Theme selection | Previous theme | Next theme | Draw | DOWN changes voice; OK opens saved signature; UP resets only an exhausted filter |
| Opening letter | Ignored | Ignored | Skip animation | OK also skips |
| Draw result | Draw again | New appearance | Save as signature | UP opens volume; DOWN toggles sound; OK returns to selection |
| Saved signature | Draw again, keep old pin | New appearance for saved card | Return to selection | UP opens volume; DOWN toggles sound; OK returns to selection |

An affine permutation plus a persistent seen bitset avoids repeating texts across
filter changes. Exhaustion requires explicit reshuffling. Appearance IDs have their
own shuffled scene/subject/color order, unaffected by a text-deck reset. These are entertainment draws;
no cryptographic randomness claim is made. See [ESP-IDF RNG prerequisites](https://docs.espressif.com/projects/esp-idf/en/v5.5.3/esp32c3/api-reference/system/random.html).

Button callbacks enqueue bounded messages. The worker owns application state and
NVS writes; all non-LVGL-task UI access holds the BSP lock. The LVGL timer handles
art and posts nonblocking sound events; it is deleted with its screen. Draw history is committed before reveal.
Save errors explicitly report that changes may be lost; NVS is never erased as
error recovery. State is explicitly encoded and checksummed. Old and new banks have separate checked fingerprints; only the explicitly
compatible format is imported. Unknown banks and corrupt saves are still rejected
without erasing NVS.

Backlight is 75% after input, 15% after 45 idle seconds, off after 120 seconds.
The first event after darkness wakes without changing the card. This is display
power management, not deep sleep. See [ESP-IDF NVS](https://docs.espressif.com/projects/esp-idf/en/v5.5.3/esp32c3/api-reference/storage/nvs_flash.html)
for persistence APIs; physical power-loss behavior remains a device check.

## Sound and playback ownership

Four original cue types accompany opening (1.15 s), reveal (0.8 s), keeping a
signature (0.58 s), and changing artwork (0.28 s), each in four related melodic
variants. The actual LVGL start/completion events trigger the opening and reveal;
pressing OK to skip replaces anticipation with exactly one reveal cue. Leaving a
reveal programmatically cancels it without a completion sound. Music never loops.

`fortune_sound.c` is a pure, allocation-free 16 kHz/16-bit mono renderer. Its
scores, pitch increments, duration table and interpolated sine table total about
680 bytes before alignment; samples are generated in 160-frame / 320-byte chunks.
Each note has an 8 ms attack and a decaying envelope, and each cue ends with
silence. `fortune_audio.c` owns one 4096-byte worker stack plus a single-slot
mailbox and a stop acknowledgement; it reuses `bsp_audio_*` without changing BSP,
pins, codec clocks, partitions, or the dependency lock. The existing BSP allocates
its I2S DMA buffers when first needed. Default output volume is 80 percent after the first device audition found
60 percent too quiet. Hold UP on a result or signature card to adjust 10–100%
in 10% steps, previewing a short cue. OK saves and enables sound; hold OK
cancels and restores the previous volume/mute state. Preview does not write NVS
or alter the card/deck. A failed save leaves the panel open for retry. The audio
worker applies gain at a cue boundary; no input callback touches the codec.

New events replace queued older effects. Only the audio worker initializes,
formats, writes, adjusts volume, sleeps, or wakes audio. Before any NVS write,
the input worker pauses event admission, requests a smooth stop and drains the
BSP's at-most-90-ms queue with 100 ms of silence. Only an acknowledged stop permits
the save; a one-second timeout reports a save failure instead of writing during
PCM playback. No LVGL lock is held while waiting. Idle playback sleeps the codec
after 2.5 seconds; mute and storage stops sleep it immediately after draining.
This is codec software suspend, not whole-device deep sleep or a claim about
measured current. The externally powered amplifier remains outside software control.

Sound starts enabled. Hold DOWN on a result or signature card to toggle it; the
muted state is visible in the title. Separate NVS `sound` and `volume` bytes default to enabled and 80%
for old saves; invalid/out-of-range volume values also use 80%. The 323-byte card format imports existing signatures. Mute preserves the selected volume.
Audio failures leave drawing and saving available and show a short notice; later
play requests retry the BSP path. No microphone capture is used.

Recreate the seven completed audition WAVs with `python3 tools/render_fortune_audio.py`.
The WAVs use the actual firmware renderer and are excluded from firmware; preview
hashes are in `assets/music/fortune-audio.json`. Host playback cannot verify the
speaker's tone, loudness or electrical clicks.

## Validation and delivery

Run the complete repository gate with `./tools/validate.sh` under ESP-IDF 5.5.3.
It includes 200 complete nonrepeating decks, filter switching, every source-string
decode, corrupt-state rejection, storage roundtrip, and pixel renderer checks. Skin tests cover 64 seeds over two complete cycles,
scene/skin block coverage, different adjacent scenes, and a synthetic save encoded
by the original `38ded1a` model. A whole-bank pixel fingerprint at three animation
phases confirms unchanged rendering of every legacy ID.
The pixel renderer additionally passed AddressSanitizer and UndefinedBehaviorSanitizer.
Input tests exercise the actual application controls, volume preview/save/cancel/clamping/reload, failed-save retry, muted-setting reload, legacy
defaults, voice selection, stop timeouts and failed-save silence.
Audio tests exercise all 16 scores for bounded peaks/DC, silent tails, unique
samples and arbitrary chunk boundaries. A threaded test runs the actual audio
worker through mute, replacement of rapid events, drain-before-save, timeout,
idle/wake and initialization/format/wake/write failures. The synth and worker also
pass AddressSanitizer and UndefinedBehaviorSanitizer.

`./tools/test_fortune_ui.sh` runs the actual LVGL renderer with the BSP rounded-panel
mask. It checks every text, 36 selection states, maximum-length layout, error states,
actual widget fonts and missing-glyph detection, timed opening, skip, cancellation,
exactly-once reveal sound events, muted labels, motion pause, and all 1920 artwork-region hashes. Generate the complete gallery with:

```bash
./tools/test_fortune_ui.sh --skins
python3 -m venv build/gallery-venv
build/gallery-venv/bin/pip install Pillow==11.3.0
build/gallery-venv/bin/python tools/render_fortune_gallery.py
```

The packaging command requires Pillow and the tracked Noto font. It checks all
1920 completed captures before converting them to PNG and building the HTML gallery.
These are host renders, not photographs.

The current delivery identity and test status are in ignored `build/delivery.json`.
The verified merged `build/FoloToy-AI-Passport-full.bin` is for offset **0x0**;
its matching ELF/MAP and manifest live in `build/firmware/<full-image-sha256>/`.
Build and Host tests PASS: ESP-IDF 5.5.3 complete gate, byte-verified merge,
current/legacy strings and poetry sources, old-save migration, fonts and actual
UI layout, all 1920 appearances, and model AddressSanitizer/UndefinedBehaviorSanitizer.
Device tests PASS for write and startup: `15621c4` has three verified component
hashes, with NVS untouched. Twenty seconds of startup match ELF `3c3eb0f55a`,
216576 free heap bytes and a 114688-byte largest block, without an observed crash
or rejected save. Physical screen, old-signature content, and controls remain
unverified. Full-image SHA256:
`18e50f269cdb6ce9bca83ab018d32109950f1348500f797bfb053c68479b6ef8`.
The merged image supports 0x0 but spans NVS; this data-preserving upgrade used
matching component images and the unchanged partition table. Prior adjustable-volume `7db813f` passed segmented hash
verification and 20 seconds of startup, with 216740 free heap bytes and a
114688-byte largest block. That result applies only to the prior firmware.
The first audio version was audible but too quiet at 60%; 10–100% control
followed. Volume operation and restart persistence still need user acceptance.
Current raw evidence is kept in ignored `build/themes/`; binaries stay out of Git.

Sound, visual and interaction acceptance
still require user observation. Pending acceptance: speaker loudness, crackles and
reveal synchronization, rapid draw/skip/mute/save sequences, persisted mute, physical buttons and repeat presses,
Chinese legibility/corner clipping, opening/wipe smoothness, pin/restart and power-cut
behavior, idle dim/off/wake, battery reporting, runtime heap, current and battery
life. Any later flash requires applicable user authorization. The 10001+ expansion and
reader review of tone/repetition are outside the completed 2000+ milestone.
