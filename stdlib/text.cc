#include "lib.h"
#include <cstring>
#include <string>
#include <iostream>

extern "C" {

  struct rm_Text {

    rm_Text(const char *cptr) : str(cptr) { }

    static rm_Text* concat(const rm_Text *lhs, const rm_Text *rhs) {
      return new rm_Text(((*lhs).str + (*rhs).str).c_str());
    }

    bool find(const rm_Text *needle) const {
      return str.find(needle->str) != std::string::npos;
    }

    std::string str;
  };

  void * rm_getConstText(const char *cptr) {
    return new rm_Text(cptr);    
  }

  void * rm_concatText(const void *lhs, const void *rhs) {
    const rm_Text *lhst = reinterpret_cast<const rm_Text*>(lhs);
    const rm_Text *rhst = reinterpret_cast<const rm_Text*>(rhs);
    
    return rm_Text::concat(lhst, rhst);

  }

  int8_t rm_substringSearch(const void *lhs, const void *rhs) {
    const rm_Text *lhst = reinterpret_cast<const rm_Text*>(lhs);
    const rm_Text *rhst = reinterpret_cast<const rm_Text*>(rhs);

    return rhst->find(lhst);

  }

  void rm_printText(void * ptr) {
    std::cout << reinterpret_cast<rm_Text*>(ptr)->str << std::endl;
  }

}
