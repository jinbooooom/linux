

#ifndef __JINBO_LIKELY_H__
#define __JINBO_LIKELY_H__

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

#endif //__JINBO_LIKELY_H__
