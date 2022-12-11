// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Packet
///
/// \file   mm2/rx/packet.hpp
/// \author Vincent Hamp
/// \date   29/11/2022

#pragma once

#include "../address.hpp"

namespace mm2::rx {

struct Packet {
  Address addr{};
  uint8_t func{};
  uint8_t data{};
  constexpr bool operator==(Packet const&) const = default;
};

}  // namespace mm2::rx