#ifndef CEL_GEN_DEF_H
#define CEL_GEN_DEF_H

/* Codegen Definitions */

#include "cel/core/inline.h"
#include "cel/core/not_implemented.h"
#include "cel/pp/cat2.h"
#include "cel/pp/eval0.h"

#define CEL_GEN     0
#define CEL_GEN_MAX 16

#define CEL_FN         CEL_PP_EVAL0(CEL_FN__ACCESS CEL_FN__TABLE)
#define CEL_IMPLEMENT  (CEL_FN__KIND & CEL_FN__IMPLEMENT)
#define CEL_PRIVATE    (CEL_FN__KIND & CEL_FN__PRIVATE)
#define CEL_CASCADE    CEL_PP_CAT2(CEL_CASCADE, CEL_GEN)
#define CEL_ALLOCATOR  CEL_PP_CAT2(CEL_ALLOCATOR, CEL_GEN)
#define CEL_END_HEADER CEL_PP_CAT2(CEL_END_HEADER, CEL_GEN)

#define CEL_FN0         0
#define CEL_CASCADE0    0
#define CEL_ALLOCATOR0  CEL_NOT_IMPLEMENTED("No allocators defined")
#define CEL_END_HEADER0 "cel/gen/dummy.h"

#define CEL_FN__TABLE                                       \
    (,                  /* DECLARE   | PUBLIC  | DEFAULT */ \
     ,                  /* IMPLEMENT | PUBLIC  | DEFAULT */ \
     static,            /* DECLARE   | PRIVATE | DEFAULT */ \
     static,            /* IMPLEMENT | PRIVATE | DEFAULT */ \
     CEL_INLINE,        /* DECLARE   | PUBLIC  | INLINE  */ \
     ,                  /* IMPLEMENT | PUBLIC  | INLINE  */ \
     static CEL_INLINE, /* DECLARE   | PRIVATE | INLINE  */ \
     static CEL_INLINE, /* IMPLEMENT | PRIVATE | INLINE  */ \
    )

#define CEL_FN__KIND      CEL_PP_CAT2(CEL_FN, CEL_GEN)
#define CEL_FN__DECLARE   0
#define CEL_FN__IMPLEMENT 1
#define CEL_FN__PRIVATE   2
#define CEL_FN__INLINE    4

#define CEL_FN__ACCESS CEL_PP_CAT2(CEL_FN__ACCESS, CEL_FN__KIND)

#define CEL_FN__ACCESS0(_0, _1, _2, _3, _4, _5, _6, _7) _0
#define CEL_FN__ACCESS1(_0, _1, _2, _3, _4, _5, _6, _7) _1
#define CEL_FN__ACCESS2(_0, _1, _2, _3, _4, _5, _6, _7) _2
#define CEL_FN__ACCESS3(_0, _1, _2, _3, _4, _5, _6, _7) _3
#define CEL_FN__ACCESS4(_0, _1, _2, _3, _4, _5, _6, _7) _4
#define CEL_FN__ACCESS5(_0, _1, _2, _3, _4, _5, _6, _7) _5
#define CEL_FN__ACCESS6(_0, _1, _2, _3, _4, _5, _6, _7) _6
#define CEL_FN__ACCESS7(_0, _1, _2, _3, _4, _5, _6, _7) _7

#endif
