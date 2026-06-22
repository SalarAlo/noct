#pragma once

#define NOCT_API_VERSION_MAJOR 0
#define NOCT_API_VERSION_MINOR 1
#define NOCT_API_VERSION_PATCH 0

namespace Noct {

inline constexpr int ApiVersionMajor = NOCT_API_VERSION_MAJOR;
inline constexpr int ApiVersionMinor = NOCT_API_VERSION_MINOR;
inline constexpr int ApiVersionPatch = NOCT_API_VERSION_PATCH;

}

namespace noct = Noct;
