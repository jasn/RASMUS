#include <stdint.h>
#include <stddef.h>

extern "C" {

  void * rm_getConstText(const char *cptr);
  
  void rm_print(uint8_t t, int64_t v);

  void rm_emitTypeError(uint32_t start, uint32_t end, uint8_t got, uint8_t expect);

  void rm_emitArgCntError(int32_t start, int32_t end, int16_t got, int16_t expect);

  int8_t rm_interactiveWrapper(char * txt, void (* fct)() );

  void rm_printText(void * ptr);
}

