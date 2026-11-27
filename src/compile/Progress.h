#pragma once
// Compiling runs in the main thread (the compiler reads the open documents). Its long loops call
// progress::tick(), which hands over to a handler at most every 100 ms: the compile window lets the
// event loop run there (no user input), so the window is drawn and the program's freeze watchdog,
// a timer of the main thread, does not report a freeze (0.64.1; a 45,000-word manuscript with tables
// held the main thread 33 s on a two-core machine). Without a handler tick() costs a clock read.
#include <functional>

namespace qb {
namespace compile {
namespace progress {

// The handler gets the share done so far (0 to 1, or -1 when not known).
void setHandler(std::function<void(double)> handler);
// done of total (total 0: not known)
void tick(int done = 0, int total = 0);

// 0.99.0 (UR-09): Cancel. The window sets the flag (requestCancel); the compiler looks at it after each document
// (cancelled()) and the converter wait loop stops the outside program. documentDone() tells "12 of 148" at once
// (not held back to 100 ms) and lets the handler run, so a click on Cancel is seen between two documents.
void requestCancel();
bool cancelled();
void clearCancel();
void documentDone(int done, int total);
int documentsDone();
int documentsTotal();

// Sets a handler for as long as it lives.
class Scope
{
public:
    explicit Scope(std::function<void(double)> handler) { setHandler(std::move(handler)); }
    ~Scope() { setHandler(nullptr); }
    Scope(const Scope &) = delete;
    Scope &operator=(const Scope &) = delete;
};

} // namespace progress
} // namespace compile
} // namespace qb
