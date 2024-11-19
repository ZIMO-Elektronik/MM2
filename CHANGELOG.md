# Changelog

## 0.10.1
- Bugfix CMake always includes tests

## 0.10.0
- Replace `int32_t` direction (1, -1) with `bool` (1, 0)
- Add `Direction` enumeration

## 0.9.0
- Use `speed` instead of `notch`
- Update to ZTL 0.18.0

## 0.8.9
- Update to ZTL 0.17.0

## 0.8.8
- Add service mode getter

## 0.8.7
- Bugfix MM1/2 detection

## 0.8.6
- `time2bit` returns `Bit` enum instead of `std::optional` (overhead is pretty significant)
- Bugfix deque not popped in error path

## 0.8.5
- Use CPM.cmake

## 0.8.4
- Decoder concept writeCv has uint8_t return type

## 0.8.3
- Update to ZTL 0.16.0

## 0.8.2
- Update to ZTL 0.15.0

## 0.8.1
- Update to ZTL 0.14.0

## 0.8.0
- [Semantic versioning](https://semver.org)
- Renamed concept Impl to Decoder
- Renamed namespace receive to rx

## 0.7
- Update to ZTL v0.13

## 0.6
- Update to ZTL v0.12

## 0.5
- Direction bits (CV29:1 and CV19:7)

## 0.4
- Removed CMake exports
- Update to ZTL v0.11

## 0.3
- Update to ZTL v0.9

## 0.2
- Zero is no valid address
- Only evaluate first function bit

## 0.1
- First release