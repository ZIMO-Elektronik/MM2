#include <mm2/mm2.hpp>

#define TIMER_VALUE 42u

struct Mm2 : mm2::rx::CrtpBase<Mm2> {
  friend mm2::rx::CrtpBase<Mm2>;

  constexpr Mm2() = default;

private:
  // Set directon
  void direction(uint32_t addr, int32_t dir) {}

  // Set notch
  void notch(uint32_t addr, int32_t notch) {}

  // Reverse direction
  void reverse(uint32_t addr) {}

  // Set function inputs
  void function(uint32_t addr, uint32_t mask, uint32_t state) {}

  // Read CV
  uint8_t readCv(uint32_t addr) {}

  // Write CV
  uint8_t writeCv(uint32_t addr, uint8_t value) {}

  // Timer interrupt calls receive with captured value
  void interrupt() { receive(TIMER_VALUE); }
};

int main() { Mm2 mm2{}; }