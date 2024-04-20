#ifndef __COMMON_H
#define __COMMON_H

#include <cstddef>

#ifdef __cplusplus
extern "C" {
#endif
#include "linux_header/include/linux/akvm.h"

#define interface struct

#ifdef __cplusplus
};
#endif

typedef unsigned long gpa;
typedef unsigned long hva;
typedef __u64 u64;

#endif
