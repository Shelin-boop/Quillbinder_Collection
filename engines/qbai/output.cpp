#include "output.h"

// miniaudio: the device part only (no decoding, encoding, generators, resource manager, node graph or engine). Backends are
// loaded at run time (no link-time dependency).
#define MA_NO_DECODING
#define MA_NO_ENCODING
#define MA_NO_GENERATION
#define MA_NO_RESOURCE_MANAGER
#define MA_NO_NODE_GRAPH
#define MA_NO_ENGINE
#define MINIAUDIO_IMPLEMENTATION
#include "third_party/miniaudio/miniaudio.h"

#include <algorithm>
#include <atomic>
#include <cstring>
#include <deque>
#include <mutex>
#include <vector>

namespace qbaiout {

struct Output {
    ma_device device;
    bool deviceOk = false;
    int channels = 1;
    std::mutex mutex;
    std::deque<float> queue;          // interleaved samples not yet played
    std::atomic<bool> paused{false};
    std::atomic<int64_t> played{0};   // frames
};

static void onData(ma_device *d, void *out, const void *, ma_uint32 frames)
{
    auto *o = static_cast<Output *>(d->pUserData);
    float *dst = static_cast<float *>(out);
    const size_t want = size_t(frames) * size_t(o->channels);
    size_t got = 0;
    if (!o->paused.load()) {
        std::lock_guard<std::mutex> g(o->mutex);
        got = std::min(want, o->queue.size());
        for (size_t i = 0; i < got; ++i) {
            dst[i] = o->queue.front();
            o->queue.pop_front();
        }
    }
    if (got < want)
        std::memset(dst + got, 0, (want - got) * sizeof(float));   // silence while paused or waiting for more
    o->played += int64_t(got / size_t(o->channels));
}

Output *open(int rate, int channels, std::string *error)
{
    if (rate <= 0 || channels < 1 || channels > 8) {
        if (error)
            *error = "bad sound output parameters";
        return nullptr;
    }
    auto *o = new Output;
    o->channels = channels;
    ma_device_config cfg = ma_device_config_init(ma_device_type_playback);
    cfg.playback.format = ma_format_f32;
    cfg.playback.channels = ma_uint32(channels);
    cfg.sampleRate = ma_uint32(rate);
    cfg.dataCallback = onData;
    cfg.pUserData = o;
    const ma_result r = ma_device_init(nullptr, &cfg, &o->device);
    if (r != MA_SUCCESS) {
        if (error)
            *error = std::string("no sound output could be opened: ") + ma_result_description(r);
        delete o;
        return nullptr;
    }
    o->deviceOk = true;
    const ma_result s = ma_device_start(&o->device);
    if (s != MA_SUCCESS) {
        if (error)
            *error = std::string("the sound output could not be started: ") + ma_result_description(s);
        close(o);
        return nullptr;
    }
    return o;
}

bool write(Output *o, const float *pcm, int64_t frames)
{
    if (!o || frames < 0 || (frames > 0 && !pcm))
        return false;
    std::lock_guard<std::mutex> g(o->mutex);
    o->queue.insert(o->queue.end(), pcm, pcm + size_t(frames) * size_t(o->channels));
    return true;
}

void pause(Output *o) { if (o) o->paused = true; }
void resume(Output *o) { if (o) o->paused = false; }

void stop(Output *o)
{
    if (!o)
        return;
    std::lock_guard<std::mutex> g(o->mutex);
    o->queue.clear();
    o->played = 0;
}

void close(Output *o)
{
    if (!o)
        return;
    if (o->deviceOk)
        ma_device_uninit(&o->device);
    delete o;
}

int64_t played(Output *o) { return o ? o->played.load() : 0; }

} // namespace qbaiout
