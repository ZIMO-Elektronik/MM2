// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Receive base
///
/// \file   mm2/rx/crtp_base.hpp
/// \author Vincent Hamp
/// \date   29/11/2022

#pragma once

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <ztl/bits.hpp>
#include <ztl/inplace_deque.hpp>
#include <ztl/math.hpp>
#include "../addresses.hpp"
#include "../bit.hpp"
#include "decoder.hpp"
#include "packet.hpp"

namespace mm2 {

/// Make EFGB mask for easier banging
///
/// \param  efgh  4 bits EFGH
/// \return EFGB mask
constexpr uint32_t efgh_mask(uint32_t efgh) {
  uint32_t retval{};
  if (efgh & 0b0001u) retval |= 1u << 0u;
  if (efgh & 0b0010u) retval |= 1u << 2u;
  if (efgh & 0b0100u) retval |= 1u << 4u;
  if (efgh & 0b1000u) retval |= 1u << 6u;
  return retval;
}

/// Make AEBFCGDH mask for easier banging
///
/// \param  dcba  4 bits DCBA
/// \param  efgh  4 bits EFGH
/// \return AEBFCGDH mask
constexpr uint32_t aebfcgdh_mask(uint32_t dcba, uint32_t efgh) {
  uint32_t retval{};
  if (efgh & 0b0001u) retval |= 1u << 0u;
  if (dcba & 0b0001u) retval |= 1u << 7u;
  if (efgh & 0b0010u) retval |= 1u << 2u;
  if (dcba & 0b0010u) retval |= 1u << 5u;
  if (efgh & 0b0100u) retval |= 1u << 4u;
  if (dcba & 0b0100u) retval |= 1u << 3u;
  if (efgh & 0b1000u) retval |= 1u << 6u;
  if (dcba & 0b1000u) retval |= 1u << 1u;
  return retval;
}

/// Decode direction
///
/// \param  data    Data to decode
/// \return int32_t Direction
/// \return {}      No direction
constexpr std::optional<int32_t> decode_direction(uint8_t data) {
  data = static_cast<uint8_t>(data & efgh_mask(0b1110u));
  if (data == efgh_mask(0b1010u)) return -1;
  if (data == efgh_mask(0b0100u)) return 1;
  return {};
}

/// Decode speed
///
/// \param  data    Data to decode
/// \return int32_t Speed
/// \return {}      No speed
constexpr std::optional<int32_t> decode_speed(uint8_t data) {
  uint32_t const value{
    (data & ztl::make_mask(7u)) >> 7u | (data & ztl::make_mask(5u)) >> 4u |
    (data & ztl::make_mask(3u)) >> 1u | (data & ztl::make_mask(1u)) << 2u};
  if (value == 0u) return 0;
  if (value == 1u) return {};
  return ztl::lerp<int32_t>(static_cast<int32_t>(value) - 1, 0, 14, 0, 255);
}

/// Decode exceptions
///
/// \param  data      Data to decode
/// \return uint32_t  Exception
/// \return {}        No exception
constexpr std::optional<uint32_t> decode_exception(uint8_t data) {
  switch (data) {
    case aebfcgdh_mask(0b0011u, 0b1010u): return 2u;
    case aebfcgdh_mask(0b0100u, 0b1010u): return 3u;
    case aebfcgdh_mask(0b0110u, 0b1010u): return 5u;
    case aebfcgdh_mask(0b0111u, 0b1010u): return 6u;
    case aebfcgdh_mask(0b1011u, 0b0101u): return 10u;
    case aebfcgdh_mask(0b1100u, 0b0101u): return 11u;
    case aebfcgdh_mask(0b1110u, 0b0101u): return 13u;
    case aebfcgdh_mask(0b1111u, 0b0101u): return 14u;
  }
  return {};
}

namespace rx {

/// CRTP base for receiving MM/MM2
///
/// \tparam T Type to downcast to
template<typename T>
struct CrtpBase {
  friend T;

  /// Initialize
  void init() { config(); }

  /// Enable
  void enable() {
    if (_enabled) return;
    _enabled = true;
  }

  /// Disable
  void disable() {
    if (!_enabled) return;
    _enabled = false;
    reset();
    _mode = Mode::Unknown;
  }

  /// Encoding of commands bit by bit
  ///
  /// \param  time  Time in µs
  void receive(uint32_t time) {
    auto const bit{time2bit(time)};
    if (bit == Invalid) return reset();

    // Alternate halfbit <-> bit
    _is_halfbit = !_is_halfbit;
    if (!_is_halfbit) return;

    // If queue is full return
    if (full(_deque)) return;

    // Successfully received a bit -> enter state machine
    switch (_state) {
      case State::Address: {
        auto& addr{end(_deque)->addr};
        addr = static_cast<uint8_t>((addr << 1u) | bit);
        if (++_bit_count < 8uz) return;
        _state = State::Function;
        _bit_count = 0uz;
        break;
      }
      case State::Function:
        if (!_bit_count) end(_deque)->func = bit;
        if (++_bit_count < 2uz) return;
        _state = State::Data;
        _bit_count = 0uz;
        break;
      case State::Data: {
        auto& data{end(_deque)->data};
        data = static_cast<uint8_t>((data << 1u) | bit);
        if (++_bit_count < 8uz) return;
        _state = State::Address;
        _bit_count = 0uz;
        _deque.push_back();  // Successfully red a packet
        break;
      }
    }
  }

  /// Execute received commands
  ///
  /// \return true  Command to own address
  /// \return false Command to other address
  bool execute() {
    if (empty(_deque)) return false;
    auto const retval{executeThreadMode()};
    _deque.front() = {};
    _deque.pop_front();
    return retval;
  }

  /// Service mode
  ///
  /// \return true  Service mode active
  /// \return false Operations mode active
  bool serviceMode() const { return _mode == Mode::Service; }

private:
  constexpr CrtpBase() = default;
  Decoder auto& impl() { return static_cast<T&>(*this); }
  Decoder auto const& impl() const { return static_cast<T const&>(*this); }

  /// Configure
  void config() {
    _addrs = {.primary = impl().readCv(1u - 1u),
              .consist = impl().readCv(19u - 1u) & 0b0111'1111u};
    _follow_up_count = impl().readCv(10u - 1u) & 0b11u;
  }

  /// Execute in thread mode
  ///
  /// \return true  Command to own address
  /// \return false Command to other address
  bool executeThreadMode() {
    // If address was found, store it
    if (auto const addr{decode_address(_deque.front().addr)})
      _deque.front().addr = *addr;
    else return false;

    switch (_mode) {
      case Mode::Unknown:
        if (isDirectionChange()) _mode = Mode::Service;
        [[fallthrough]];
      case Mode::Service: return executeService();
      case Mode::Operations: return executeOperations();
    }
  }

  /// Execute commands in service mode
  ///
  /// \return false
  bool executeService() {
    auto const [addr, func, data]{_deque.front()};

    // Check if address is either 0, 80 or primary address
    auto is_addr{
      [&] { return addr == 0u || addr == 80u || addr == _addrs.primary; }};

    switch (_prog.state) {
      case Prog::Entry:
        if (is_addr() && data == 0u) break;
        else if (is_addr() && isDirectionChange()) _prog.state = Prog::Wait0_1;
        else _mode = Mode::Operations;
        break;

      case Prog::Wait0_1:
        if (is_addr() && isDirectionChange()) break;
        else if (is_addr() && !isDirectionChange()) _prog.state = Prog::Address;
        break;

      case Prog::Address:
        if (is_addr() && !isDirectionChange()) break;
        else if (isDirectionChange()) {
          _prog.addr = addr;
          _prog.state = Prog::Wait0_2;
        }
        break;

      case Prog::Wait0_2:
        if (isDirectionChange()) break;
        _prog.state = Prog::Value;
        break;

      case Prog::Value:
        if (!isDirectionChange()) break;
        _prog.value = addr;
        _prog.state = Prog::Wait0_1;
        impl().writeCv(_prog.addr - 1u, _prog.value);
        break;
    }

    return false;
  }

  /// Execute commands in operations mode
  ///
  /// \return true  Command to own address
  /// \return false Command to other address
  bool executeOperations() {
    auto const [addr, func, data]{_deque.front()};
    if (!addr) return false;  // Zero is no valid address

    auto const fshift{fShift()};

    // Address check
    if (addr != _addrs.primary && addr != _addrs.consist && !fshift)
      return false;

    // Valid packets must be received twice
    if (_last_valid_own_packet != _deque.front()) {
      _last_valid_own_packet = _deque.front();
      return true;
    }

    // Function
    if (addr == _addrs.primary) impl().function(addr, ztl::make_mask(0u), func);

    // MM1 or MM2
    if (auto const is_mm2{(data & 0b11'00'00'00u) == 0b01'00'00'00u ||
                          (data & 0b00'11'00'00u) == 0b00'01'00'00u ||
                          (data & 0b00'00'11'00u) == 0b00'00'01'00u ||
                          (data & 0b00'00'00'11u) == 0b00'00'00'01u}) {
      // Follow-up address needs to act like normal one
      uint32_t const addr_to_fwd{_deque.front().addr - (fshift >> 2u)};
      motorola2(addr_to_fwd, fshift);
    } else if (!fshift) motorola1();

    return true;
  }

  /// Execute MM1 command
  void motorola1() {
    auto const addr{_deque.front().addr}, data{_deque.front().data};
    if (auto const speed{decode_speed(data)}) {
      _last_cmd_was_dir_change = false;
      impl().speed(addr, *speed);
    }
    // Direction
    else if (!_last_cmd_was_dir_change) {
      _last_cmd_was_dir_change = true;
      impl().reverse(addr);
    }
  }

  /// Execute MM2 command
  ///
  /// \param  addr    Address
  /// \param  fshift  0   Packet contains base address
  ///                 >0  Packet contains follow-up address
  void motorola2(uint32_t addr, uint32_t fshift) {
    auto const data{_deque.front().data};

    // Command is either exception
    if (auto const exc{decode_exception(data)})
      motorola2Exception(addr, fshift, *exc);
    else {
      // ... or direction
      if (auto const dir{decode_direction(data)})
        motorola2Direction(addr, fshift, *dir);
      // ... or function
      else motorola2Function(addr, fshift, data);
    }

    // Speed, ignore direction changes
    if (auto const speed{decode_speed(data)}; speed && !fshift)
      impl().speed(addr, *speed);
  }

  /// Execute MM2 exception
  ///
  /// \param  addr    Address
  /// \param  fshift  0   Packet contains base address
  ///                 >0  Packet contains follow-up address
  /// \param  exc     Exception
  void motorola2Exception(uint32_t addr, uint32_t fshift, uint32_t exc) {
    switch (exc) {
      case 2u: impl().function(addr, ztl::make_mask(1u) << fshift, 0u); break;
      case 3u: impl().function(addr, ztl::make_mask(2u) << fshift, 0u); break;
      case 5u: impl().function(addr, ztl::make_mask(3u) << fshift, 0u); break;
      case 6u: impl().function(addr, ztl::make_mask(4u) << fshift, 0u); break;
      case 10u:
        impl().function(
          addr, ztl::make_mask(1u) << fshift, ztl::make_mask(1u) << fshift);
        break;
      case 11u:
        impl().function(
          addr, ztl::make_mask(2u) << fshift, ztl::make_mask(2u) << fshift);
        break;
      case 13u:
        impl().function(
          addr, ztl::make_mask(3u) << fshift, ztl::make_mask(3u) << fshift);
        break;
      case 14u:
        impl().function(
          addr, ztl::make_mask(4u) << fshift, ztl::make_mask(4u) << fshift);
        break;
    }
  }

  /// Execute MM2 direction
  ///
  /// \param  addr    Address
  /// \param  fshift  0   Packet contains base address
  ///                 >0  Packet contains follow-up address
  /// \param  dir     Direction
  void motorola2Direction(uint32_t addr, uint32_t fshift, int32_t dir) {
    if (fshift) return;
    auto const reverse{addr == _addrs.primary
                         ? impl().readCv(29u - 1u) & ztl::make_mask(0u)
                         : impl().readCv(19u - 1u) & ztl::make_mask(7u)};
    impl().direction(addr, reverse ? dir * -1 : dir);
  }

  /// Execute MM2 function
  ///
  /// \param  addr    Address
  /// \param  fshift  0   Packet contains base address
  ///                 >0  Packet contains follow-up address
  /// \param  data    Data
  void motorola2Function(uint32_t addr, uint32_t fshift, uint8_t data) {
    switch (data & efgh_mask(0b1110u)) {
      case efgh_mask(0b1100u):
        impl().function(addr,
                        ztl::make_mask(1u) << fshift,
                        data & 0b1u ? ztl::make_mask(1u) << fshift : 0u);
        break;
      case efgh_mask(0b0010u):
        impl().function(addr,
                        ztl::make_mask(2u) << fshift,
                        data & 0b1u ? ztl::make_mask(2u) << fshift : 0u);
        break;
      case efgh_mask(0b0110u):
        impl().function(addr,
                        ztl::make_mask(3u) << fshift,
                        data & 0b1u ? ztl::make_mask(3u) << fshift : 0u);
        break;
      case efgh_mask(0b1110u):
        impl().function(addr,
                        ztl::make_mask(4u) << fshift,
                        data & 0b1u ? ztl::make_mask(4u) << fshift : 0u);
        break;
    }
  }

  /// Check whether command is direction change
  ///
  /// \return true  Command is direction change
  /// \return false Command is not direction change
  bool isDirectionChange() const { return _deque.front().data == 0xC0u; }

  /// Check if address is a follow-up one. In case it is return a shift which
  /// accounts for 4x function positions so that the first follow-up address
  /// handles F5-F8, the second one F9-F12, ...
  ///
  /// \return F-shift
  uint32_t fShift() const {
    auto const addr{_deque.front().addr};
    // Zero is no valid address
    if (!addr) return 0u;
    // Check if address is one of three chaining addresses
    for (auto i{_follow_up_count}; i; --i)
      if (addr == _addrs.primary + i) return i << 2u;
    return 0u;
  }

  /// Flush the current packet
  void flush() { *end(_deque) = {}; }

  /// Reset
  void reset() {
    flush();
    _bit_count = 0uz;
    _is_halfbit = false;
    _state = State::Address;
  }

  size_t _bit_count{};
  ztl::inplace_deque<Packet, MM2_RX_DEQUE_SIZE> _deque{};  ///< Task queue
  Packet _last_valid_own_packet{};  ///< Copy of last packet for own address

  enum class State : uint8_t { Address, Function, Data } _state{};
  enum class Mode : uint8_t { Unknown, Service, Operations } _mode{};

  Addresses _addrs{};

  struct Prog {
    enum : uint8_t { Entry, Wait0_1, Address, Wait0_2, Value } state;
    uint8_t addr;
    uint8_t value;
  } _prog{};

  // Not bitfields as those are most likely mutated in interrupt context
  bool _is_halfbit{};

  uint8_t _follow_up_count : 2 {};  // Limit to 3x follow-up addresses
  bool _enabled : 1 {};
  bool _last_cmd_was_dir_change : 1 {};
};

}  // namespace rx

}  // namespace mm2