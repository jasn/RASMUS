#include "lib.h"
#include <cstring>
#include <string>
#include <iostream>

extern "C" {

  struct rm_Text {

    rm_Text(const char *cptr) : str(cptr) { }

    std::string str;
  };

  void * rm_getConstText(const char *cptr) {
    return new rm_Text(cptr);    
  }

  void rm_printText(void * ptr) {
    std::cout << reinterpret_cast<rm_Text*>(ptr)->str << std::endl;
  }

}
