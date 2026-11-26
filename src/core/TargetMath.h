#pragma once
// Word targets with a sign (0.95.0). A positive target is a writing target: the bar fills as the count rises. A negative
// target is a cutting target ("cut 2,000 words today", "bring this chapter down to 4,000 words"): its bar fills as the
// count falls. 0 is no target.
#include <QtGlobal>

namespace qb {
namespace targetmath {

// A document's or the draft's count against its target: positive, count / target; negative (down to N words), full once
// the count is N or lower, N / count above it. -1: no target.
inline double fill(int count, int target)
{
    if (target == 0)
        return -1.0;
    if (target > 0)
        return qBound(0.0, double(qMax(0, count)) / target, 1.0);
    const int n = -target;
    if (count <= n)
        return 1.0;
    return double(n) / count;
}

// Today's count against the session target: the count is net (below 0 when more words were removed than written) while
// the target is negative; positive target, as before.
inline double sessionFill(int count, int target)
{
    if (target == 0)
        return -1.0;
    if (target > 0)
        return qBound(0.0, double(qMax(0, count)) / target, 1.0);
    return qBound(0.0, double(count) / target, 1.0); // both below 0 when words were cut
}

} // namespace targetmath
} // namespace qb
