[简体中文](fortune-card.zh_CN.md) · **English**

# One Fortune: eight-theme pixel collection

There are **2200 complete records**: 1980 original lines and 220 classical
Chinese poetry excerpts. This update adds 1540 original lines and retains 440
reassurance notes. Poetry displays its author and work title. Home draws from the
whole corpus by default; a separate large-text selector confirms optional topics. The original 10001+ target
remains future work. Sources are `assets/fortune/corpus.json` and the checked
`assets/fortune/poetry-sources.json`.

## Product

Give someone under everyday pressure a sentence they can keep as a personal
signature. Choose a theme, draw a letter, change its appearance if desired, and
keep one card. Drawing does not replace the pinned signature; confirming does.
Outside continuous-letter reading, reboot opens the saved signature and restores whole-bank draws. The card header
shows the current draw scope.

No account, network, payment, daily quota, streak, rarity ranking, bad-luck draw,
or guaranteed prediction is used. Radio stacks remain off; audio plays on demand. Text is
written as complete sentences; no sentence parts are combined at runtime.

### Continuous letters

The existing type selector adds Continuous Letters at index 9. Home UP/DOWN
opens that selector, while OK still draws from the complete fortune bank.
The new type has only two views: a random first-page preview and a selected
continuous reader. In preview, UP draws another story without repeats in a
four-story cycle, DOWN changes appearance, and OK selects or resumes it.
While reading, OK advances across both pages and letters, UP goes back one
page, and DOWN continues to change appearance. Hold OK from either view to
return directly home. At the end, OK marks the story complete and returns to
preview; UP chooses another, and OK rereads the completed story. Sound and
volume use the same long presses as fortune cards.

Four original stories contain 120 pages: *Windstill Post Office* has eight
six-page letters; *Platform Lost Property*, *The Lighthouse Keeper* and
*Rooftop Garden* each have four. Every page has at most two lines and 18
characters, including punctuation. Authored breaks, curated pixel scenes,
empty space and subtle animation preserve the original card rhythm. The story
is linear; the old mailbox, reply, receipt, archive and restart menus are gone.
All text and artwork run offline. No clock, phone, radio or service is required.

The independent `fortune/mail` record remains 16 explicit bytes. Version 2
stores four bookmarks, the last selected story, the active-reader flag and a
checksum. Zero means unread; page plus one is the bookmark; page count plus
one means completed. Browsing and changing appearance do not write storage.
Confirming or turning a page commits in the input worker after audio quiesces;
a failed save leaves that action available to retry. Exiting remains possible
even if saving fails. Re-entering the type previews the last selected story,
and OK resumes its own bookmark; reboot during reading reopens the saved page.
Other stories' bookmarks and the existing fortune/album/audio records survive.

Version 1 Slow Mail records migrate to the corresponding page in the first
story, with a sent receipt mapping to the next letter and a finished story
remaining complete. The linear revision replaces reply branches. Unknown or
corrupt records are preserved; the player can still read for the session with
an explicit unsaved-progress notice. No fallback erases NVS.

Pure model tests cover all pages, shuffled cycles, independent bookmarks,
completion/reread, every valid old checkpoint, and rejected corruption.
Application tests exercise the real input worker with failed commits, audio
quiescence, navigation and collection isolation. LVGL tests render every story
page and preview with real fonts and check bounds, overlap and actual pixels.
Simulator integration tests restart a separate process to verify bookmarks.
The 108 by 58 RGB565 buffer remains 12,528 bytes, the progress model is six
bytes and view state four bytes; no new image buffer or task is introduced.

On October 6, 2026, the complete ESP-IDF 5.5.3 gate and actual LVGL/simulator
checks passed. The application is 1,535,024 bytes; the merged image is 1,600,560
bytes, 3,584 more than the previous Slow Mail build. The verified archive is
`build/firmware/2c64f284accd4c19450639b7e6c3efb91610213c8e5cf4a252fb389b192181ae/`.
Full-image SHA256: `2c64f284accd4c19450639b7e6c3efb91610213c8e5cf4a252fb389b192181ae`.
Matching ELF SHA256: `4d719a93eecbfd73e910becd328aadf855c63f872ac845565c34bd12a9b4e967`.

Following the user's request, the three components were flashed at `0x0`,
`0x8000` and `0x10000`; all write hashes matched. The partition table is
unchanged and NVS/PHY were not written. A 20-second observation matched the
version and ELF, loaded 2200 records and recorded one boot without warnings,
crashes or rejected saves. Free heap was 216,520 bytes and the largest block
114,688 bytes; the serial monitor was then closed. Build: PASS; Host tests:
PASS; Device tests: PASS for writing and startup only. Physical button feel,
readability, sound, interrupted-save recovery, repeated-use heap and idle/wake
remain unverified. This build was submitted to project 914 as revision 2134 on October 6, 2026.
The revision is pending review; public revision remains 2072. Its matching
firmware source is `3e6522e`, and publication adds no physical-test evidence.

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
The 455-byte collection save imports the preceding 323-byte save and original 312-byte save with
corpus fingerprint 3420887641. Existing current/pinned records use a separate
legacy ID namespace, preserving their exact words and art. The 440 retained
records import their old seen bits; new content starts unread. Old mood filters
become reassurance, chance remains chance, and the artwork cursor is retained.
The preceding 2200-record fingerprint 3335999125 also migrates: unchanged
texts retain their read status; 312 retired texts use a compact compatibility
bank for current and pinned cards only. Their exact words and artwork survive;
new replacement texts start unread. Saved voice preferences are retired and
reset to unrestricted; the theme and artwork cursor remain.
The old banks serve existing cards only, never new draws. Separate NVS mute and
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
| Small insights | 220 | 10% |
| Relationship moments | 220 | 10% |
| Small outings | 110 | 5% |

Whole-bank mode shuffles every record together without theme quotas. Poetry
accounts for 10% of the full bank, but a short run may contain zero or several
poems. Seen records are skipped across topic changes and imported saves. The
voice filter is retired. Returning home or restarting restores the whole-bank
scope without resetting read history or the saved signature. An optional topic
requires confirmation on a separate page with a 20 px topic label. Exhaustion
requires changing topics or explicit reshuffling.
All 220 former personal declarations and 92 first-person humor openings were
rewritten. First-person openings fell from 320 to 21, including three unchanged
poetry excerpts; insights now cover concrete opinions across multiple subjects.

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
Host checks roundtrip 2200 current and 2112 legacy strings, all 2200 preceding
version cards through migration, theme proportions,
and all 220 poetry sources. The 10001 `long_term_target` is a target, not content.

| Resource | Encoded size |
| --- | ---: |
| Current UTF-8 including terminators | 105154 bytes |
| Current 11-bit character stream | 47188 bytes |
| Current dictionary | 3812 bytes |
| Current index and metadata | 17600 bytes |
| Current encoded text subtotal | **68600 bytes / 67.0 KiB** |
| Attribution strings and 32-bit pointers | 3744 bytes |
| Old seen-record mapping | 4400 bytes |
| Legacy text compatibility bank | **58686 bytes / 57.3 KiB** |
| Retired 312-record bank and previous-ID mapping | **18978 bytes / 18.5 KiB** |
| Theme offsets and counts | 36 bytes |
| Read-only content total, before linker alignment | **154444 bytes / 150.8 KiB** |
| Internal-RAM pixel canvas | **12528 bytes** |
| NVS state including seen bitset and CRC | **455 bytes** |
| Separate sound preferences | **Two one-byte values (mute and volume)** |
| Previous community application image | **1513840 bytes** |
| Previous verified merged image | **1579376 bytes** |

Only one record is decoded into a 128-byte buffer; the entire bank is never
loaded into RAM. The limit is 16384 records. Noto Sans CJK SC and
`lv_font_conv 1.5.3` generate 12/20 px, 4 bpp fonts with **2312 characters per
size**, covering current/legacy text, attribution, and UI literals. The OTF,
1920 gallery PNGs, and audition WAVs are excluded from firmware. Sources and
regeneration are in `assets/README.md`.

## Firmware and controls

The Mac/default correction uses `codex/mac-simulator`, based on `fff9cf9`,
preserving earlier changes. BSP is reused unchanged. Demo screens
are not linked. Target remains ESP32-C3, 8 MB Flash, no PSRAM, ESP-IDF 5.5.3,
LVGL 9.5.0 and the default minimal NVS/PHY/factory partition layout.

| Page | UP | DOWN | OK | Long press |
| --- | --- | --- | --- | --- |
| Whole-bank home | Open type selector | Open type selector | Whole-bank draw | OK opens collection; UP reshuffles only after exhaustion |
| Type selector | Previous type | Next type | Confirm type / preview letters | OK/DOWN cancels to whole-bank home; UP reshuffles an exhausted selection |
| Story preview | Random next story | New appearance | Select / resume | OK returns home; UP volume; DOWN sound |
| Story reading | Previous page | New appearance | Next page / finish | OK returns home; UP volume; DOWN sound |
| Opening letter | Ignored | Ignored | Skip animation | OK also skips |
| Draw result | Draw again | New appearance | Collect and display signature | UP opens volume; DOWN toggles sound; OK returns to whole-bank home |
| Saved signature | Draw again, keep old pin | New appearance for saved card | Open collection | UP opens volume; DOWN toggles sound; OK returns to whole-bank home |

A keyed Feistel permutation shuffles the entire text bank. A persistent seen
bitset avoids repeats across topic changes and preserves progress when upgrading
from the earlier quota-based order. No per-topic schedule or RAM shuffle array
is allocated. Reset starts a newly keyed order. Exhaustion requires explicit
reshuffling. Appearance IDs have their
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
for old saves; invalid/out-of-range volume values also use 80%. The 455-byte card format imports existing signatures. Mute preserves the selected volume.
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
defaults, return-to-chance controls, stop timeouts and failed-save silence.
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
Historical device tests PASS for write/startup: `38ad745` was flashed using its verified
components, with three matching write hashes and the unchanged partition table.
NVS was not written. Twenty seconds of observation match ELF `1cce326bad3`,
2200 records, 216576 free heap bytes and a 114688-byte largest block; no crash
or rejected state was observed. Full-image SHA256:
`288d57d659e3a99993527f98567ba75e0b693e54cbf6d1d7a574b65751d49766`.
Physical poetry/chance behavior, saved-signature identity, screen legibility
and controls still require user acceptance; startup logs cannot establish them.
Earlier eight-theme `15621c4` passed startup, but player feedback exposed the
hidden voice filter and declaration-heavy content, motivating this correction.
Current raw evidence is kept in ignored `build/mixing/`; binaries stay out of Git.

Sound, visual and interaction acceptance
still require user observation. Pending acceptance: speaker loudness, crackles and
reveal synchronization, rapid draw/skip/mute/save sequences, persisted mute, physical buttons and repeat presses,
Chinese legibility/corner clipping, opening/wipe smoothness, pin/restart and power-cut
behavior, idle dim/off/wake, battery reporting, runtime heap, current and battery
life. Any later flash requires applicable user authorization. The 10001+ expansion and
reader review of tone/repetition are outside the completed 2000+ milestone.

The default/selector firmware update `00878f6` is submitted as community revision
1967, now approved and public. Its verified component images
were flashed at 0x0, 0x8000 and 0x10000, with NVS and PHY excluded. All three write
hashes matched. Twenty seconds of startup match ELF `d248cefc0dc`, 2200 records,
216840 free heap bytes and a 114688-byte largest block; no crash or rejected state
was observed. Device tests PASS for writing and startup only; physical readability,
controls, saved-card identity and sound still need player acceptance. Full-image SHA256:
`f7a38b3c91be059686ba0a413a595ee85a1bfab5603555d580e3d31a09977b34`.
Current evidence lives in ignored `build/publish-simulator/`. The [Mac simulator](fortune-simulator.md)
uses the actual application input, storage, model and LVGL renderer, and shows
draw scope and categorized history. It is distributed in the source repository.

## Sixteen favorites

This update collects up to 16 quote/artwork pairs. Result OK
collects and displays the signature; home long OK or signature OK opens the album.
UP/DOWN browses without writing storage or changing the signature. OK opens actions
to display, change artwork or remove a bookmark. Removing it keeps the signature.
Recollecting a quote updates its artwork without using another slot. When full,
select an old card, preview the new one, then explicitly confirm; cancel is the default.
Long OK steps back through each page. Storage failure keeps the original state and
page for retry. Showcase artwork changes synchronize with its saved bookmark.

FTC2 state uses 455 bytes, adding 132 bytes. It retains read bits, cards and CRC,
then adds count/reserved bytes and sixteen 8-byte records. Importing previous
FTC1 or old-bank 323/312-byte saves moves the exact signature into slot one.
No signature yields an empty album. Invalid count, duplicate IDs, invalid cards,
wrong length and CRC are rejected without erasing NVS. Older firmware cannot
read the new format; arbitrary future bank changes/downgrades are not supported.
Partitions and independent sound keys are unchanged.

Sixteen drawn bookmarks show capacity/selection on the existing canvas. A turn
uses eight 25 ms steps from a 10-pixel offset and 60% text opacity (200 ms).
Successful collection shows a bookmark seal for twelve 40 ms steps (480 ms).
Rapid reversal and navigation clear previous offsets. No full-screen buffer or
animation-time Flash writes are added. Host checks cover 0..16 cards, every
index/action/confirmation, active fonts, bounds, animation interruption, old-save
migration, full/cancel/replace, failed-save retries and restart. Chinese rendering,
physical buttons, frame timing, interrupted writes and endurance await device testing.

Validation: Build PASS (ESP-IDF 5.5.3 complete gate, merged image and archive
verification); Host tests PASS (static gate, every LVGL layout/artwork and five
simulator integration checks). The application is 1519760 bytes and merged image
1585296 bytes, each 5920 bytes above the preceding community build. ELF application
state is 444 bytes (formerly 316), storage buffer 455 bytes (formerly 323), and
pixel canvas remains 12528 bytes. UI retains six labels with no extra screen buffer.
Merged SHA256: `a74ee55cb317b5fdb3ae642cd8a9d83b4bd62045b700910df54e996765ece6c5`.
Matching ELF SHA256: `b99fb1e999b409e16ad83173beeb892d315ca6cde0c8c71af3b5743c4c667bbf`.
Device tests PASS for writing and startup only: following explicit user approval,
the verified components were written at 0x0, 0x8000 and 0x10000. All three write
hashes matched; the unchanged partition table and segmented writes excluded NVS
and PHY. A 20-second startup observation matched ELF `b99fb1e99`, with 2200 records,
216684 free heap bytes and a 114688-byte largest block. One boot was observed,
with no crash or rejected state. The serial monitor is closed. Evidence is kept
in ignored `build/favorites/`. Unverified: physical controls, saved-signature/first
favorite identity, Chinese readability, animation timing, sound, interrupted
writes and endurance still require player or instrument checks.

This update is submitted to existing community project 914 as revision 2072,
pending review; public revision 1967 remains available. Firmware source commit
`22ef0dd` is on the public `alienzhou/one-line` main branch. The submission uses
the verified merged image above, the inspected existing cover and four completed
application-rendered detail panels. Bilingual descriptions, five-step instructions
and this update's release notes are submitted as separate fields. The server
confirmed the image hash and all text fields. Publication does not add physical
acceptance evidence. Local receipts are kept in ignored `build/publish-favorites/`.
