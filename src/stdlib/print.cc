#include <iostream>
#include <stdint.h>
#include "lib.h"
#include <shared/type.hh>

extern "C" {
  void rm_print(uint8_t t, int64_t v) {
    switch (Type(t)) {
    case TBool:
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
    case TInt:
      std::cout << v << std::endl;
      break;
    case TText: //It's a Text
      rm_printText((rm_object*)v);
      break;
    case TRel:
      rm_printRel((rm_object*)v);
      break;
    default:
      std::cout << Type(t) << "(" << v << ")" << std::endl;
      break;
    }
  }

}
