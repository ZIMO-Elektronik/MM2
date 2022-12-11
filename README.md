# MM2

<img src="data/images/logo.png" width="10%" align="right">

MM2 is an implementation of the Motorola 1 and 2 (MM1/2) protocol. Details on the format can be found on [Dr. König's Märklin-Digital-Page](http://www.drkoenig.de/digital/digital.htm) or in the data folder in case this site goes offline. This library supports MM1 and 2 as well as some additions like CV programming and follow-up addresses for supporting more than 5 functions.

## Examples
Currently there is only one class for receiving MM2.
- mm2::rx::CrtpBase

As the names suggest this class relies on [CRTP](https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern) to implement static polymorphism. The template argument of the base classes is checked with a concept called Decoder.

Here is an example for how a class which implements it might look. The friend declarations are only necessary if the methods the base(s) need to call are not public.
```cpp
#include <mm2/mm2.hpp>

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
};
```