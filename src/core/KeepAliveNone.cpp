// Linux and Windows: nothing to keep alive (the program keeps running while its window is out of sight).
#include "core/KeepAlive.h"

namespace qb {
namespace keepalive {

void platformChanged(bool, const QStringList &) {}

} // namespace keepalive
} // namespace qb
