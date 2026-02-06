#include "cel/gen/def.h"
#include CEL_END_HEADER

#if CEL_CASCADE < CEL_GEN
#    define CEL_CASCADE__TRIGGER
#endif

#if CEL_GEN_MAX > 16
#    error "<cel/gen/end.h> does not cover all valid codegen depths"
#elif CEL_GEN == 0
#    error "<cel/gen/end.h> without matching <cel/gen/begin.h>"
#elif CEL_GEN == 16
#    undef CEL_GEN
#    undef CEL_CASCADE16
#    undef CEL_FN16
#    undef CEL_ALLOCATOR16
#    undef CEL_END_HEADER16
#    define CEL_GEN 15
#elif CEL_GEN == 15
#    undef CEL_GEN
#    undef CEL_CASCADE15
#    undef CEL_FN15
#    undef CEL_ALLOCATOR15
#    undef CEL_END_HEADER15
#    define CEL_GEN 14
#elif CEL_GEN == 14
#    undef CEL_GEN
#    undef CEL_CASCADE14
#    undef CEL_FN14
#    undef CEL_ALLOCATOR14
#    undef CEL_END_HEADER14
#    define CEL_GEN 13
#elif CEL_GEN == 13
#    undef CEL_GEN
#    undef CEL_CASCADE13
#    undef CEL_FN13
#    undef CEL_ALLOCATOR13
#    undef CEL_END_HEADER13
#    define CEL_GEN 12
#elif CEL_GEN == 12
#    undef CEL_GEN
#    undef CEL_CASCADE12
#    undef CEL_FN12
#    undef CEL_ALLOCATOR12
#    undef CEL_END_HEADER12
#    define CEL_GEN 11
#elif CEL_GEN == 11
#    undef CEL_GEN
#    undef CEL_CASCADE11
#    undef CEL_FN11
#    undef CEL_ALLOCATOR11
#    undef CEL_END_HEADER11
#    define CEL_GEN 10
#elif CEL_GEN == 10
#    undef CEL_GEN
#    undef CEL_CASCADE10
#    undef CEL_FN10
#    undef CEL_ALLOCATOR10
#    undef CEL_END_HEADER10
#    define CEL_GEN 9
#elif CEL_GEN == 9
#    undef CEL_GEN
#    undef CEL_CASCADE9
#    undef CEL_FN9
#    undef CEL_ALLOCATOR9
#    undef CEL_END_HEADER9
#    define CEL_GEN 8
#elif CEL_GEN == 8
#    undef CEL_GEN
#    undef CEL_CASCADE8
#    undef CEL_FN8
#    undef CEL_ALLOCATOR8
#    undef CEL_END_HEADER8
#    define CEL_GEN 7
#elif CEL_GEN == 7
#    undef CEL_GEN
#    undef CEL_CASCADE7
#    undef CEL_FN7
#    undef CEL_ALLOCATOR7
#    undef CEL_END_HEADER7
#    define CEL_GEN 6
#elif CEL_GEN == 6
#    undef CEL_GEN
#    undef CEL_CASCADE6
#    undef CEL_FN6
#    undef CEL_ALLOCATOR6
#    undef CEL_END_HEADER6
#    define CEL_GEN 5
#elif CEL_GEN == 5
#    undef CEL_GEN
#    undef CEL_CASCADE5
#    undef CEL_FN5
#    undef CEL_ALLOCATOR5
#    undef CEL_END_HEADER5
#    define CEL_GEN 4
#elif CEL_GEN == 4
#    undef CEL_GEN
#    undef CEL_CASCADE4
#    undef CEL_FN4
#    undef CEL_ALLOCATOR4
#    undef CEL_END_HEADER4
#    define CEL_GEN 3
#elif CEL_GEN == 3
#    undef CEL_GEN
#    undef CEL_CASCADE3
#    undef CEL_FN3
#    undef CEL_ALLOCATOR3
#    undef CEL_END_HEADER3
#    define CEL_GEN 2
#elif CEL_GEN == 2
#    undef CEL_GEN
#    undef CEL_CASCADE2
#    undef CEL_FN2
#    undef CEL_ALLOCATOR2
#    undef CEL_END_HEADER2
#    define CEL_GEN 1
#else
#    undef CEL_GEN
#    undef CEL_CASCADE1
#    undef CEL_FN1
#    undef CEL_ALLOCATOR1
#    undef CEL_END_HEADER1
#    define CEL_GEN 0
#endif

#ifdef CEL_CASCADE__TRIGGER
#    undef CEL_CASCADE__TRIGGER
#    include "cel/gen/end.h"
#endif
