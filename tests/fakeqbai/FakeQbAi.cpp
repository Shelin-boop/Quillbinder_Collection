// A fake libqbai (0.99.0, suite v0990h): the C interface of tools/QbAi.h without any model, so that the program's side
// (core/AiEngine.*), the device choice, ModelMemory's registration, cancelling and the install helper are checked without
// the real engine. Built by the tests as <build>/tests/fake-ai-engine/libqbai.so.1.
//
//  - devices: the CPU and the cards of QBAI_FAKE_CARDS ("<name>,<total MB>,<free MB>[,i]" separated by ";", "i" for a
//    graphics part that shares the RAM); without it one card "Fake Card" with 4096 MB, 3072 MB free; "" for none
//  - model_memory: the sizes of the files
//  - load: a file that begins with "FAIL" does not load; one that begins with "NOGPU" does not load on a card
//  - transcribe: "[<language>] The quick brown fox jumps over the lazy dog." in two segments
//  - synthesize: a sine of 440 Hz at 24 kHz, 50 ms per character of the text, in pieces of 1200 samples (QBAI_FAKE_SLOW_MS
//    waits after each piece, so that a test can cancel)
//  - stretch: the samples resampled to 1/speed of their count
//  - output_*: a sound output that plays nothing and counts (qbai_fake_out_stats, qbai_fake_out_set_scale); QBAI_FAKE_NO_SOUND: none
//  - qbai_fake_stats(&loaded, &loads, &lastDevice, &lastThreads): what the tests look at (not part of the interface)
#define QBAI_BUILD 1
#include "tools/QbAi.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace {

thread_local std::string t_error;
std::mutex g_lock;
std::vector<qbai_device> g_devices;
std::atomic<int> g_threads{2};
std::atomic<int> g_loaded{0}, g_loads{0}, g_lastDevice{-1}, g_lastThreads{0};

int fail(int status, const char *text)
{
    t_error = text;
    return status;
}

void readDevices()
{
    std::lock_guard<std::mutex> g(g_lock);
    g_devices.clear();
    qbai_device cpu{};
    cpu.index = 0;
    cpu.type = QBAI_DEVICE_CPU;
    std::snprintf(cpu.id, sizeof cpu.id, "CPU");
    std::snprintf(cpu.name, sizeof cpu.name, "Fake Processor");
    cpu.memory_total = 8LL << 30;
    cpu.memory_free = 6LL << 30;
    g_devices.push_back(cpu);
    const char *env = std::getenv("QBAI_FAKE_CARDS");
    const std::string cards = env ? env : "Fake Card,4096,3072";
    size_t start = 0;
    while (start < cards.size()) {
        size_t end = cards.find(';', start);
        if (end == std::string::npos)
            end = cards.size();
        const std::string one = cards.substr(start, end - start);
        start = end + 1;
        char name[200] = {};
        long long total = 0, freeMB = 0;
        char kind[8] = {};
        const int n = std::sscanf(one.c_str(), "%199[^,],%lld,%lld,%7s", name, &total, &freeMB, kind);
        if (n < 3)
            continue;
        qbai_device d{};
        d.index = int(g_devices.size());
        d.type = (n == 4 && kind[0] == 'i') ? QBAI_DEVICE_IGPU : QBAI_DEVICE_GPU;
        std::snprintf(d.id, sizeof d.id, "Vulkan%d", d.index - 1);
        std::snprintf(d.name, sizeof d.name, "%s", name);
        d.memory_total = total * 1024 * 1024;
        d.memory_free = freeMB * 1024 * 1024;
        g_devices.push_back(d);
    }
}

int deviceCount(int refresh)
{
    {
        std::lock_guard<std::mutex> g(g_lock);
        if (!refresh && !g_devices.empty())
            return int(g_devices.size());
    }
    readDevices();
    std::lock_guard<std::mutex> g(g_lock);
    return int(g_devices.size());
}

int deviceGet(int index, qbai_device *out)
{
    deviceCount(0);
    std::lock_guard<std::mutex> g(g_lock);
    if (!out || index < 0 || index >= int(g_devices.size()))
        return fail(QBAI_BAD_ARGS, "no such device");
    *out = g_devices[size_t(index)];
    return QBAI_OK;
}

void setThreads(int n) { g_threads = std::max(1, n); }
int threads() { return g_threads.load(); }

std::string head(const char *path)
{
    std::string s;
    if (FILE *f = path ? std::fopen(path, "rb") : nullptr) {
        char buf[8] = {};
        const size_t n = std::fread(buf, 1, sizeof buf - 1, f);
        s.assign(buf, n);
        std::fclose(f);
    }
    return s;
}

int64_t fileSize(const char *path)
{
    if (!path || !*path)
        return 0;
    FILE *f = std::fopen(path, "rb");
    if (!f)
        return -1;
    std::fseek(f, 0, SEEK_END);
    const long n = std::ftell(f);
    std::fclose(f);
    return n;
}

int64_t modelMemory(int, const char *model, const char *extra, int)
{
    const int64_t a = fileSize(model), b = fileSize(extra);
    return a <= 0 || b < 0 ? 0 : a + b;
}

} // namespace

struct qbai_model {
    int kind = 0;
    int device = 0;
    int64_t bytes = 0;
    std::vector<std::string> voices, languages;
};

namespace {

qbai_model *load(const qbai_load_params *p)
{
    t_error.clear();
    if (!p || p->size < int(sizeof(qbai_load_params)) || !p->model_path) {
        fail(QBAI_BAD_ARGS, "bad load parameters");
        return nullptr;
    }
    if (fileSize(p->model_path) <= 0) {
        fail(QBAI_BAD_ARGS, "a model file is missing or empty");
        return nullptr;
    }
    if (p->device < 0 || p->device >= deviceCount(0)) {
        fail(QBAI_BAD_ARGS, "no such device");
        return nullptr;
    }
    const std::string h = head(p->model_path);
    if (h.rfind("FAIL", 0) == 0) {
        fail(QBAI_ERROR, "the fake model says it cannot load");
        return nullptr;
    }
    if (h.rfind("NOGPU", 0) == 0 && p->device > 0) {
        fail(QBAI_NO_MEMORY, "the fake card has no room for it");
        return nullptr;
    }
    auto *m = new qbai_model;
    m->kind = p->kind;
    m->device = p->device;
    m->bytes = modelMemory(p->kind, p->model_path, p->extra_path, p->device);
    if (p->kind == QBAI_TTS_POCKET)
        m->voices = {"alba", "marius"};
    if (p->kind == QBAI_TTS_QWEN && h.rfind("CUSTOM", 0) == 0)
        m->voices = {"ryan", "vivian"};
    m->languages = p->kind == QBAI_TTS_QWEN ? std::vector<std::string>{"english", "german"}
                                            : std::vector<std::string>{"en", "de"};
    g_loaded++;
    g_loads++;
    g_lastDevice = p->device;
    g_lastThreads = p->threads > 0 ? p->threads : g_threads.load();
    return m;
}

void unload(qbai_model *m)
{
    if (!m)
        return;
    g_loaded--;
    delete m;
}

int modelInfo(qbai_model *m, qbai_model_info *out)
{
    if (!m || !out || out->size < int(sizeof(qbai_model_info)))
        return fail(QBAI_BAD_ARGS, "bad arguments");
    const int size = out->size;
    std::memset(out, 0, sizeof(qbai_model_info));
    out->size = size;
    out->kind = m->kind;
    out->device = m->device;
    out->bytes = m->bytes;
    out->sample_rate = m->kind == QBAI_ASR_WHISPER ? 16000 : 24000;
    out->voices = int(m->voices.size());
    out->languages = int(m->languages.size());
    out->multilingual = 1;
    out->voice_clone = m->kind == QBAI_TTS_QWEN && m->voices.empty() ? 1 : 0;
    std::snprintf(out->type, sizeof out->type, "%s", m->kind == QBAI_ASR_WHISPER ? "whisper" : m->kind == QBAI_TTS_POCKET ? "pocket-tts" : "base");
    return QBAI_OK;
}

const char *voiceName(qbai_model *m, int i) { return m && i >= 0 && i < int(m->voices.size()) ? m->voices[size_t(i)].c_str() : nullptr; }
const char *languageName(qbai_model *m, int i) { return m && i >= 0 && i < int(m->languages.size()) ? m->languages[size_t(i)].c_str() : nullptr; }

int transcribe(qbai_model *m, const float *pcm, int64_t samples, const qbai_transcribe_params *p, char **text)
{
    if (text)
        *text = nullptr;
    if (!m || m->kind != QBAI_ASR_WHISPER || !pcm || samples <= 0 || !p || p->size < int(QBAI_TRANSCRIBE_V1_SIZE))
        return fail(QBAI_BAD_ARGS, "bad arguments");
    const bool v2 = p->size >= int(sizeof(qbai_transcribe_params));
    const std::string lang = p->language ? p->language : "auto";
    const std::string first = "[" + lang + "] The quick brown fox", second = " jumps over the lazy dog.";
    if (p->cancel && p->cancel(p->user))
        return fail(QBAI_CANCELLED, "cancelled");
    if (p->progress)
        p->progress(50, p->user);
    if (p->segment)
        p->segment(first.c_str(), 0, 1500, p->user);
    if (p->cancel && p->cancel(p->user))
        return fail(QBAI_CANCELLED, "cancelled");
    if (p->segment)
        p->segment(second.c_str(), 1500, 3000, p->user);
    if (p->progress)
        p->progress(100, p->user);
    std::string all = first + second + (p->translate ? " (in English)" : "");
    if (v2 && p->beam_size > 1)
        all += " (beam " + std::to_string(p->beam_size) + ")";
    if (v2 && p->no_speech_max > 0.f)
        all += " (no-speech limit)";
    if (text) {
        *text = static_cast<char *>(std::malloc(all.size() + 1));
        std::memcpy(*text, all.c_str(), all.size() + 1);
    }
    return QBAI_OK;
}

void freeText(char *t) { std::free(t); }

// QBAI_FAKE_LANG=<code>: that language gets 0.9, the others 0.05 (default "en"); QBAI_FAKE_LANG_CALLS counts in g_detects
std::atomic<int> g_detects{0};
int detectLanguage(qbai_model *m, const float *pcm, int64_t samples, const char *const *codes, int count, float *probs)
{
    if (!m || m->kind != QBAI_ASR_WHISPER || !pcm || samples <= 0 || !codes || count <= 0 || !probs)
        return fail(QBAI_BAD_ARGS, "bad arguments");
    ++g_detects;
    const char *env = std::getenv("QBAI_FAKE_LANG");
    const std::string want = env && *env ? env : "en";
    for (int i = 0; i < count; ++i)
        probs[i] = want == codes[i] ? 0.9f : 0.05f;
    return QBAI_OK;
}

bool putPcm(qbai_pcm *out, const float *data, int64_t n, int rate)
{
    out->samples = static_cast<float *>(std::malloc(size_t(std::max<int64_t>(n, 1)) * sizeof(float)));
    if (!out->samples)
        return false;
    if (n > 0)
        std::memcpy(out->samples, data, size_t(n) * sizeof(float));
    out->count = n;
    out->rate = rate;
    return true;
}

void freePcm(qbai_pcm *pcm)
{
    if (!pcm)
        return;
    std::free(pcm->samples);
    pcm->samples = nullptr;
    pcm->count = 0;
}

int synthesize(qbai_model *m, const qbai_synth_params *p, qbai_pcm *out)
{
    if (out) {
        out->samples = nullptr;
        out->count = 0;
        out->rate = 0;
    }
    if (!m || !p || p->size < int(sizeof(qbai_synth_params)) || !p->text || !*p->text || (!p->chunk && !out))
        return fail(QBAI_BAD_ARGS, "bad arguments");
    if (m->kind == QBAI_ASR_WHISPER)
        return fail(QBAI_BAD_ARGS, "the model does not speak");
    const qbai_voice *v = p->voice;
    if (v && v->preset && *v->preset) {
        bool known = false;
        for (const std::string &s : m->voices)
            known = known || s == v->preset;
        if (!known)
            return fail(QBAI_BAD_ARGS, "the fake model has no such voice");
    }
    const int rate = 24000;
    const int64_t total = std::max<int64_t>(int64_t(std::strlen(p->text)) * rate / 20, rate / 10);
    const char *slow = std::getenv("QBAI_FAKE_SLOW_MS");
    const int waitMs = slow ? std::atoi(slow) : 0;
    std::vector<float> all;
    std::vector<float> piece;
    for (int64_t done = 0; done < total;) {
        if (p->cancel && p->cancel(p->user))
            return fail(QBAI_CANCELLED, "cancelled");
        const int64_t n = std::min<int64_t>(1200, total - done);
        piece.resize(size_t(n));
        for (int64_t i = 0; i < n; ++i)
            piece[size_t(i)] = 0.3f * float(std::sin(2.0 * M_PI * 440.0 * double(done + i) / rate));
        done += n;
        if (p->chunk) {
            if (p->chunk(piece.data(), n, p->user) == 0)
                return fail(QBAI_CANCELLED, "cancelled");
        } else {
            all.insert(all.end(), piece.begin(), piece.end());
        }
        if (waitMs > 0)
            std::this_thread::sleep_for(std::chrono::milliseconds(waitMs));
    }
    if (!p->chunk && !putPcm(out, all.data(), int64_t(all.size()), rate))
        return fail(QBAI_NO_MEMORY, "out of memory");
    return QBAI_OK;
}

} // namespace

struct qbai_stretcher {
    int rate = 0;
    float speed = 1;
    std::vector<float> in;
};

namespace {

std::vector<float> resampled(const std::vector<float> &in, float speed)
{
    const int64_t n = int64_t(in.size()), m = int64_t(std::llround(double(n) / speed));
    std::vector<float> out(size_t(std::max<int64_t>(m, 0)));
    for (int64_t i = 0; i < m; ++i)
        out[size_t(i)] = in[size_t(std::min<int64_t>(int64_t(double(i) * speed), n - 1))];
    return out;
}

qbai_stretcher *stretchBegin(int rate, float speed)
{
    if (rate <= 0 || !(speed >= 0.5f && speed <= 4.0f)) {
        fail(QBAI_BAD_ARGS, "speed must be between 0.5 and 4.0");
        return nullptr;
    }
    auto *s = new qbai_stretcher;
    s->rate = rate;
    s->speed = speed;
    return s;
}

int stretchFeed(qbai_stretcher *s, const float *pcm, int64_t samples, qbai_pcm *out)
{
    if (!s || !out)
        return fail(QBAI_BAD_ARGS, "bad arguments");
    s->in.insert(s->in.end(), pcm, pcm + samples);
    return putPcm(out, nullptr, 0, s->rate) ? QBAI_OK : QBAI_NO_MEMORY;   // all of it at the end
}

int stretchEnd(qbai_stretcher *s, qbai_pcm *out)
{
    if (!s)
        return fail(QBAI_BAD_ARGS, "bad arguments");
    const std::vector<float> r = resampled(s->in, s->speed);
    const bool ok = !out || putPcm(out, r.data(), int64_t(r.size()), s->rate);
    delete s;
    return ok ? QBAI_OK : QBAI_NO_MEMORY;
}

int stretch(const float *pcm, int64_t samples, int rate, float speed, qbai_pcm *out)
{
    if (!out || (samples > 0 && !pcm))
        return fail(QBAI_BAD_ARGS, "bad arguments");
    if (rate <= 0 || !(speed >= 0.5f && speed <= 4.0f))
        return fail(QBAI_BAD_ARGS, "speed must be between 0.5 and 4.0");
    const std::vector<float> r = resampled(std::vector<float>(pcm, pcm + samples), speed);
    return putPcm(out, r.data(), int64_t(r.size()), rate) ? QBAI_OK : QBAI_NO_MEMORY;
}

// ------------------------------------------------------------------ the sound output: nothing is played; the clock runs
// `scale` times faster than real time (qbai_fake_out_set_scale; default 1; 0 stands still) over what was written
} // namespace

struct qbai_output {
    int rate = 0, channels = 1;
    int64_t written = 0;
    double played = 0;   // frames
    bool paused = false, stopped = false;
    std::chrono::steady_clock::time_point last = std::chrono::steady_clock::now();
};
namespace {

std::atomic<double> g_outScale{1.0};
std::atomic<int> g_outOpens{0};
std::atomic<long long> g_outWritten{0};
std::atomic<int> g_outPaused{0}, g_outStopped{0}, g_outRate{0}, g_outLive{0};
std::mutex g_outMutex;

void outAdvance(qbai_output *o)
{
    const auto now = std::chrono::steady_clock::now();
    const double sec = std::chrono::duration<double>(now - o->last).count();
    o->last = now;
    if (!o->paused)
        o->played = std::min<double>(double(o->written), o->played + sec * g_outScale.load() * o->rate);
}
qbai_output *outputOpen(int rate, int channels)
{
    if (rate <= 0 || channels < 1) {
        fail(QBAI_BAD_ARGS, "bad sound output parameters");
        return nullptr;
    }
    if (std::getenv("QBAI_FAKE_NO_SOUND")) {
        fail(QBAI_ERROR, "the fake has no sound output");
        return nullptr;
    }
    auto *o = new qbai_output;
    o->rate = rate;
    o->channels = channels;
    ++g_outOpens;
    ++g_outLive;
    g_outRate = rate;
    g_outWritten = 0;
    g_outPaused = 0;
    g_outStopped = 0;
    return o;
}
int outputWrite(qbai_output *o, const float *pcm, int64_t frames)
{
    if (!o || frames < 0 || (frames > 0 && !pcm))
        return fail(QBAI_BAD_ARGS, "bad arguments");
    std::lock_guard<std::mutex> g(g_outMutex);
    outAdvance(o);
    o->written += frames;
    g_outWritten += frames;
    return QBAI_OK;
}
int outputPause(qbai_output *o)
{
    if (!o) return fail(QBAI_BAD_ARGS, "bad arguments");
    std::lock_guard<std::mutex> g(g_outMutex);
    outAdvance(o);
    o->paused = true;
    g_outPaused = 1;
    return QBAI_OK;
}
int outputResume(qbai_output *o)
{
    if (!o) return fail(QBAI_BAD_ARGS, "bad arguments");
    std::lock_guard<std::mutex> g(g_outMutex);
    outAdvance(o);
    o->paused = false;
    g_outPaused = 0;
    return QBAI_OK;
}
int outputStop(qbai_output *o)
{
    if (!o) return fail(QBAI_BAD_ARGS, "bad arguments");
    std::lock_guard<std::mutex> g(g_outMutex);
    o->written = 0;
    o->played = 0;
    g_outStopped = 1;
    return QBAI_OK;
}
void outputClose(qbai_output *o)
{
    if (!o) return;
    --g_outLive;
    delete o;
}
int64_t outputPlayed(qbai_output *o)
{
    if (!o) return 0;
    std::lock_guard<std::mutex> g(g_outMutex);
    outAdvance(o);
    return int64_t(o->played);
}

const char *engineVersion() { return "fake qbai 1 (tests)"; }
const char *lastError() { return t_error.c_str(); }

qbai_api makeApi(int version, int size)
{
    qbai_api a = {
        version, size, engineVersion, lastError, deviceCount, deviceGet, setThreads, threads, modelMemory, load, unload,
        modelInfo, voiceName, languageName, transcribe, freeText, synthesize, freePcm, stretch, stretchBegin, stretchFeed, stretchEnd,
        detectLanguage, outputOpen, outputWrite, outputPause, outputResume, outputStop, outputClose, outputPlayed,
    };
    return a;
}
const qbai_api kApi1 = makeApi(1, int(QBAI_API_V1_SIZE));
const qbai_api kApi2 = makeApi(2, int(sizeof(qbai_api)));

} // namespace

extern "C" const qbai_api *qbai_get_api(int version)
{
    if (version < 1 || version > QBAI_API_VERSION)
        return nullptr;
    return version == 1 ? &kApi1 : &kApi2;
}

extern "C" __attribute__((visibility("default"))) int qbai_fake_detects() { return g_detects.load(); }

extern "C" __attribute__((visibility("default"))) void qbai_fake_stats(int *loaded, int *loads, int *lastDevice, int *lastThreads)
{
    if (loaded)
        *loaded = g_loaded.load();
    if (loads)
        *loads = g_loads.load();
    if (lastDevice)
        *lastDevice = g_lastDevice.load();
    if (lastThreads)
        *lastThreads = g_lastThreads.load();
}

// what the checks look at: the number of opens, frames written since the last open, 1 while paused, 1 after a stop, the rate,
// the outputs still open; and the speed of the clock
extern "C" __attribute__((visibility("default"))) void qbai_fake_out_stats(int *opens, long long *written, int *paused, int *stopped, int *rate, int *live)
{
    if (opens) *opens = g_outOpens.load();
    if (written) *written = g_outWritten.load();
    if (paused) *paused = g_outPaused.load();
    if (stopped) *stopped = g_outStopped.load();
    if (rate) *rate = g_outRate.load();
    if (live) *live = g_outLive.load();
}
extern "C" __attribute__((visibility("default"))) void qbai_fake_out_set_scale(double scale) { g_outScale = scale; }
