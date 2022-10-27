#pragma once

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"

#include <ntifs.h>
#include <ntddk.h>
#include <intrin.h>

#pragma clang diagnostic pop

#ifdef DBG
#define SE_VERIFY()
#endif // DBG
