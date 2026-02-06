#ifndef CEL_CORE_STDC_H
#define CEL_CORE_STDC_H

/* C Standard Versioning */

#define CEL_STDC23 202311L
#define CEL_STDC11 201112L
#define CEL_STDC99 199901L
#define CEL_STDC89 0L

#ifdef __STDC_VERSION__
#    define CEL_STDC __STDC_VERSION__
#else
#    define CEL_STDC CEL_STDC89
#endif

#endif
