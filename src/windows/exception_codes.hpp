#pragma once

#define NOMINMAX

#include <Windows.h>

#include <exceptionCodes.h>

// See https://learn.microsoft.com/en-us/windows-hardware/drivers/debugger/specific-exceptions

#ifndef STATUS_CPP_EH_EXCEPTION
#	define STATUS_CPP_EH_EXCEPTION 0xE06D7363
#endif

#ifndef STATUS_CLR_EXCEPTION
#	define STATUS_CLR_EXCEPTION 0xE0434f4D
#endif

#ifndef VS_SET_THREAD_NAME
#	define VS_SET_THREAD_NAME 0x406D1388
#endif
