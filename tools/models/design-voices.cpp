// Designs voice clips with Qwen3-TTS VoiceDesign (the job "voices" of .github/workflows/models-5.yml, 0.99.0).
//
// One program that loads the model once and makes every clip of a list, through the public interface of qwentts.cpp
// (qwen.h; the library is built by tools/get-ai-engine.sh at its pinned commit). It is a tool of the release, not part of the
// program.
//
//   design-voices <talker.gguf> <tokenizer.gguf> <language> <list.tsv> <out dir> <first seed>
//
// <list.tsv>: one clip per line, three fields separated by a tab: the clip's name, the voice description (the instruction),
// the text to speak. Each clip is written as <out dir>/<name>.wav: mono, 24 kHz, 16 bit. The seed of the n-th clip is
// <first seed> + n, so that a run can be repeated. The exit code is 0 when every clip was made.
#include "qwen.h"

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

static void put32(std::string &s, uint32_t v) {
    for (int i = 0; i < 4; ++i)
        s.push_back(char((v >> (8 * i)) & 0xff));
}
static void put16(std::string &s, uint16_t v) {
    s.push_back(char(v & 0xff));
    s.push_back(char((v >> 8) & 0xff));
}

static bool writeWav(const std::string &path, const float *x, int n, int rate) {
    std::string h = "RIFF";
    put32(h, 36 + uint32_t(n) * 2);
    h += "WAVEfmt ";
    put32(h, 16);
    put16(h, 1);          // PCM
    put16(h, 1);          // mono
    put32(h, uint32_t(rate));
    put32(h, uint32_t(rate) * 2);
    put16(h, 2);
    put16(h, 16);
    h += "data";
    put32(h, uint32_t(n) * 2);
    std::string body;
    body.reserve(size_t(n) * 2);
    for (int i = 0; i < n; ++i) {
        float v = x[i] * 32767.0f;
        if (v > 32767.0f) v = 32767.0f;
        if (v < -32768.0f) v = -32768.0f;
        put16(body, uint16_t(int16_t(std::lrintf(v))));
    }
    std::ofstream f(path, std::ios::binary);
    f.write(h.data(), std::streamsize(h.size()));
    f.write(body.data(), std::streamsize(body.size()));
    return bool(f);
}

int main(int argc, char **argv) {
    if (argc != 7) {
        std::fprintf(stderr, "usage: %s <talker.gguf> <tokenizer.gguf> <language> <list.tsv> <out dir> <first seed>\n", argv[0]);
        return 2;
    }
    qt_init_params ip;
    qt_init_default_params(&ip);
    ip.talker_path = argv[1];
    ip.codec_path = argv[2];
    ip.device = "CPU";
    qt_context *q = qt_init(&ip);
    if (!q) {
        std::fprintf(stderr, "qt_init failed: %s\n", qt_last_error());
        return 1;
    }
    if (std::strcmp(qt_model_type(q), "voice_design") != 0) {
        std::fprintf(stderr, "the model is of type %s, not voice_design\n", qt_model_type(q));
        qt_free(q);
        return 1;
    }
    std::ifstream list(argv[4]);
    if (!list) {
        std::fprintf(stderr, "cannot read %s\n", argv[4]);
        qt_free(q);
        return 1;
    }
    const std::string outDir = argv[5];
    const long long seed0 = std::atoll(argv[6]);
    std::string line;
    int n = 0, failed = 0;
    while (std::getline(list, line)) {
        if (line.empty())
            continue;
        const size_t a = line.find('\t');
        const size_t b = a == std::string::npos ? a : line.find('\t', a + 1);
        if (b == std::string::npos) {
            std::fprintf(stderr, "bad line %d\n", n + 1);
            ++failed;
            continue;
        }
        const std::string name = line.substr(0, a), instruct = line.substr(a + 1, b - a - 1), text = line.substr(b + 1);
        qt_tts_params p;
        qt_tts_default_params(&p);
        p.text = text.c_str();
        p.lang = argv[3];
        p.instruct = instruct.c_str();
        p.seed = seed0 + n;
        p.max_new_tokens = 12 * 30;   // 30 s at 12 Hz: a clip never needs more, a runaway ends here
        qt_audio out{};
        const qt_status st = qt_synthesize(q, &p, &out);
        if (st != QT_STATUS_OK || !out.samples || out.n_samples <= 0 || out.sample_rate != 24000 || out.channels != 1) {
            std::fprintf(stderr, "%s: synthesis failed (%d): %s\n", name.c_str(), int(st), qt_last_error());
            qt_audio_free(&out);
            ++failed;
            ++n;
            continue;
        }
        if (!writeWav(outDir + "/" + name + ".wav", out.samples, out.n_samples, out.sample_rate)) {
            std::fprintf(stderr, "%s: cannot write\n", name.c_str());
            ++failed;
        } else {
            std::printf("%s %.2f s\n", name.c_str(), double(out.n_samples) / out.sample_rate);
        }
        qt_audio_free(&out);
        ++n;
    }
    qt_free(q);
    return failed ? 1 : 0;
}
