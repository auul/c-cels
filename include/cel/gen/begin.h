#include "cel/gen/def.h"

#if CEL_GEN == CEL_GEN_MAX
#    error "Maximum codegen depth exceeded"
#elif CEL_GEN == 16
#    error "<cel/gen/begin.h> does not cover all valid codegen depths"
#elif CEL_GEN == 15
#    undef CEL_GEN
#    define CEL_GEN          16
#    define CEL_CASCADE16    16
#    define CEL_FN16         CEL_FN15
#    define CEL_ALLOCATOR16  CEL_ALLOCATOR15
#    define CEL_END_HEADER16 "cel/gen/dummy.h"
#elif CEL_GEN == 14
#    undef CEL_GEN
#    define CEL_GEN          15
#    define CEL_CASCADE15    15
#    define CEL_FN15         CEL_FN14
#    define CEL_ALLOCATOR15  CEL_ALLOCATOR14
#    define CEL_END_HEADER15 "cel/gen/dummy.h"
#elif CEL_GEN == 13
#    undef CEL_GEN
#    define CEL_GEN          14
#    define CEL_CASCADE14    14
#    define CEL_FN14         CEL_FN13
#    define CEL_ALLOCATOR14  CEL_ALLOCATOR13
#    define CEL_END_HEADER14 "cel/gen/dummy.h"
#elif CEL_GEN == 12
#    undef CEL_GEN
#    define CEL_GEN          13
#    define CEL_CASCADE13    13
#    define CEL_FN13         CEL_FN12
#    define CEL_ALLOCATOR13  CEL_ALLOCATOR12
#    define CEL_END_HEADER13 "cel/gen/dummy.h"
#elif CEL_GEN == 11
#    undef CEL_GEN
#    define CEL_GEN          12
#    define CEL_CASCADE12    12
#    define CEL_FN12         CEL_FN11
#    define CEL_ALLOCATOR12  CEL_ALLOCATOR11
#    define CEL_END_HEADER12 "cel/gen/dummy.h"
#elif CEL_GEN == 10
#    undef CEL_GEN
#    define CEL_GEN          11
#    define CEL_CASCADE11    11
#    define CEL_FN11         CEL_FN10
#    define CEL_ALLOCATOR11  CEL_ALLOCATOR10
#    define CEL_END_HEADER11 "cel/gen/dummy.h"
#elif CEL_GEN == 9
#    undef CEL_GEN
#    define CEL_GEN          10
#    define CEL_CASCADE10    10
#    define CEL_FN10         CEL_FN9
#    define CEL_ALLOCATOR10  CEL_ALLOCATOR9
#    define CEL_END_HEADER10 "cel/gen/dummy.h"
#elif CEL_GEN == 8
#    undef CEL_GEN
#    define CEL_GEN         9
#    define CEL_CASCADE9    9
#    define CEL_FN9         CEL_FN8
#    define CEL_ALLOCATOR9  CEL_ALLOCATOR8
#    define CEL_END_HEADER9 "cel/gen/dummy.h"
#elif CEL_GEN == 7
#    undef CEL_GEN
#    define CEL_GEN         8
#    define CEL_CASCADE8    8
#    define CEL_FN8         CEL_FN7
#    define CEL_ALLOCATOR8  CEL_ALLOCATOR7
#    define CEL_END_HEADER8 "cel/gen/dummy.h"
#elif CEL_GEN == 6
#    undef CEL_GEN
#    define CEL_GEN         7
#    define CEL_CASCADE7    7
#    define CEL_FN7         CEL_FN6
#    define CEL_ALLOCATOR7  CEL_ALLOCATOR6
#    define CEL_END_HEADER7 "cel/gen/dummy.h"
#elif CEL_GEN == 5
#    undef CEL_GEN
#    define CEL_GEN         6
#    define CEL_CASCADE6    6
#    define CEL_FN6         CEL_FN5
#    define CEL_ALLOCATOR6  CEL_ALLOCATOR5
#    define CEL_END_HEADER6 "cel/gen/dummy.h"
#elif CEL_GEN == 4
#    undef CEL_GEN
#    define CEL_GEN         5
#    define CEL_CASCADE5    5
#    define CEL_FN5         CEL_FN4
#    define CEL_ALLOCATOR5  CEL_ALLOCATOR4
#    define CEL_END_HEADER5 "cel/gen/dummy.h"
#elif CEL_GEN == 3
#    undef CEL_GEN
#    define CEL_GEN         4
#    define CEL_CASCADE4    4
#    define CEL_FN4         CEL_FN3
#    define CEL_ALLOCATOR4  CEL_ALLOCATOR3
#    define CEL_END_HEADER4 "cel/gen/dummy.h"
#elif CEL_GEN == 2
#    undef CEL_GEN
#    define CEL_GEN         3
#    define CEL_CASCADE3    3
#    define CEL_FN3         CEL_FN2
#    define CEL_ALLOCATOR3  CEL_ALLOCATOR2
#    define CEL_END_HEADER3 "cel/gen/dummy.h"
#elif CEL_GEN == 1
#    undef CEL_GEN
#    define CEL_GEN         2
#    define CEL_CASCADE2    2
#    define CEL_FN2         CEL_FN1
#    define CEL_ALLOCATOR2  CEL_ALLOCATOR1
#    define CEL_END_HEADER2 "cel/gen/dummy.h"
#else
#    undef CEL_GEN
#    define CEL_GEN         1
#    define CEL_CASCADE1    1
#    define CEL_FN1         CEL_FN0
#    define CEL_ALLOCATOR1  CEL_ALLOCATOR0
#    define CEL_END_HEADER1 "cel/gen/dummy.h"
#endif
