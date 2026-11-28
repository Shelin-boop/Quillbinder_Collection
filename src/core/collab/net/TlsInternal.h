#pragma once
// Botan's key and certificate behind a DeviceIdentity (0.99.0); only the .cpp files of this folder include it.
#ifdef QB_HAVE_BOTAN_TLS
#include "core/collab/net/DeviceIdentity.h"

#include <botan/auto_rng.h>
#include <botan/pk_keys.h>
#include <botan/x509cert.h>
#include <memory>

namespace qb {
namespace collab {
namespace net {

struct DeviceIdentity::Impl {
    std::shared_ptr<Botan::Private_Key> key;
    std::shared_ptr<Botan::X509_Certificate> cert;
};

// the process's random number generator (used from one thread at a time: the program's main thread)
std::shared_ptr<Botan::RandomNumberGenerator> sharedRng();

} // namespace net
} // namespace collab
} // namespace qb
#endif
