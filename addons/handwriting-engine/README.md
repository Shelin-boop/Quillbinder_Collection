# The package "Handwriting Engine" (0.99.0)

ONNX Runtime (Microsoft's own build for Linux x64, the native library of the Maven Central package
`com.microsoft.onnxruntime:onnxruntime`, `tools/get-onnxruntime.sh`, the pack `onnxruntime`) as a package
of its own, installed from the settings page of Handwriting, never by default. The program opens
`engine/lib/libonnxruntime.so.1` with QLibrary and calls `OrtGetApiBase` (`qb::ai::onnxRuntimeLibrary()` in
`src/core/AiEngine.h` finds it); there is no wrapper of ours and no link-time dependency.

`stage.sh` leaves the package out with a message while the pack is not there. The Maven Central package carries Microsoft's
third-party notices but not ONNX Runtime's own MIT text: that is in `payload/engine/licenses/LICENSE-onnxruntime.txt`.
`tools/check-engine-package.py` checks a finished package.
