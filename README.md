# tmg1-esp32-demo

**English** | [日本語](README.ja.md)

A reference **ESP32 / Arduino** player for the **TMG1** 1-bit-per-pixel
(bitplane) video format. It shows how to use the shared decoder library
[`tmg1-codec`](https://github.com/tmg1-labs/tmg1-codec) on a device: it decodes
`.tmg1` files from LittleFS and streams the frames to an SSD1306 128x64 I2C OLED
through [U8g2](https://github.com/olikraus/u8g2).

The decoding itself is **not** implemented here — it lives in `tmg1-codec`, which
is an Arduino library in its own right (it ships a `library.properties` and the
`tmg1/arduino_stream.h` adapter). This repository is the **example**:
[src/main.cpp](src/main.cpp) is a sample sketch, and the codec is pulled in via
PlatformIO `lib_deps` pinned to a git tag (`#v0.2.0`), so the on-device decoder
always matches a known-good release of the format.

## Using the codec in your own project

To decode TMG1 in your own Arduino / PlatformIO sketch, depend on **`tmg1-codec`**
(not on this repository). With PlatformIO, add it to `lib_deps`:

```ini
lib_deps =
    https://github.com/tmg1-labs/tmg1-codec.git
    olikraus/U8g2@^2.36.15   ; only if you draw to a U8g2 display
```

Then wrap any Arduino `Stream` / `File` as a `Tmg1Stream` and pull frames out one
at a time:

```cpp
#include <LittleFS.h>
#include "tmg1/decoder.h"
#include "tmg1/arduino_stream.h"

tmg1::Decoder decoder;

void setup() {
  LittleFS.begin();
  File f = LittleFS.open("/video.tmg1", "r");

  // Wrap the Arduino File as a codec stream and read the header.
  Tmg1Stream stream = tmg1_stream_from_arduino(f);
  decoder.begin(stream);

  const size_t frameSize = (decoder.getWidth() * decoder.getHeight() + 7) / 8;
  uint8_t* buf = new uint8_t[frameSize];

  // Each call yields one frame: ceil(w*h/8) bytes of an MSB-first 1bpp bitplane.
  while (decoder.decodeFrame(buf, frameSize) == tmg1::Error::None) {
    // ... draw buf, then pace by decoder.getLastPtsDelta() and the timebase ...
  }
}
```

See [src/main.cpp](src/main.cpp) for the full version (OLED output, FPS overlay,
microsecond-precision frame pacing, and looping). Produce `.tmg1` files from
video with [`tmg1-cli`](https://github.com/tmg1-labs/tmg1-cli).

## The example player

[src/main.cpp](src/main.cpp) demonstrates a complete player:

- **Streaming decode** — frames are read from LittleFS and decoded one at a time
  through `Tmg1Stream`, so a whole video never has to fit in RAM.
- **OLED output with FPS overlay** — decoded bitplanes are drawn with
  `u8g2.drawXBMP`, with a live frames-per-second counter in the corner.
- **VFR-aware timing** — playback paces each frame from the stream's `ptsDelta`
  and timebase with microsecond precision, so variable-frame-rate streams play
  at the right speed.
- **Loop playback** — at end-of-stream the decoder re-opens the file and
  restarts.

### Hardware

- An ESP32 / ESP32-C3 board.
- An SSD1306 128x64 I2C OLED.

> **Display polarity:** on the target OLED, `drawXBMP` treats bit `1` as the
> drawing color, which makes the panel render black/white inverted. The sample
> therefore expects the video to be color-inverted (ffmpeg `negate`) at encode
> time, and the codec/decoder are left untouched. See the comment in
> [src/main.cpp](src/main.cpp) for details.

### Build

```bash
git clone https://github.com/tmg1-labs/tmg1-esp32-demo
cd tmg1-esp32-demo
pio run -e esp32dev            # ESP32 DevKit
pio run -e seeed_xiao-esp32c3  # Seeed XIAO ESP32-C3
pio run -e super-mini-k2       # Super Mini K2 (ESP32-C3)
```

Requirements:

- [PlatformIO](https://platformio.org/) (`pip install platformio`).
- `tmg1-codec` is fetched automatically from `lib_deps` (git tag) on first build —
  no submodule setup needed.

### Upload / Flash

```bash
pio run -e esp32dev -t upload     # flash the firmware
pio run -e esp32dev -t uploadfs   # flash the LittleFS image built from data/
pio device monitor -b 115200      # serial monitor
```

Place your `.tmg1` file in `data/`. The sample plays `/sample-video.tmg1` from
LittleFS (see [src/main.cpp](src/main.cpp)), so name the file accordingly or
adjust `videoFileName`.

## Tests

```bash
# PlatformIO native tests
pio test -e native -v
```

The codec's own unit tests (CMake + Unity) live in and are run by
[`tmg1-codec`](https://github.com/tmg1-labs/tmg1-codec); they are not duplicated here.

## Build & CI

CI runs on GitHub Actions (`.github/workflows/ci.yml`) with a single
`test_native` job (PlatformIO native tests). The codec is pulled from `lib_deps`
(git tag), so no submodule initialization is required.

## TMG1 Format

The authoritative byte-level layout of `.tmg1` lives in the standalone
[**TMG1 format specification**](https://github.com/tmg1-labs/.github/blob/main/docs/tmg1-format.md).
See [`tmg1-codec`](https://github.com/tmg1-labs/tmg1-codec) for the codec
internals and the C++/FFI API, and [`tmg1-cli`](https://github.com/tmg1-labs/tmg1-cli)
for encoding `.tmg1` files on the desktop.

## Related projects

Part of **[TMG1 Labs](https://github.com/tmg1-labs)** — see the organization
profile for all repositories in the project.

## License

MIT
