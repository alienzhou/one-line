[简体中文](fortune-simulator.zh_CN.md) · **English**

# One Fortune on a Mac

Double-click `tools/Launch-Fortune-Simulator.command` in Finder, or run:

```bash
python3 tools/fortune_simulator/server.py
```

The launcher builds the desktop adapter and opens `http://127.0.0.1:8786/`.
Keep its terminal running; stop it with Control-C. Apple Command Line Tools
and the managed LVGL dependency are required. If LVGL is absent, prepare
ESP-IDF 5.5.3 and build this checkout once. CMake and Ninja are reused when
available; otherwise the launcher installs pinned versions into an isolated
environment under `build/fortune-simulator/tooling`.

## Try the actual controls

Click the three buttons, or use Up, Down and Enter. Hold a key for 0.5 seconds
for a long press; separate long-press buttons also work.

1. On home, OK draws randomly from all 2,200 records. UP/DOWN opens a separate
   topic selector with a large topic label. Browse there and press OK to apply
   the topic and draw; hold OK to cancel and return to the whole bank.
2. Wait for the letter to open, or press OK to skip. On the result, UP draws
   again, DOWN changes the artwork, and OK collects and displays the signature.
3. Hold OK to return home. Returning home or restarting restores the whole-bank
   mode while retaining read history and the saved signature.
4. Hold OK on home or press OK on the signature to open sixteen favorites. Browse
   with UP/DOWN; OK opens actions. Full-capacity replacement requires confirmation,
   with cancel selected by default.
5. On a card, hold UP for volume, adjust with UP/DOWN, then OK to save or hold
   OK to cancel. Hold DOWN on a card to toggle sound.

The page shows the actual draw scope, unread counts, the words, poetry
attribution, and recent draws. **Draw 20** uses the application button path
and skips opening animations. It retains the current scope and saved signature.
Choosing a topic in the desktop diagnostic control opens the selector; the
topic takes effect when the draw is confirmed.

## Compare and reproduce

The bank contains 220 poetry excerpts (10%). Whole-bank mode shuffles all
records together without category quotas or repeats within a cycle. A short
sample can contain no poetry or several poems; two poems per 20 draws is no
longer guaranteed. A topic-specific session continues in that topic until
returning home. The selector preview alone never applies a topic.

New blank simulation resets only the local simulator state. Set a random seed
to reproduce a sequence with an empty history. For example, seed `42` starts
with four poetry cards in its first 20 whole-bank draws in this implementation.
Restart simulation retains sixteen favorites, the signature, artwork, read history, volume and
mute preference while returning the draw scope to the whole bank. Refreshing
the browser keeps the current session. Export includes all draw records, the
seed, source fingerprint and serialized simulator state for diagnosis.

## What is shared and what is simulated

The desktop executable compiles `main/main.c` directly, including its input
processor and storage codec, and links the actual model, corpus, LVGL UI,
pixel renderer, Chinese fonts and music generator. No second JavaScript draw
algorithm or browser font is used for the device screen.

Desktop adapters replace USB, the buttons, NVS storage, battery, audio device
and RTOS scheduling. Rendering is 240 by 320 RGB565 with the BSP's rounded
display mask; the browser enlarges it. Battery is a labeled simulated value.
Sound is the shared PCM score played through the Mac; codec loudness, audio
mailbox timing, hardware input thresholds, idle power behavior and device
performance still require on-device tests. The simulator never opens USB or
reads/writes device data. Local files stay under `build/fortune-simulator/`.

## Verification

```bash
python3 tools/fortune_simulator/server.py --build-only
python3 tests/test_fortune_simulator.py
./tools/test_fortune_ui.sh
./tools/validate.sh
```

The simulator integration checks compare batch draws with the real button
sequence, topic preview/confirm/cancel, whole-bank defaults after restart,
sixteen-slot capacity/cancel/replace/remove/restart, saved signature/volume/history, export, and local-only HTTP mutations. The
model gate covers complete nonrepeating decks and all poetry records. Host
tests do not establish hardware acceptance.
