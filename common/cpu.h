#ifndef CPU_H
#define CPU_H

#if defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7EM__)
#define ARM7M 1
#include "arm7m.h"

#elif defined(__ARM_ARCH_7A__) || defined(__ARM_ARCH_7R__)
#define ARM7AR 1
#include "armv7-ar.h"

#endif

#endif // CPU_H
