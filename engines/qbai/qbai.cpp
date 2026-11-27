// libqbai (0.99.0): the C interface of the AI engine package (src/tools/QbAi.h) over ggml, whisper.cpp, llama.cpp with
// libmtmd (Pocket TTS), qwentts.cpp (Qwen3-TTS) and Sonic. Built against the pack of tools/get-ai-engine.sh
// (engines/qbai/CMakeLists.txt); it lies beside those libraries in the package's engine/lib and finds them, and ggml's
// compute modules, in its own folder (RUNPATH $ORIGIN).
#define QBAI_BUILD 1
#include "tools/QbAi.h"
#include "qbai_parts.h"   // QBAI_PARTS: the versions of the pack's parts (engines/qbai/CMakeLists.txt)

#include <ggml-backend.h>
#include <ggml.h>
#include <llama.h>
#include <mtmd-helper.h>
#include <mtmd.h>
#include <qwen.h>
#include <sonic.h>
#include <whisper.h>

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <string>
#include <sys/stat.h>
#include <thread>
#include <vector>

#if !defined(_WIN32)
#include <dlfcn.h>
#endif

namespace {

// ------------------------------------------------------------------ errors and logs

thread_local std::string t_error;
thread_local std::string t_lastLog;   // the last error line a library logged on this thread (added to a failure's text)

int fail(int status, const std::string &text)
{
    t_error = text;
    if (!t_lastLog.empty() && t_error.find(t_lastLog) == std::string::npos)
        t_error += " (" + t_lastLog + ")";
    return status;
}

bool verbose()
{
    static const bool on = std::getenv("QBAI_LOG") && *std::getenv("QBAI_LOG") && std::strcmp(std::getenv("QBAI_LOG"), "0") != 0;
    return on;
}

void ggmlLog(enum ggml_log_level level, const char *text, void *)
{
    if (!text)
        return;
    if (level == GGML_LOG_LEVEL_ERROR) {
        std::string line(text);
        while (!line.empty() && (line.back() == '\n' || line.back() == ' '))
            line.pop_back();
        if (!line.empty())
            t_lastLog = line;
    }
    if (verbose() || level == GGML_LOG_LEVEL_ERROR)
        std::fputs(text, stderr);
}

void qwenLog(enum qt_log_level level, const char *msg, void *)
{
    if (!msg)
        return;
    if (level == QT_LOG_ERROR)
        t_lastLog = msg;
    if (verbose() || level == QT_LOG_ERROR)
        std::fprintf(stderr, "qwentts: %s\n", msg);
}

// ------------------------------------------------------------------ set-up: the compute modules, once

std::once_flag g_setup;
std::atomic<int> g_threads{0};

std::string ownFolder()
{
#if defined(_WIN32)
    return std::string();
#else
    Dl_info info{};
    if (dladdr(reinterpret_cast<void *>(&ggml_backend_load_all_from_path), &info) && info.dli_fname) {
        std::string path(info.dli_fname);
        const size_t slash = path.rfind('/');
        if (slash != std::string::npos)
            return path.substr(0, slash);
    }
    return std::string();
#endif
}

void setup()
{
    std::call_once(g_setup, [] {
        ggml_log_set(ggmlLog, nullptr);
        whisper_log_set(ggmlLog, nullptr);
        llama_log_set(ggmlLog, nullptr);
        qt_log_set(qwenLog, nullptr);
        const std::string dir = ownFolder();
        // the compute modules beside the libraries: the best CPU variant for this processor, Vulkan when it loads
        if (!dir.empty())
            ggml_backend_load_all_from_path(dir.c_str());
        else
            ggml_backend_load_all();
        llama_backend_init();
        if (g_threads.load() <= 0) {
            const int cores = int(std::thread::hardware_concurrency());
            g_threads = cores > 6 ? cores * 3 / 4 : std::max(1, cores - 1);
        }
    });
}

// ------------------------------------------------------------------ devices

struct DeviceList {
    std::mutex lock;
    std::vector<qbai_device> list;
    std::vector<ggml_backend_dev_t> devs;   // [0] = the CPU
};
DeviceList g_devices;

void copyText(char *to, size_t size, const char *from)
{
    std::snprintf(to, size, "%s", from ? from : "");
}

void readDevices()
{
    std::lock_guard<std::mutex> g(g_devices.lock);
    g_devices.list.clear();
    g_devices.devs.clear();
    ggml_backend_dev_t cpu = ggml_backend_dev_by_type(GGML_BACKEND_DEVICE_TYPE_CPU);
    qbai_device c{};
    c.index = 0;
    c.type = QBAI_DEVICE_CPU;
    copyText(c.id, sizeof c.id, cpu ? ggml_backend_dev_name(cpu) : "CPU");
    copyText(c.name, sizeof c.name, cpu ? ggml_backend_dev_description(cpu) : "CPU");
    if (cpu) {
        size_t freeB = 0, totalB = 0;
        ggml_backend_dev_memory(cpu, &freeB, &totalB);
        c.memory_free = int64_t(freeB);
        c.memory_total = int64_t(totalB);
    }
    g_devices.list.push_back(c);
    g_devices.devs.push_back(cpu);
    for (size_t i = 0; i < ggml_backend_dev_count(); ++i) {
        ggml_backend_dev_t d = ggml_backend_dev_get(i);
        const enum ggml_backend_dev_type t = ggml_backend_dev_type(d);
        if (t != GGML_BACKEND_DEVICE_TYPE_GPU && t != GGML_BACKEND_DEVICE_TYPE_IGPU)
            continue;
        qbai_device g{};
        g.index = int(g_devices.list.size());
        g.type = t == GGML_BACKEND_DEVICE_TYPE_GPU ? QBAI_DEVICE_GPU : QBAI_DEVICE_IGPU;
        copyText(g.id, sizeof g.id, ggml_backend_dev_name(d));
        copyText(g.name, sizeof g.name, ggml_backend_dev_description(d));
        size_t freeB = 0, totalB = 0;
        ggml_backend_dev_memory(d, &freeB, &totalB);   // Vulkan: the budget of VK_EXT_memory_budget when the driver has it
        g.memory_free = int64_t(freeB);
        g.memory_total = int64_t(totalB);
        g_devices.list.push_back(g);
        g_devices.devs.push_back(d);
    }
}

int deviceCount(int refresh)
{
    setup();
    {
        std::lock_guard<std::mutex> g(g_devices.lock);
        if (!refresh && !g_devices.list.empty())
            return int(g_devices.list.size());
    }
    readDevices();
    std::lock_guard<std::mutex> g(g_devices.lock);
    return int(g_devices.list.size());
}

int deviceGet(int index, qbai_device *out)
{
    deviceCount(0);
    std::lock_guard<std::mutex> g(g_devices.lock);
    if (!out || index < 0 || index >= int(g_devices.list.size()))
        return fail(QBAI_BAD_ARGS, "no such device");
    *out = g_devices.list[size_t(index)];
    return QBAI_OK;
}

ggml_backend_dev_t deviceHandle(int index)
{
    deviceCount(0);
    std::lock_guard<std::mutex> g(g_devices.lock);
    if (index < 0 || index >= int(g_devices.devs.size()))
        return nullptr;
    return g_devices.devs[size_t(index)];
}

// the position of a card among ggml's GPU and IGPU devices (whisper.cpp's gpu_device counts them so)
int gpuOrdinal(int index) { return index - 1; }

void setThreads(int n)
{
    setup();
    g_threads = std::max(1, n);
}

int threads()
{
    setup();
    return std::max(1, g_threads.load());
}

// ------------------------------------------------------------------ models

int64_t fileSize(const char *path)
{
    if (!path || !*path)
        return 0;
    struct stat st{};
    if (stat(path, &st) != 0)
        return -1;
    return int64_t(st.st_size);
}

constexpr int64_t kMiB = 1024 * 1024;

// An estimate from the files: what whisper.cpp's own table says for its models (tiny 75 MB -> about 270 MB, small
// 466 MB -> about 850 MB, large 2.9 GB -> about 3.9 GB), for the speech models a margin for the KV cache and the codec's
// decoding. The catalog's figure (memoryMB) wins where it gives one.
int64_t modelMemory(int kind, const char *modelPath, const char *extraPath, int /*device*/)
{
    const int64_t a = fileSize(modelPath), b = fileSize(extraPath);
    if (a <= 0 || b < 0)
        return 0;
    switch (kind) {
    case QBAI_ASR_WHISPER:
        return a + a / 4 + 200 * kMiB;
    case QBAI_TTS_POCKET:
        return (a + b) + (a + b) / 5 + 256 * kMiB;
    case QBAI_TTS_QWEN:
        return (a + b) + (a + b) / 4 + 1024 * kMiB;
    default:
        return 0;
    }
}

struct Pocket {
    llama_model *model = nullptr;
    llama_context *ctx = nullptr;
    mtmd_context *mtmd = nullptr;
    mtmd_helper_gen_audio *gen = nullptr;
    int rate = 24000;
    int channels = 1;
    bool cloning = false;
};

} // namespace

struct qbai_model {
    int kind = 0;
    int device = 0;
    int threads = 1;
    int64_t bytes = 0;
    whisper_context *whisper = nullptr;
    Pocket pocket;
    qt_context *qwen = nullptr;
    std::string qwenType;
    std::vector<std::string> voices, languages;
};

namespace {

void freeModel(qbai_model *m)
{
    if (!m)
        return;
    if (m->whisper)
        whisper_free(m->whisper);
    if (m->pocket.gen)
        mtmd_helper_gen_audio_free(m->pocket.gen);
    if (m->pocket.mtmd)
        mtmd_free(m->pocket.mtmd);
    if (m->pocket.ctx)
        llama_free(m->pocket.ctx);
    if (m->pocket.model)
        llama_model_free(m->pocket.model);
    if (m->qwen)
        qt_free(m->qwen);
    delete m;
}

// ISO 639-1 -> the names qwentts.cpp takes ("english"); "" when the model does not list it (then "auto")
std::string qwenLanguage(const qbai_model *m, const char *code)
{
    if (!code || !*code || std::strcmp(code, "auto") == 0)
        return std::string();
    static const char *const map[][2] = {{"en", "english"}, {"de", "german"}, {"fr", "french"}, {"es", "spanish"},
                                         {"it", "italian"}, {"pt", "portuguese"}, {"ru", "russian"}, {"ja", "japanese"},
                                         {"ko", "korean"}, {"zh", "chinese"}};
    std::string name = code;
    for (const auto &p : map)
        if (std::strcmp(code, p[0]) == 0)
            name = p[1];
    for (const std::string &l : m->languages)
        if (l == name)
            return name;
    return std::string();
}

qbai_model *loadWhisper(qbai_model *m, const qbai_load_params *p)
{
    whisper_context_params cp = whisper_context_default_params();
    cp.use_gpu = p->device > 0;
    cp.gpu_device = p->device > 0 ? gpuOrdinal(p->device) : 0;
    cp.flash_attn = p->device > 0;
    m->whisper = whisper_init_from_file_with_params(p->model_path, cp);
    if (!m->whisper) {
        fail(QBAI_ERROR, std::string("whisper.cpp could not load the model ") + p->model_path);
        freeModel(m);
        return nullptr;
    }
    for (int i = 0; i <= whisper_lang_max_id(); ++i)
        if (const char *code = whisper_lang_str(i))
            m->languages.emplace_back(code);
    return m;
}

qbai_model *loadPocket(qbai_model *m, const qbai_load_params *p)
{
    ggml_backend_dev_t dev = p->device > 0 ? deviceHandle(p->device) : nullptr;
    if (p->device > 0 && !dev) {
        fail(QBAI_BAD_ARGS, "no such device");
        freeModel(m);
        return nullptr;
    }
    ggml_backend_dev_t devs[2] = {dev, nullptr};   // the CPU: an empty list, nothing is offloaded
    llama_model_params mp = llama_model_default_params();
    mp.devices = devs;
    mp.n_gpu_layers = dev ? -1 : 0;
    m->pocket.model = llama_model_load_from_file(p->model_path, mp);
    if (!m->pocket.model) {
        fail(QBAI_ERROR, std::string("llama.cpp could not load the model ") + p->model_path);
        freeModel(m);
        return nullptr;
    }
    llama_context_params cp = llama_context_default_params();
    cp.embeddings = true;             // the hidden states go to the audio generation
    cp.n_ctx = 4096;                  // one sentence or a paragraph at a time: bounds the KV cache
    cp.n_threads = m->threads;
    cp.n_threads_batch = m->threads;
    m->pocket.ctx = llama_init_from_model(m->pocket.model, cp);
    if (!m->pocket.ctx) {
        fail(QBAI_ERROR, "llama.cpp could not make a context for the model");
        freeModel(m);
        return nullptr;
    }
    mtmd_context_params tp = mtmd_context_params_default();
    tp.use_gpu = dev != nullptr;
    tp.device = dev;
    tp.n_threads = m->threads;
    tp.print_timings = false;
    tp.warmup = false;
    m->pocket.mtmd = mtmd_init_from_file(p->extra_path, m->pocket.model, tp);
    if (!m->pocket.mtmd) {
        fail(QBAI_ERROR, std::string("libmtmd could not load ") + (p->extra_path ? p->extra_path : "(no mmproj)"));
        freeModel(m);
        return nullptr;
    }
    const mtmd_gen_audio_info info = mtmd_gen_audio_get_info(m->pocket.mtmd);
    if (info.type != MTMD_GEN_AUDIO_TYPE_POCKETTTS) {
        fail(QBAI_UNSUPPORTED, "the mmproj is not one of Pocket TTS");
        freeModel(m);
        return nullptr;
    }
    m->pocket.rate = info.sample_rate > 0 ? info.sample_rate : 24000;
    m->pocket.channels = std::max(1, int(info.n_channels));
    for (int32_t i = 0; i < info.n_voices; ++i)
        if (const char *v = mtmd_gen_audio_voice_name(m->pocket.mtmd, i))
            m->voices.emplace_back(v);
    const mtmd_helper_cap cap = mtmd_helper_get_capabilities(m->pocket.model, m->pocket.mtmd);
    m->pocket.cloning = cap.audio_gen_spk_ref && info.n_voices == 0;
    m->pocket.gen = mtmd_helper_gen_audio_init(m->pocket.ctx, m->pocket.mtmd);
    if (!m->pocket.gen) {
        fail(QBAI_ERROR, "the context cannot run Pocket TTS's audio generation");
        freeModel(m);
        return nullptr;
    }
    return m;
}

qbai_model *loadQwen(qbai_model *m, const qbai_load_params *p)
{
    std::string devName = "CPU";
    if (p->device > 0) {
        qbai_device d{};
        if (deviceGet(p->device, &d) != QBAI_OK) {
            freeModel(m);
            return nullptr;
        }
        devName = d.id;
    }
    qt_init_params ip;
    qt_init_default_params(&ip);
    ip.talker_path = p->model_path;
    ip.codec_path = p->extra_path;
    ip.device = devName.c_str();
    ip.use_fa = p->device > 0;
    ip.max_batch = 1;
    ip.codec_chunk_sec = 4.0f;   // bounds the decoder's peak memory (the library's default is a whole 24 s window)
    m->qwen = qt_init(&ip);
    if (!m->qwen) {
        fail(QBAI_ERROR, std::string("qwentts.cpp could not load the model: ") + qt_last_error());
        freeModel(m);
        return nullptr;
    }
    m->qwenType = qt_model_type(m->qwen) ? qt_model_type(m->qwen) : "";
    for (int i = 0; i < qt_n_speakers(m->qwen); ++i)
        if (const char *s = qt_speaker_name(m->qwen, i))
            m->voices.emplace_back(s);
    for (int i = 0; i < qt_n_languages(m->qwen); ++i)
        if (const char *l = qt_language_name(m->qwen, i))
            m->languages.emplace_back(l);
    return m;
}

qbai_model *load(const qbai_load_params *p)
{
    setup();
    t_error.clear();
    t_lastLog.clear();
    if (!p || p->size < int(sizeof(qbai_load_params)) || !p->model_path || !*p->model_path) {
        fail(QBAI_BAD_ARGS, "bad load parameters");
        return nullptr;
    }
    if (fileSize(p->model_path) <= 0 || (p->kind != QBAI_ASR_WHISPER && fileSize(p->extra_path) <= 0)) {
        fail(QBAI_BAD_ARGS, "a model file is missing or empty");
        return nullptr;
    }
    if (p->device < 0 || p->device >= deviceCount(0)) {
        fail(QBAI_BAD_ARGS, "no such device");
        return nullptr;
    }
    auto *m = new qbai_model;
    m->kind = p->kind;
    m->device = p->device;
    m->threads = p->threads > 0 ? p->threads : threads();
    m->bytes = modelMemory(p->kind, p->model_path, p->extra_path, p->device);
    switch (p->kind) {
    case QBAI_ASR_WHISPER:
        return loadWhisper(m, p);
    case QBAI_TTS_POCKET:
        return loadPocket(m, p);
    case QBAI_TTS_QWEN:
        return loadQwen(m, p);
    default:
        delete m;
        fail(QBAI_BAD_ARGS, "unknown kind of model");
        return nullptr;
    }
}

void unload(qbai_model *m) { freeModel(m); }

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
    out->voices = int(m->voices.size());
    out->languages = int(m->languages.size());
    switch (m->kind) {
    case QBAI_ASR_WHISPER:
        out->sample_rate = WHISPER_SAMPLE_RATE;
        out->multilingual = whisper_is_multilingual(m->whisper);
        copyText(out->type, sizeof out->type, "whisper");
        break;
    case QBAI_TTS_POCKET:
        out->sample_rate = m->pocket.rate;
        out->multilingual = 0;
        out->voice_clone = m->pocket.cloning ? 1 : 0;
        copyText(out->type, sizeof out->type, "pocket-tts");
        break;
    case QBAI_TTS_QWEN:
        out->sample_rate = 24000;
        out->multilingual = 1;
        out->voice_clone = m->qwenType == "base" ? 1 : 0;
        copyText(out->type, sizeof out->type, m->qwenType.c_str());
        break;
    }
    return QBAI_OK;
}

const char *voiceName(qbai_model *m, int i)
{
    return m && i >= 0 && i < int(m->voices.size()) ? m->voices[size_t(i)].c_str() : nullptr;
}

const char *languageName(qbai_model *m, int i)
{
    return m && i >= 0 && i < int(m->languages.size()) ? m->languages[size_t(i)].c_str() : nullptr;
}

// ------------------------------------------------------------------ ASR

struct WhisperCall {
    const qbai_transcribe_params *p = nullptr;
    int lastSegment = 0;
};

void whisperSegment(whisper_context *ctx, whisper_state *, int n_new, void *user)
{
    auto *c = static_cast<WhisperCall *>(user);
    if (!c->p->segment)
        return;
    const int n = whisper_full_n_segments(ctx);
    for (int i = std::max(0, n - n_new); i < n; ++i)
        c->p->segment(whisper_full_get_segment_text(ctx, i), whisper_full_get_segment_t0(ctx, i) * 10,
                      whisper_full_get_segment_t1(ctx, i) * 10, c->p->user);
}

void whisperProgress(whisper_context *, whisper_state *, int progress, void *user)
{
    auto *c = static_cast<WhisperCall *>(user);
    if (c->p->progress)
        c->p->progress(progress, c->p->user);
}

bool whisperAbort(void *user)
{
    auto *c = static_cast<WhisperCall *>(user);
    return c->p->cancel && c->p->cancel(c->p->user) != 0;
}

int transcribe(qbai_model *m, const float *pcm, int64_t samples, const qbai_transcribe_params *p, char **text)
{
    t_error.clear();
    t_lastLog.clear();
    if (text)
        *text = nullptr;
    if (!m || m->kind != QBAI_ASR_WHISPER || !pcm || samples <= 0 || !p || p->size < int(QBAI_TRANSCRIBE_V1_SIZE))
        return fail(QBAI_BAD_ARGS, "bad arguments");
    // the fields of version 2 count only when the caller's struct reaches them
    const bool v2 = p->size >= int(sizeof(qbai_transcribe_params));
    const int beam = v2 ? p->beam_size : 0;
    WhisperCall call;
    call.p = p;
    whisper_full_params wp = whisper_full_default_params(beam > 1 ? WHISPER_SAMPLING_BEAM_SEARCH : WHISPER_SAMPLING_GREEDY);
    if (beam > 1)
        wp.beam_search.beam_size = beam;
    if (v2) {
        wp.suppress_nst = p->suppress_nst != 0;
        wp.no_timestamps = p->no_timestamps != 0;
        wp.no_context = p->no_context != 0;
    }
    wp.n_threads = m->threads;
    wp.language = p->language && *p->language ? p->language : "auto";
    wp.translate = p->translate != 0;
    wp.initial_prompt = p->prompt;
    wp.print_progress = false;
    wp.print_realtime = false;
    wp.print_special = false;
    wp.print_timestamps = false;
    wp.new_segment_callback = whisperSegment;
    wp.new_segment_callback_user_data = &call;
    wp.progress_callback = whisperProgress;
    wp.progress_callback_user_data = &call;
    wp.abort_callback = whisperAbort;
    wp.abort_callback_user_data = &call;
    const int rc = whisper_full(m->whisper, wp, pcm, int(std::min<int64_t>(samples, INT32_MAX)));
    if (p->cancel && p->cancel(p->user) != 0)
        return fail(QBAI_CANCELLED, "cancelled");
    if (rc != 0)
        return fail(QBAI_ERROR, "whisper.cpp could not transcribe the audio (code " + std::to_string(rc) + ")");
    std::string all;
    const float noSpeechMax = v2 ? p->no_speech_max : 0.f;
    for (int i = 0; i < whisper_full_n_segments(m->whisper); ++i) {
        if (noSpeechMax > 0.f && whisper_full_get_segment_no_speech_prob(m->whisper, i) > noSpeechMax)
            continue;   // most likely no speech: whisper invents text for such stretches
        all += whisper_full_get_segment_text(m->whisper, i);
    }
    if (text) {
        *text = static_cast<char *>(std::malloc(all.size() + 1));
        if (!*text)
            return fail(QBAI_NO_MEMORY, "out of memory");
        std::memcpy(*text, all.c_str(), all.size() + 1);
    }
    return QBAI_OK;
}

void freeText(char *text) { std::free(text); }

int detectLanguage(qbai_model *m, const float *pcm, int64_t samples, const char *const *codes, int count, float *probs)
{
    t_error.clear();
    if (!m || m->kind != QBAI_ASR_WHISPER || !pcm || samples <= 0 || !codes || count <= 0 || !probs)
        return fail(QBAI_BAD_ARGS, "bad arguments");
    if (!whisper_is_multilingual(m->whisper))
        return fail(QBAI_UNSUPPORTED, "the model knows English only");
    if (whisper_pcm_to_mel(m->whisper, pcm, int(std::min<int64_t>(samples, INT32_MAX)), m->threads) != 0)
        return fail(QBAI_ERROR, "whisper.cpp could not read the audio");
    std::vector<float> all(size_t(whisper_lang_max_id()) + 1, 0.f);
    if (whisper_lang_auto_detect(m->whisper, 0, m->threads, all.data()) < 0)
        return fail(QBAI_ERROR, "whisper.cpp could not detect the language");
    for (int i = 0; i < count; ++i) {
        const int id = codes[i] ? whisper_lang_id(codes[i]) : -1;
        probs[i] = id >= 0 && size_t(id) < all.size() ? all[size_t(id)] : 0.f;
    }
    return QBAI_OK;
}

// ------------------------------------------------------------------ PCM helpers

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

std::vector<float> resample(const float *in, int64_t n, int from, int to)
{
    std::vector<float> out;
    if (from == to || from <= 0 || to <= 0) {
        out.assign(in, in + n);
        return out;
    }
    // linear: good enough for a voice reference (the encoders look at the voice, not at the highest frequencies)
    const int64_t m = int64_t(double(n) * to / from);
    out.resize(size_t(std::max<int64_t>(m, 0)));
    for (int64_t i = 0; i < m; ++i) {
        const double x = double(i) * from / to;
        const int64_t a = int64_t(x);
        const double f = x - double(a);
        const float s0 = in[std::min(a, n - 1)], s1 = in[std::min(a + 1, n - 1)];
        out[size_t(i)] = float(s0 + (s1 - s0) * f);
    }
    return out;
}

// ------------------------------------------------------------------ TTS

struct QwenCall {
    const qbai_synth_params *p = nullptr;
    std::atomic<bool> stopped{false};
};

bool qwenCancel(void *user)
{
    auto *c = static_cast<QwenCall *>(user);
    if (c->p->cancel && c->p->cancel(c->p->user) != 0)
        c->stopped = true;
    return c->stopped.load();
}

bool qwenChunk(const float *samples, int n, void *user)
{
    auto *c = static_cast<QwenCall *>(user);
    if (c->p->chunk(samples, n, c->p->user) == 0) {
        c->stopped = true;
        return false;
    }
    return true;
}

int synthQwen(qbai_model *m, const qbai_synth_params *p, qbai_pcm *out)
{
    const qbai_voice *v = p->voice && p->voice->size >= int(sizeof(qbai_voice)) ? p->voice : nullptr;
    qt_tts_params tp;
    qt_tts_default_params(&tp);
    tp.text = p->text;
    const std::string lang = qwenLanguage(m, p->language);
    tp.lang = lang.empty() ? nullptr : lang.c_str();
    tp.seed = p->seed;
    std::vector<float> clip;
    if (v && v->clip && v->clip_samples > 0) {
        if (m->qwenType != "base")
            return fail(QBAI_UNSUPPORTED, "only a Qwen3-TTS Base model takes a reference clip");
        clip = resample(v->clip, v->clip_samples, v->clip_rate, 24000);
        tp.ref_audio_24k = clip.data();
        tp.ref_n_samples = int(clip.size());
        tp.ref_text = v->clip_text && *v->clip_text ? v->clip_text : nullptr;
    }
    if (v && v->preset && *v->preset) {
        if (m->qwenType != "custom_voice")
            return fail(QBAI_UNSUPPORTED, "only a Qwen3-TTS CustomVoice model has named speakers");
        tp.speaker = v->preset;
    } else if (m->qwenType == "custom_voice" && !m->voices.empty()) {
        tp.speaker = m->voices.front().c_str();
    }
    if (v && v->instruct && *v->instruct && m->qwenType != "base")
        tp.instruct = v->instruct;
    QwenCall call;
    call.p = p;
    tp.cancel = qwenCancel;
    tp.cancel_user_data = &call;
    if (p->chunk) {
        tp.on_chunk = qwenChunk;
        tp.on_chunk_user_data = &call;
    }
    qt_audio a = {};
    const qt_status rc = qt_synthesize(m->qwen, &tp, &a);
    if (rc == QT_STATUS_CANCELLED || call.stopped) {
        qt_audio_free(&a);
        return fail(QBAI_CANCELLED, "cancelled");
    }
    if (rc != QT_STATUS_OK) {
        const std::string why = qt_last_error() ? qt_last_error() : "";
        qt_audio_free(&a);
        return fail(rc == QT_STATUS_OOM ? QBAI_NO_MEMORY : rc == QT_STATUS_MODE_INVALID ? QBAI_UNSUPPORTED : QBAI_ERROR,
                    "qwentts.cpp could not speak the text: " + why);
    }
    if (!p->chunk && !putPcm(out, a.samples, a.n_samples, a.sample_rate > 0 ? a.sample_rate : 24000)) {
        qt_audio_free(&a);
        return fail(QBAI_NO_MEMORY, "out of memory");
    }
    qt_audio_free(&a);
    return QBAI_OK;
}

// interleaved channels -> mono
std::vector<float> toMono(const float *data, int64_t frames, int channels)
{
    std::vector<float> mono(size_t(std::max<int64_t>(frames, 0)));
    for (int64_t i = 0; i < frames; ++i) {
        float s = 0;
        for (int c = 0; c < channels; ++c)
            s += data[i * channels + c];
        mono[size_t(i)] = s / float(channels);
    }
    return mono;
}

int synthPocket(qbai_model *m, const qbai_synth_params *p, qbai_pcm *out)
{
    const qbai_voice *v = p->voice && p->voice->size >= int(sizeof(qbai_voice)) ? p->voice : nullptr;
    Pocket &k = m->pocket;
    mtmd_bitmap *voice = nullptr;
    if (v && v->clip && v->clip_samples > 0) {
        if (!k.cloning)
            return fail(QBAI_UNSUPPORTED, "this Pocket TTS model has no voice encoder; choose one of its voices");
        const std::vector<float> clip = resample(v->clip, v->clip_samples, v->clip_rate, k.rate);
        voice = mtmd_bitmap_init_from_audio(clip.size(), clip.data());
    } else if (!m->voices.empty()) {
        const char *name = v && v->preset && *v->preset ? v->preset : m->voices.front().c_str();
        voice = mtmd_gen_audio_get_voice(k.mtmd, name);
        if (!voice)
            return fail(QBAI_BAD_ARGS, std::string("the model has no voice \"") + name + "\"");
    }
    struct BitmapFree {
        mtmd_bitmap *b;
        ~BitmapFree() { if (b) mtmd_bitmap_free(b); }
    } voiceGuard{voice};

    const std::string text = p->text;
    mtmd_helper_gen_audio_reset(k.gen);
    mtmd_helper_gen_audio_inp inp = mtmd_helper_gen_audio_inp_default();
    inp.stream = p->chunk != nullptr;
    inp.seq_id = 0;
    inp.prompt = text.c_str();
    inp.prompt_len = text.size();
    inp.speaker_ref = voice;
    inp.out_type = MTMD_HELPER_GEN_AUDIO_OUTTYPE_PCM;
    if (p->seed >= 0)
        inp.seed = uint32_t(p->seed);
    const int32_t set = mtmd_helper_gen_audio_set_input(k.gen, &inp);
    if (set != 0)
        return fail(set == 2 ? QBAI_BAD_ARGS : QBAI_ERROR, "Pocket TTS did not take the text");
    llama_set_embeddings(k.ctx, true);
    for (;;) {
        const int32_t left = mtmd_helper_gen_audio_step_prompt(k.gen, int32_t(llama_n_batch(k.ctx)));
        if (left < 0)
            return fail(QBAI_ERROR, "Pocket TTS could not read the text");
        if (left == 0)
            break;
        if (p->cancel && p->cancel(p->user) != 0)
            return fail(QBAI_CANCELLED, "cancelled");
    }
    // the backbone's token is not used by Pocket TTS (its frames come from the hidden state), but the loop takes one
    llama_sampler_chain_params sp = llama_sampler_chain_default_params();
    llama_sampler *smpl = llama_sampler_chain_init(sp);
    llama_sampler_chain_add(smpl, llama_sampler_init_top_k(40));
    llama_sampler_chain_add(smpl, llama_sampler_init_top_p(0.95f, 1));
    llama_sampler_chain_add(smpl, llama_sampler_init_temp(0.8f));
    llama_sampler_chain_add(smpl, llama_sampler_init_dist(p->seed >= 0 ? uint32_t(p->seed) : LLAMA_DEFAULT_SEED));
    struct SamplerFree {
        llama_sampler *s;
        ~SamplerFree() { llama_sampler_free(s); }
    } samplerGuard{smpl};

    std::vector<float> all;
    auto take = [&](bool last) -> int {
        int32_t rate = 0;
        const char *data = nullptr;
        size_t len = 0;
        int64_t frames = 0;
        if (last && mtmd_helper_gen_audio_flush(k.gen) != 0)
            return fail(QBAI_ERROR, "Pocket TTS could not finish the audio");
        if (mtmd_helper_gen_audio_get_output(k.gen, &rate, &data, &len, &frames) != 0)
            return fail(QBAI_ERROR, "Pocket TTS gave no audio");
        if (!data || len == 0)
            return QBAI_OK;
        const int64_t count = int64_t(len / sizeof(float));
        const int64_t perChannel = k.channels > 1 ? count / k.channels : count;
        std::vector<float> mono = k.channels > 1 ? toMono(reinterpret_cast<const float *>(data), perChannel, k.channels)
                                                 : std::vector<float>(reinterpret_cast<const float *>(data),
                                                                      reinterpret_cast<const float *>(data) + count);
        if (p->chunk) {
            if (!mono.empty() && p->chunk(mono.data(), int64_t(mono.size()), p->user) == 0)
                return fail(QBAI_CANCELLED, "cancelled");
        } else {
            all = std::move(mono);   // without streaming the helper hands out the whole sentence at the end
        }
        return QBAI_OK;
    };

    const int32_t limit = mtmd_helper_gen_audio_max_frames(k.gen) > 0 ? mtmd_helper_gen_audio_max_frames(k.gen) : 3000;
    llama_token sampled = llama_sampler_sample(smpl, k.ctx, -1);
    const float *h = llama_get_embeddings_ith(k.ctx, -1);
    std::vector<float> hState;
    if (h)
        hState.assign(h, h + llama_model_n_embd(k.model));
    const float *state = hState.empty() ? nullptr : hState.data();
    bool stop = false;
    for (int32_t n = 0; !stop && n < limit; ++n) {
        if (p->cancel && p->cancel(p->user) != 0)
            return fail(QBAI_CANCELLED, "cancelled");
        const float *next = nullptr;
        if (mtmd_helper_gen_audio_step_gen(k.gen, sampled, state, &next, &stop) != 0)
            return fail(QBAI_ERROR, "Pocket TTS failed while it spoke");
        if (!next)
            break;   // the end of the speech without a frame
        state = next;
        if (!stop)
            sampled = llama_sampler_sample(smpl, k.ctx, -1);
        if (p->chunk) {
            const int rc = take(false);
            if (rc != QBAI_OK)
                return rc;
        }
    }
    const int rc = take(true);
    if (rc != QBAI_OK)
        return rc;
    if (!p->chunk && !putPcm(out, all.data(), int64_t(all.size()), k.rate))
        return fail(QBAI_NO_MEMORY, "out of memory");
    return QBAI_OK;
}

int synthesize(qbai_model *m, const qbai_synth_params *p, qbai_pcm *out)
{
    t_error.clear();
    t_lastLog.clear();
    if (out) {
        out->samples = nullptr;
        out->count = 0;
        out->rate = 0;
    }
    if (!m || !p || p->size < int(sizeof(qbai_synth_params)) || !p->text || !*p->text || (!p->chunk && !out))
        return fail(QBAI_BAD_ARGS, "bad arguments");
    if (m->kind == QBAI_TTS_QWEN)
        return synthQwen(m, p, out);
    if (m->kind == QBAI_TTS_POCKET)
        return synthPocket(m, p, out);
    return fail(QBAI_BAD_ARGS, "the model does not speak");
}

// ------------------------------------------------------------------ Sonic

} // namespace

struct qbai_stretcher {
    sonicStream stream = nullptr;
    int rate = 0;
};

namespace {

bool readAll(sonicStream s, int rate, qbai_pcm *out)
{
    const int n = sonicSamplesAvailable(s);
    std::vector<float> buf(size_t(std::max(n, 0)));
    int got = n > 0 ? sonicReadFloatFromStream(s, buf.data(), n) : 0;
    return putPcm(out, buf.data(), std::max(got, 0), rate);
}

qbai_stretcher *stretchBegin(int rate, float speed)
{
    if (rate <= 0 || !(speed >= 0.5f && speed <= 4.0f)) {
        fail(QBAI_BAD_ARGS, "speed must be between 0.5 and 4.0");
        return nullptr;
    }
    auto *s = new qbai_stretcher;
    s->rate = rate;
    s->stream = sonicCreateStream(rate, 1);
    if (!s->stream) {
        delete s;
        fail(QBAI_NO_MEMORY, "out of memory");
        return nullptr;
    }
    sonicSetSpeed(s->stream, speed);
    sonicSetPitch(s->stream, 1.0f);
    sonicSetRate(s->stream, 1.0f);
    sonicSetQuality(s->stream, 1);
    return s;
}

int stretchFeed(qbai_stretcher *s, const float *pcm, int64_t samples, qbai_pcm *out)
{
    if (!s || !out || (samples > 0 && !pcm))
        return fail(QBAI_BAD_ARGS, "bad arguments");
    for (int64_t done = 0; done < samples;) {
        const int n = int(std::min<int64_t>(samples - done, 1 << 20));
        if (!sonicWriteFloatToStream(s->stream, pcm + done, n))
            return fail(QBAI_NO_MEMORY, "out of memory");
        done += n;
    }
    return readAll(s->stream, s->rate, out) ? QBAI_OK : fail(QBAI_NO_MEMORY, "out of memory");
}

int stretchEnd(qbai_stretcher *s, qbai_pcm *out)
{
    if (!s)
        return fail(QBAI_BAD_ARGS, "bad arguments");
    int rc = QBAI_OK;
    sonicFlushStream(s->stream);
    if (out && !readAll(s->stream, s->rate, out))
        rc = fail(QBAI_NO_MEMORY, "out of memory");
    sonicDestroyStream(s->stream);
    delete s;
    return rc;
}

int stretch(const float *pcm, int64_t samples, int rate, float speed, qbai_pcm *out)
{
    if (!out || (samples > 0 && !pcm))
        return fail(QBAI_BAD_ARGS, "bad arguments");
    qbai_stretcher *s = stretchBegin(rate, speed);
    if (!s)
        return QBAI_BAD_ARGS;
    qbai_pcm first = {}, rest = {};
    int rc = stretchFeed(s, pcm, samples, &first);
    const int rc2 = stretchEnd(s, &rest);
    if (rc == QBAI_OK)
        rc = rc2;
    if (rc != QBAI_OK) {
        freePcm(&first);
        freePcm(&rest);
        return rc;
    }
    std::vector<float> all(first.samples, first.samples + first.count);
    all.insert(all.end(), rest.samples, rest.samples + rest.count);
    freePcm(&first);
    freePcm(&rest);
    return putPcm(out, all.data(), int64_t(all.size()), rate) ? QBAI_OK : fail(QBAI_NO_MEMORY, "out of memory");
}

// ------------------------------------------------------------------ the table

const char *engineVersion()
{
    static const std::string v = std::string("qbai 2; ") + QBAI_PARTS;
    return v.c_str();
}

const char *lastError() { return t_error.c_str(); }

qbai_api makeApi(int version, int size)
{
    qbai_api a;
    std::memset(&a, 0, sizeof a);
    a.version = version;
    a.size = size;
    a.engine_version = engineVersion;
    a.last_error = lastError;
    a.device_count = deviceCount;
    a.device_get = deviceGet;
    a.set_threads = setThreads;
    a.threads = threads;
    a.model_memory = modelMemory;
    a.load = load;
    a.unload = unload;
    a.model_info = modelInfo;
    a.voice_name = voiceName;
    a.language_name = languageName;
    a.transcribe = transcribe;
    a.free_text = freeText;
    a.synthesize = synthesize;
    a.free_pcm = freePcm;
    a.stretch = stretch;
    a.stretch_begin = stretchBegin;
    a.stretch_feed = stretchFeed;
    a.stretch_end = stretchEnd;
    a.detect_language = detectLanguage;
    return a;
}

// version 1 callers get the table as they know it (the size says where it ends); version 2 the whole
const qbai_api kApi1 = makeApi(1, int(QBAI_API_V1_SIZE));
const qbai_api kApi2 = makeApi(2, int(sizeof(qbai_api)));

} // namespace

extern "C" const qbai_api *qbai_get_api(int version)
{
    if (version < 1 || version > QBAI_API_VERSION)
        return nullptr;
    return version == 1 ? &kApi1 : &kApi2;
}
