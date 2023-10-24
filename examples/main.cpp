#include <mm2/mm2.hpp>

struct Mm2 : mm2::rx::CrtpBase<Mm2> {
  friend mm2::rx::CrtpBase<Mm2>;

  constexpr Mm2() = default;

private:
  // Set directon
  void direction(uint32_t addr, int32_t dir) {}

  // Set speed
  void speed(uint32_t addr, int32_t speed) {}

  // Reverse direction
  void reverse(uint32_t addr) {}

  // Set function inputs
  void function(uint32_t addr, uint32_t mask, uint32_t state) {}

  // Read CV
  uint8_t readCv(uint32_t cv_addr) { return {}; }

  // Write CV
  uint8_t writeCv(uint32_t cv_addr, uint8_t value) { return {}; }

  // Timer interrupt calls receive with captured value
  void interrupt() {
    uint32_t const timer_value{/* captured timer value */};
    receive(timer_value);
  }
};

int main() { Mm2 mm2{}; }