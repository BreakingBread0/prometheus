#ifdef __MINGW64__
#include <intrin.h>

// surely this is just a linking error? why is there no `intrin.c`????????????
// this probably returns the _ReturnAddress() of itself, not the calling function?
// maybe make it a macro ???

// extern "C" void* _ReturnAddress(void)
// {
//     return __builtin_extract_return_addr(__builtin_return_address(0));
// }
//

// nvm, i think it was incorrectly removed in
// `https://sourceforge.net/p/mingw-w64/mingw-w64/ci/e6ac7e4230c9f075c1c243e9c2ce2ec56bbf900c/`
// mingw bug? my stupidity? 50/50
// TODO make this actually replace the builtin `_ReturnAddress()`..
#define _ReturnAddress()		__builtin_extract_return_addr(__builtin_return_address(0));
#endif