

#ifndef __JINBO_ERROR_H__
#define __JINBO_ERROR_H__

/**
 * ISO C standard errors
 */
#include <errno.h>
#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif

typedef int Error_t;

#define ERRORNO(no)        (100000 + (no))
#define ERROR_OFFSET          (100)


#define ERRORSTART           ERRORNO(0)     /* lynxi error code start */
#define ERRORALIGN           ERRORNO(1)     /* buffer width/height alignment error */
#define ERRORFORMAT          ERRORNO(2)     /* format not support */
#define ERROREOF             ERRORNO(3)     /* end of file */
#define ERRORPTR             ERRORNO(4)     /* invalid ptr */
#define ERRORPARAM           ERRORNO(5)     /* invalid argument */
#define ERRORMALLOC          ERRORNO(7)     /* new memory failed */

////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////split line of commom and module-related error code/////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
/*utils error number*/
#define ERROR_UTILS_START       (ERRORNO(0) + ERROR_OFFSET)
#define THREADPOOLFAILED        (ERRORNO(0) + ERROR_UTILS_START)     /*100100 thread pool open error */


#define CHECK_ERROR(cond)                                                                                              \
    do {                                                                                                               \
        int error = cond;                                                                                              \
        if (unlikely(error)) {                                                                                         \
            loge("%s failed!, error code = %d", #cond, error);                                                         \
            return error;                                                                                              \
        }                                                                                                              \
    } while (0);

#ifdef __cplusplus
}
#endif

#endif //__JINBO_ERROR_H__
