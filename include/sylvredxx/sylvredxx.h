
#define SYLVREDXX_VER_MAJOR                                 0
#define SYLVREDXX_VER_MINOR                                 0
#define SYLVREDXX_VER_PATCH                                 0
#define SYLVREDXX_VER_ALPHABETA                             0


#define SYLVREDXX_VER \
    (0\
        |   (   SYLVREDXX_VER_MAJOR       << 24   ) \
        |   (   SYLVREDXX_VER_MINOR       << 16   ) \
        |   (   SYLVREDXX_VER_PATCH       <<  8   ) \
        |   (   SYLVREDXX_VER_ALPHABETA   <<  0   ) \
    )


#include <stdint.h>


/** Obtains the value of SYLVREDXX_VER at the time of compilation of the
 * library.
 */
uint32_t
sylvredxx_api_version(void);


#ifdef __cplusplus

namespace sylvredxx {

/** Obtains the value of SYLVREDXX_VER at the time of compilation of the
 * library.
 */
inline
uint32_t
api_version()
{
    return sylvredxx_api_version();
}

} /* namespace sylvredxx */
#endif /* __cplusplus */



#pragma once
