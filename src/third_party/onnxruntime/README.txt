onnxruntime_c_api.h and onnxruntime_error_code.h, onnxruntime_ep_c_api.h: the C headers of ONNX Runtime 1.32.0 (Microsoft, MIT License, text in LICENSE), unchanged, from the
headers/ folder of com.microsoft.onnxruntime:onnxruntime-android:1.32.0 on Maven Central (SHA-256 of the AAR:
8e8dbb412ed4939675a157702f0c1e3621b436a645af7f7244536ac20bfad628), the same file that tools/get-onnxruntime.sh fetches.
Only the header is here: the program has no link-time dependency on ONNX Runtime. It loads libonnxruntime from the
installed Handwriting Engine with QLibrary and calls OrtGetApiBase (src/ui/handwriting/OnnxRecognizer.cpp).
