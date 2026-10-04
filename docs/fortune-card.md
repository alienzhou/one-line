[简体中文](fortune-card.zh_CN.md) · **English**

# One Fortune: pixel collection

This iteration implements the user's later **2000+ complete-text milestone**:
2112 individually authored sentences, 88 for each of 8 moods × 3 voices.
The original request for more than 10000 independently authored sentences is
still a longer-term expansion, not satisfied by visual combinations. The three
voices are warm with agency, internet humor, and restrained poetry. Mood selection
includes an unfiltered option. Content lives in `assets/fortune/corpus.json`.

## Product

Give someone under everyday pressure a sentence they can keep as a personal
signature. Choose a mood, draw a letter, change its appearance if desired, and
keep one card. Drawing does not replace the pinned signature; confirming does.
Reboot opens the saved signature. The public card shows its collection and text,
not the mood that led the user to draw it.

No account, network, payment, daily quota, streak, rarity ranking, bad-luck draw,
or guaranteed prediction is used. Audio and radio stacks remain off. Text is
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
wrapping uses measured glyph widths, favors short clauses, and keeps closing
Chinese punctuation off line starts without changing source bytes.

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
The 312-byte save layout and text corpus identity remain unchanged; the old cursor
is converted lazily on the first appearance change. Downgrading to the old firmware
cannot read new appearance IDs. A merged 0x0 flash may still reset NVS.

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

All 2112 records were authored as complete strings. The eight topics are depletion,
boundaries, restarting, comparison, uncertainty, loneliness, small joys, and overload.
The source has no exact duplicates; the longest current sentence is 21 characters.
A near-text screen using character-bigram Jaccard ≥0.48 followed by sequence
similarity >0.72 found no pairs. This detects close phrasing, not every repeated
meaning. The voices and emotional themes deliberately recur; final reader
preference and perceived repetition remain for user review.

`python3 tools/pack_fortunes.py --release` checks the source's explicit
`production_minimum` (currently 2001). Host tests also require 2000+ records and
roundtrip every byte through the C decoder. `long_term_target` records 10001;
changing that metadata does not manufacture missing content.

| Resource | Measured size |
| --- | ---: |
| Original UTF-8 text, including terminators | 86490 bytes |
| 11-bit packed code-point stream | 38674 bytes |
| Character dictionary | 3116 bytes |
| Record index and metadata | 16896 bytes |
| Entire encoded text bank | **58686 bytes / 57.3 KiB** |
| Pixel canvas, internal RAM | **12528 bytes** |
| Serialized NVS state including seen bitset and CRC | **312 bytes** |
| Original application image | **1138976 bytes** |
| Ten-collection application image | **1186544 bytes** |
| Sound application image | **1253840 bytes** |
| Sound increase, including playback support | **67296 bytes / 65.7 KiB** |
| Sound preferences, stored separately | **Two 1-byte values (mute and volume)** |

Measured with the complete ESP-IDF 5.5.3 gate on 2026-10-04. The 1920 gallery
images and seven audition WAVs are development previews and are not linked into
firmware. Canvas RAM and the serialized card save size are unchanged.

The decoder reads a single record into 128 bytes. It never assembles text and
never loads the whole bank into RAM. The implementation supports up to 16384
records. Subset fonts are generated from Noto Sans CJK SC Regular with
`lv_font_conv 1.5.3`, 12/20 px, 4 bpp, with **1705 printable glyphs per size**.
The source OTF is a development asset and is not embedded. License, source hash,
and rebuilding instructions are in `assets/README.md`.

## Firmware and controls

The sound extension uses `codex/fortune-sound`, based on the ten-collection
application at `dbc22f8`. Earlier skin work used `codex/fortune-skins`; the original
checkout's unrelated edits were preserved. BSP is reused unchanged. Demo screens
are not linked. Target remains ESP32-C3, 8 MB Flash, no PSRAM, ESP-IDF 5.5.3,
LVGL 9.5.0 and the default minimal NVS/PHY/factory partition layout.

| Page | UP | DOWN | OK | Long press |
| --- | --- | --- | --- | --- |
| Mood selection | Previous mood | Next mood | Draw | DOWN changes voice; OK opens saved signature; UP resets only an exhausted filter |
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
error recovery. State is explicitly encoded and checksummed. A corpus fingerprint
change currently resets the deck and pin, so stable-ID migration is required before
promising future content upgrades will preserve saved signatures.

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
for old saves; invalid/out-of-range volume values also use 80%. The 312-byte card
format and existing signatures remain compatible. Mute preserves the selected volume.
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
A merged flash can reset existing NVS data. **Device tests: PASS for flashing and startup of adjustable volume `7db813f`.**
Its three component hashes matched after segmented writing without touching NVS.
A 20-second startup matched ELF `e94944f4b` with 216740 free heap bytes and
a 114688-byte largest block, with no crash observed. Volume control and persistence
still need user acceptance; raw evidence is in ignored `build/audio-volume/`.
The first audio version `881fb43` passed segmented write and hash verification
and a 20-second matching startup on 2026-10-04: free heap 216740 bytes, largest
block 114688 bytes. The user confirmed audible sound but found 60% too quiet.
The current control is the follow-up; its loudness and persistence need acceptance.
The earlier 320-skin revision passed segmented write, hash verification and a
15-second startup observation on 2026-10-04.
The verified component images were written without touching NVS. The matching application completed initialization, with 223164 bytes of free heap and a largest free block of 114688 bytes. Screen and button acceptance remain unverified. The first release was flashed with user approval on 2026-10-03 and passed
write verification and a 15-second clean startup observation. That result applies
to the old firmware only; its record is in ignored `build/device-test-pixel.json`.
The first community submission (project 914, revision 1910) also uses that prior
image and is separate from this skin extension.

Sound, visual and interaction acceptance
still require user observation. Pending acceptance: speaker loudness, crackles and
reveal synchronization, rapid draw/skip/mute/save sequences, persisted mute, physical buttons and repeat presses,
Chinese legibility/corner clipping, opening/wipe smoothness, pin/restart and power-cut
behavior, idle dim/off/wake, battery reporting, runtime heap, current and battery
life. Any later flash requires applicable user authorization. The 10001+ expansion and
reader review of tone/repetition are outside the completed 2000+ milestone.
