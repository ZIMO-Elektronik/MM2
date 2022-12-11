// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Writable
///
/// \file   mm2/rx/writable.hpp
/// \author Vincent Hamp
/// \date   29/11/2022

#pragma once

#include <concepts>
#include <cstdint>

namespace mm2::rx {

template<typename T>
concept Writable = requires(T t, uint32_t addr, uint8_t value) {
                     { t.writeCv(addr, value) } -> std::convertible_to<uint8_t>;
                   };

}  // namespace mm2::rx