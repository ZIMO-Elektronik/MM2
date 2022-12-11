// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Addresses
///
/// \file   mm2/addresses.hpp
/// \author Vincent Hamp
/// \date   29/11/2022

#pragma once

#include "address.hpp"

namespace mm2 {

struct Addresses {
  Address primary{};
  Address consist{};
};

}  // namespace mm2