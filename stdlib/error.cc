#include <stdint.h>
#include <iostream>
#include <stdexcept>

struct TypeError: public std::runtime_error {
  int32_t start;
  int32_t end;
  int8_t got;
  int8_t expect;
  TypeError(int32_t start, int32_t end, int8_t got, int8_t expect)
    : std::runtime_error("TypeError")
    , start(start)
    , end(end)
    , got(got)
    , expect(expect) {}
};

struct ArgCntError: public std::runtime_error {
  int32_t start;
  int32_t end;
  int16_t got;
  int16_t expect;
  ArgCntError(int32_t start, int32_t end, int16_t got, int16_t expect)
    : std::runtime_error("ArgCntError")
    , start(start)
    , end(end)
    , got(got)
    , expect(expect) {}
};



extern "C" {

  void emit_type_error(uint32_t start, uint32_t end, uint8_t got, uint8_t expect) {
    std::cout << "EMIT TYPE ERROR " << start << " " << end << " " << (int)got << " " << (int)expect << std::endl;
    throw TypeError(start, end, got, expect);
  }
  
  void emit_arg_cnt_error(int32_t start, int32_t end, int16_t got, int16_t expect) {
    std::cout << "EMIT ARGCNT ERROR " << start << " " << end << " " << (int)got << " " << (int)expect << std::endl;
	throw ArgCntError(start, end, got, expect);
  }
}
