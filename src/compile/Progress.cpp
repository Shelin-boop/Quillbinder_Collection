#include "compile/Progress.h"

#include <QElapsedTimer>

namespace qb {
namespace compile {
namespace progress {

namespace {
std::function<void(double)> g_handler;
QElapsedTimer g_since;
bool g_inHandler = false;
bool g_cancel = false;
int g_docsDone = 0, g_docsTotal = 0;
} // namespace

void setHandler(std::function<void(double)> handler)
{
    g_cancel = false;
    g_docsDone = g_docsTotal = 0;
    g_handler = std::move(handler);
    g_since.start();
}

void tick(int done, int total)
{
    if (!g_handler || g_inHandler || g_since.elapsed() < 100)
        return;
    g_since.restart();
    g_inHandler = true; // a handler that lets the event loop run must not come back in here
    g_handler(total > 0 ? double(done) / total : -1.0);
    g_inHandler = false;
}

void requestCancel() { g_cancel = true; }
bool cancelled() { return g_cancel; }
void clearCancel() { g_cancel = false; }
int documentsDone() { return g_docsDone; }
int documentsTotal() { return g_docsTotal; }

void documentDone(int done, int total)
{
    g_docsDone = done;
    g_docsTotal = total;
    if (!g_handler || g_inHandler)
        return;
    g_since.restart();
    g_inHandler = true;
    g_handler(total > 0 ? double(done) / total : -1.0);
    g_inHandler = false;
}

} // namespace progress
} // namespace compile
} // namespace qb
