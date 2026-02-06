#ifndef CEL_CORE_INLINE_H
#define CEL_CORE_INLINE_H

/* Inline Keyword */

#include "cel/core/stdc.h"

#if CEL_STDC >= CEL_STDC99
#    define CEL_INLINE inline
#else
#    define CEL_INLINE
#endif

#endif
