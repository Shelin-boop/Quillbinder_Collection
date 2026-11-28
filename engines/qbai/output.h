#pragma once
// The sound output of libqbai (Read Aloud): float PCM that is queued and played by miniaudio's default output device. Only played,
// never saved. Implemented in output.cpp (the only file that compiles miniaudio).
#include <cstdint>
#include <string>

namespace qbaiout {

struct Output;
// the default output, `channels` interleaved float channels at `rate`; null with the reason in *error
Output *open(int rate, int channels, std::string *error);
bool write(Output *o, const float *pcm, int64_t frames);   // queued
void pause(Output *o);
void resume(Output *o);
void stop(Output *o);                                      // drops the queue, the played position goes back to 0
void close(Output *o);
int64_t played(Output *o);                                 // frames played of what was written since the open or the last stop

} // namespace qbaiout
