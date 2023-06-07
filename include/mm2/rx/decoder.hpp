// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Decoder
///
/// \file   mm2/rx/decoder.hpp
/// \author Vincent Hamp
/// \date   29/11/2022

#pragma once

#include "readable.hpp"
#include "writable.hpp"

namespace mm2::rx {

template<typename T>
concept Decoder = Readable<T> && Writable<T> &&
                  requires(T t,
                           uint32_t addr,
                           int32_t dir,
                           int32_t notch,
                           uint32_t mask,
                           uint32_t state) {
                    { t.direction(addr, dir) } -> std::same_as<void>;
                    { t.notch(addr, notch) } -> std::same_as<void>;
                    { t.reverse(addr) } -> std::same_as<void>;
                    { t.function(addr, mask, state) } -> std::same_as<void>;
                  };

}  // namespace mm2::rx