# miniaudio

`miniaudio.h` is miniaudio v0.11.26 (2026-10-31) by David Reid, https://miniaud.io, https://github.com/mackron/miniaudio,
unchanged. It is the same single header that llama.cpp's libmtmd vendors (llama.cpp b12808, `vendor/miniaudio/miniaudio.h`);
libmtmd keeps its copy's symbols to itself, so libqbai compiles its own (`engines/qbai/output.cpp`) for the sound output of
Read Aloud (`qbai_api::output_*` in `src/tools/QbAi.h`).

License: a choice of public domain (Unlicense) or MIT No Attribution, as the end of the file states; the text is also in the
AI engine package (`addons/ai-engine/payload/engine/licenses/miniaudio.txt`).

No new link-time dependency: on Linux miniaudio loads ALSA, PulseAudio (and, through PulseAudio's compatibility, PipeWire) at run
time with `dlopen`; on Windows it uses WASAPI, on Android AAudio or OpenSL ES.
