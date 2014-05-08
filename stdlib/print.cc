#include <iostream>
#include <stdint.h>
#include "lib.h"

extern "C" {
  void rm_print(uint8_t t, int64_t v) {
    switch (t) {
    case 0: //It is a Bool
      switch (v) {
      case 0:
	std::cout << "false" << std::endl;
	break;
      case 1:
	std::cout << "true" << std::endl;
	break;
      default:
	std::cout << "undefined" << std::endl;
	break;
      }
      break;
    case 1: //Its an Int
      std::cout << v << std::endl;
      break;
    case 3: //It's a Text
      rm_printText((void*)v);
      break;
    default:
      std::cout << "Unknown type" << std::endl;
      break;
    }
  }

}
