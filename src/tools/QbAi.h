#pragma once
/* The C interface of the AI engine package (0.99.0), version 1.
 *
 * The package "ai-engine" (addons/ai-engine) holds one ggml with its compute modules (the CPU variants and, on Linux and
 * Windows, Vulkan), whisper.cpp, llama.cpp with libmtmd (Pocket TTS), qwentts.cpp (Qwen3-TTS) and Sonic, and our wrapper
 * libqbai (engines/qbai), the only library anybody loads. The program has no link-time dependency on any of it: it opens
 * libqbai with QLibrary (dlopen) when an AI function is used, asks for qbai_get_api(QBAI_API_VERSION) and calls through
 * the struct of function pointers it returns. An add-on (Speech Transcription 1.10) gets the same struct from the
 * program (tool interface 1.3, ToolHost::aiEngine), so one process holds one engine.
 *
 * Versioning: QBAI_API_VERSION counts the layouts of struct qbai_api. A newer libqbai keeps every function of an older
 * version at its place and adds new ones at the end; qbai_get_api(v) returns the struct for any 1 <= v <= its own
 * version (NULL above it), and `size` says how much of it there is. The parameter structs begin with their own `size`
 * (set it to sizeof): a field added later lies beyond the size an older caller passes and is then not read.
 *
 * Threads: one model per handle; all calls on one handle from one thread at a time. device_count/device_get,
 * set_threads, model_memory, stretch and last_error may be called from any thread. Call load, transcribe and synthesize
 * from a worker thread that has lowered its own priority first (ToolHost::lowerThreadPriority,
 * ModelMemory::lowerThreadPriority): the engine's threads are started from it and inherit that priority.
 *
 * Callbacks of transcribe run on the calling thread. The `chunk` and `cancel` callbacks of synthesize may run on a
 * thread of the engine (qwentts.cpp computes on a worker of its own): keep them short and thread-safe, and do not call
 * back into this interface from them.
 *
 * Audio is handed over as memory only (mono float PCM in -1..1); nothing here writes a file.
 * Texts are UTF-8. Error texts are English and meant for a log or a detail line, not as the writer's message.
 */
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Version 2 (0.99.0, Speech Transcription 1.10) adds, at the end, beam search, the no-speech limit and more switches in
   qbai_transcribe_params and the function detect_language in qbai_api. A caller that wants them asks qbai_get_api(2) and falls
   back to qbai_get_api(1) (an older engine package); QBAI_API_VERSION is the newest this header knows, QBAI_API_V1_SIZE the
   size of the version 1 table. */
#define QBAI_API_VERSION 2
/* the library's file name inside the package's engine/lib folder (Linux; Windows: qbai.dll) */
#define QBAI_LIBRARY_LINUX "libqbai.so.1"
#define QBAI_LIBRARY_WINDOWS "qbai.dll"

enum qbai_status {
    QBAI_OK = 0,
    QBAI_ERROR = -1,        /* see last_error() */
    QBAI_CANCELLED = -2,    /* the cancel callback asked for it */
    QBAI_NO_MEMORY = -3,
    QBAI_BAD_ARGS = -4,
    QBAI_UNSUPPORTED = -5   /* the model cannot do this (a voice clip for a model without voice cloning, ...) */
};

enum qbai_device_type {
    QBAI_DEVICE_CPU = 0,
    QBAI_DEVICE_GPU = 1,    /* a graphics card with memory of its own */
    QBAI_DEVICE_IGPU = 2    /* a graphics part that shares the computer's memory */
};

/* Index 0 is always the CPU (with the computer's memory); 1 .. device_count()-1 are the graphics cards in ggml's order. */
struct qbai_device {
    int index;
    int type;                  /* enum qbai_device_type */
    char id[64];               /* ggml's name of the device: "CPU", "Vulkan0" */
    char name[256];            /* what the writer sees: "AMD Radeon RX 6600 (RADV NAVI23)" */
    int64_t memory_free;       /* bytes: the budget the driver gives this program now (the CPU: the free RAM) */
    int64_t memory_total;
};

enum qbai_model_kind {
    QBAI_ASR_WHISPER = 1,      /* model_path: whisper.cpp's ggml .bin */
    QBAI_TTS_POCKET = 2,       /* model_path: the Pocket TTS .gguf (llama.cpp), extra_path: its mmproj .gguf (libmtmd) */
    QBAI_TTS_QWEN = 3          /* model_path: the Qwen3-TTS talker .gguf, extra_path: the 12 Hz tokenizer .gguf (qwentts.cpp) */
};

typedef struct qbai_model qbai_model;   /* one loaded model */

struct qbai_load_params {
    int size;                  /* sizeof(struct qbai_load_params) */
    int kind;                  /* enum qbai_model_kind */
    const char *model_path;
    const char *extra_path;    /* NULL for whisper */
    int device;                /* qbai_device.index; 0 = the CPU */
    int threads;               /* 0 = the value of set_threads */
};

struct qbai_model_info {
    int size;                  /* sizeof(struct qbai_model_info) */
    int kind;
    int device;
    int sample_rate;           /* TTS: the rate of the PCM it makes (24000); ASR: the rate it takes (16000) */
    int64_t bytes;             /* the memory it takes (the estimate of model_memory) */
    int voices;                /* voice_name(0 .. voices-1): Pocket TTS's presets, a Qwen CustomVoice model's speakers */
    int languages;             /* language_name(0 .. languages-1) */
    int multilingual;          /* whisper: 0 for an English-only model */
    int voice_clone;           /* 1: a reference clip is taken (Qwen Base; Pocket TTS with its voice encoder) */
    char type[32];             /* Qwen: "base", "custom_voice", "voice_design"; Pocket TTS: "pocket-tts"; whisper: "whisper" */
};

/* ---- speech recognition */
struct qbai_transcribe_params {
    int size;                  /* sizeof(struct qbai_transcribe_params) */
    const char *language;      /* "en", "de", ... or "auto" (NULL: "auto") */
    int translate;             /* 1: write the text in English */
    const char *prompt;        /* words that help (names), may be NULL */
    /* called on the calling thread; each may be NULL */
    void (*segment)(const char *text, int64_t from_ms, int64_t to_ms, void *user);
    void (*progress)(int percent, void *user);
    int (*cancel)(void *user); /* not 0: stop (transcribe returns QBAI_CANCELLED) */
    void *user;
    /* ---- version 2 (read only when `size` reaches them) */
    int beam_size;             /* > 1: beam search with this many beams ("careful"); 0 or 1: greedy decoding */
    float no_speech_max;       /* > 0: a segment whose no-speech probability is above this is left out of the text (0.6 usual) */
    int suppress_nst;          /* 1: whisper's suppression of non-speech tokens */
    int no_timestamps;         /* 1: no timestamps are decoded (faster; the segment callback then gets 0 for both times) */
    int no_context;            /* 1: the text before is not used as context (the prompt alone is) */
};

/* offset of the first version 2 field: callers that pass a struct of version 1 have a smaller `size` */
#define QBAI_TRANSCRIBE_V1_SIZE offsetof(struct qbai_transcribe_params, beam_size)

/* ---- speech synthesis */
struct qbai_voice {
    int size;                  /* sizeof(struct qbai_voice) */
    const char *preset;        /* Pocket TTS: one of voice_name() (NULL: the first); Qwen CustomVoice: a speaker */
    const float *clip;         /* a reference clip (Qwen Base; Pocket TTS with voice cloning): mono float PCM */
    int64_t clip_samples;
    int clip_rate;             /* its sample rate; resampled to the model's rate when it differs */
    const char *clip_text;     /* what is said in the clip (Qwen Base: closer to the voice), may be NULL */
    const char *instruct;      /* Qwen CustomVoice: a style ("calm, slow"), may be NULL */
};

struct qbai_pcm {
    float *samples;            /* owned by the engine: free_pcm() */
    int64_t count;
    int rate;
};

struct qbai_synth_params {
    int size;                  /* sizeof(struct qbai_synth_params) */
    const char *text;          /* one sentence or a short paragraph */
    const char *language;      /* "en", "de", ... (Qwen: its language; NULL or "auto": the model decides; Pocket TTS: not used) */
    const struct qbai_voice *voice;   /* NULL: the model's default voice */
    int64_t seed;              /* -1: random */
    /* streaming: each new piece of PCM (mono, the model's rate) as soon as it is made; return 0 to stop. NULL: the whole
       sentence in `out` at the end. May run on a thread of the engine (see the top of this file). */
    int (*chunk)(const float *pcm, int64_t samples, void *user);
    int (*cancel)(void *user); /* not 0: stop (synthesize returns QBAI_CANCELLED); may run on a thread of the engine */
    void *user;
};

struct qbai_stretcher;         /* Sonic's stream, for PCM that comes in pieces */

struct qbai_api {
    int version;               /* the version this struct was made for (what was asked) */
    int size;                  /* sizeof(struct qbai_api) of the library */

    const char *(*engine_version)(void);   /* "qbai 1; ggml 1.1.1; whisper.cpp 1.9.6; llama.cpp b12808; ..." */
    const char *(*last_error)(void);       /* the last error of the calling thread ("" when none) */

    /* the devices; refresh != 0 reads the memory figures anew */
    int (*device_count)(int refresh);
    int (*device_get)(int index, struct qbai_device *out);   /* QBAI_OK or QBAI_BAD_ARGS */
    /* the CPU threads of the computations (the shared AI setting); at least 1 */
    void (*set_threads)(int threads);
    int (*threads)(void);

    /* the memory a model will take on a device, before it is loaded (an estimate from its files; 0 when unknown) */
    int64_t (*model_memory)(int kind, const char *model_path, const char *extra_path, int device);
    qbai_model *(*load)(const struct qbai_load_params *params);   /* NULL on failure (last_error) */
    void (*unload)(qbai_model *model);                            /* frees everything; NULL is allowed */
    int (*model_info)(qbai_model *model, struct qbai_model_info *out);
    const char *(*voice_name)(qbai_model *model, int index);      /* NULL past the end */
    const char *(*language_name)(qbai_model *model, int index);   /* NULL past the end */

    /* ASR: pcm is mono float at 16 kHz; *text gets the whole text (free_text) */
    int (*transcribe)(qbai_model *model, const float *pcm, int64_t samples, const struct qbai_transcribe_params *params,
                      char **text);
    void (*free_text)(char *text);

    /* TTS: one sentence; `out` gets the PCM when params->chunk is NULL (free_pcm), else stays empty */
    int (*synthesize)(qbai_model *model, const struct qbai_synth_params *params, struct qbai_pcm *out);
    void (*free_pcm)(struct qbai_pcm *pcm);

    /* Sonic: the speed changed, the pitch kept (speed 2.0 = twice as fast; 0.5 .. 4.0) */
    int (*stretch)(const float *pcm, int64_t samples, int rate, float speed, struct qbai_pcm *out);
    struct qbai_stretcher *(*stretch_begin)(int rate, float speed);
    int (*stretch_feed)(struct qbai_stretcher *s, const float *pcm, int64_t samples, struct qbai_pcm *out);
    int (*stretch_end)(struct qbai_stretcher *s, struct qbai_pcm *out);   /* the rest; frees s */

    /* ---- version 2 */
    /* ASR: how likely each of the `count` languages (ISO 639-1 codes) is for this audio (mono float at 16 kHz): probs[i] gets the
       probability of codes[i] (0 for a code the model does not know). Runs the model's encoder once: use it on a piece, then
       transcribe with the language chosen. A model that is not multilingual gets QBAI_UNSUPPORTED. */
    int (*detect_language)(qbai_model *model, const float *pcm, int64_t samples, const char *const *codes, int count, float *probs);
};

/* the size of the version 1 table (what an engine package of version 1 reports in `size`) */
#define QBAI_API_V1_SIZE offsetof(struct qbai_api, detect_language)

/* The one entry of libqbai: NULL when version is above the library's own (or below 1). The program and add-ons never
   link it: they look it up by name (QLibrary::resolve("qbai_get_api"), dlsym, GetProcAddress) as qbai_get_api_fn. */
typedef const struct qbai_api *(*qbai_get_api_fn)(int version);
#if defined(QBAI_BUILD)
#if defined(_WIN32)
__declspec(dllexport)
#else
__attribute__((visibility("default")))
#endif
const struct qbai_api *qbai_get_api(int version);
#endif

#ifdef __cplusplus
}
#endif
