// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Timing
///
/// \file   mm2/timing.hpp
/// \author Vincent Hamp
/// \date   29/11/2022

#pragma once

#include "bit.hpp"

namespace mm2 {

enum Timing {
  Bit1Norm = 182u,           ///< Norm timing for a 1-bit
  Bit1Min = Bit1Norm - 20u,  ///< Minimal timing for a 1-bit
  Bit1Max = Bit1Norm + 20u,  ///< Maximal timing for a 1-bit
  Bit0Norm = 26u,            ///< Norm timing for a 0-bit
  Bit0Min = Bit0Norm - 6u,   ///< Minimal timing for a 0-bit
  Bit0Max = Bit0Norm + 6u,   ///< Maximal timing for a 0-bit
};

/// Convert timing to bit
///
/// \param  time  Last pulse on tracks in us
/// \return Bit
constexpr Bit time2bit(uint32_t time) {
  if (time >= Bit1Min && time <= Bit1Max) return _1;
  else if (time >= Bit0Min && time <= Bit0Max) return _0;
  else return Invalid;
}

}  // namespace mm2