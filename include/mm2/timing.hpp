// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Timing
///
/// \file   mm2/timing.hpp
/// \author Vincent Hamp
/// \date   29/11/2022

#pragma once

#include <cstdint>

namespace mm2 {

enum Timing {
  Bit1 = 182u,          ///< Standard timing for a 1-bit
  Bit1Min = Bit1 - 20u, ///< Minimal timing for a 1-bit
  Bit1Max = Bit1 + 20u, ///< Maximal timing for a 1-bit
  Bit0 = 26u,           ///< Standard timing for a 0-bit
  Bit0Min = Bit0 - 6u,  ///< Minimal timing for a 0-bit
  Bit0Max = Bit0 + 6u,  ///< Maximal timing for a 0-bit
};

} // namespace mm2
