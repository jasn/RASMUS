#include <iostream>
#include <stdint.h>
#include "lib.h"

extern "C" {
  void rm_print(uint8_t t, int64_t v) {
    switch (t) {
    case 0: //It is a bool
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
    case 1: //Its an int
      std::cout << v << std::endl;
      break;
    case 3:
      rm_printText((void*)v);
      break;
    default:
      std::cout << "Unknown type" << std::endl;
      break;
    }
  }

}
