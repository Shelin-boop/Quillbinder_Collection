# The package "AI Engine" (0.99.0)

One package per system with the engine of the AI functions that run on the writer's computer: ggml with its compute
modules (one per processor generation, and Vulkan for graphics cards where the package has it), whisper.cpp, llama.cpp's
libraries (libllama, libmtmd: Pocket TTS), qwentts.cpp (Qwen3-TTS), Sonic, and Quillbinder's wrapper libqbai
(`engines/qbai`, the C interface `src/tools/QbAi.h`). The program opens libqbai with QLibrary when an AI function is used;
it has no link-time dependency on any of it. The package is installed from the settings pages of the functions that need
it (`src/ui/AiInstallPanel.h`), never by default.

- `stage.sh` puts the libraries of `tools/get-ai-engine.sh` (the pack `aiengine`) and libqbai into the package
  (`packaging/linux/build-addons.sh`); without them the package is left out.
- `payload/engine/licenses/`: the license texts of the code that the libraries hold besides their own (taken from the
  pinned sources); `stage.sh` adds the libraries' own license files and a `README.txt` that names every library.
- `tools/check-engine-package.py` checks a finished package: the expected libraries, and that each needs nothing outside
  the package and the base system.
