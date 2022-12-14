/**
 * Copyright (C) 2022 Heckerpowered Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 * and associated documentation files (the ¡°Software¡±), to deal in the Software without
 * restriction, including without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED ¡°AS IS¡±, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "Core/Core.h"
#include "exception"

#pragma warning(disable: 4595)
#pragma warning(disable: 28301)
#pragma warning(disable: 28253)

_IRQL_requires_max_(DISPATCH_LEVEL)
inline void __cdecl operator delete(_Pre_notnull_ __drv_freesMem(Mem) void* p) noexcept {
	if (p != nullptr) [[unlikely]] {
		ExFreePool(p);
	}
}

_IRQL_requires_max_(DISPATCH_LEVEL)
inline void __cdecl operator delete(_Pre_notnull_ __drv_freesMem(Mem) void* p, _In_ SIZE_T) noexcept
{
	if (p != nullptr) [[unlikely]] {
		ExFreePool(p);
	}
}

[[noreturn]] _declspec(noreturn) static void KernelRaiseException(unsigned long const BugCheckCode) noexcept
{
#pragma warning(disable : __WARNING_USE_OTHER_FUNCTION)
	KeBugCheck(BugCheckCode);
#pragma warning(default : __WARNING_USE_OTHER_FUNCTION)
}

[[nodiscard]]
_Ret_notnull_
_Post_writable_byte_size_(Size)
__drv_allocatesMem(Mem)
_Ret_notnull_
_IRQL_requires_max_(APC_LEVEL)
__declspec(allocator)
inline void* __cdecl operator new(SIZE_T Size)
{
	if (Size == 0) [[unlikely]]
	{
		Size = 1;
	}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmultichar"
	PVOID const p = ExAllocatePool2(POOL_FLAG_NON_PAGED, Size, 'STD');
#pragma clang diagnostic pop

	if (p == nullptr) [[unlikely]] {
		KernelRaiseException(MUST_SUCCEED_POOL_EMPTY);
	}
		
	return p;
}

#pragma warning(default: 4595)
#pragma warning(default: 28301)
#pragma warning(default: 28253)

inline __declspec(noreturn) void __cdecl _invalid_parameter_noinfo_noreturn()
{
	KernelRaiseException(KMODE_EXCEPTION_NOT_HANDLED);
}

__declspec(noreturn) void __cdecl _invoke_watson(_In_opt_z_ wchar_t const* const expression [[maybe_unused]],
	_In_opt_z_ wchar_t const* const function_name [[maybe_unused]], _In_opt_z_ wchar_t const* const file_name [[maybe_unused]],
	_In_ unsigned int const line_number [[maybe_unused]], _In_ uintptr_t const reserved [[maybe_unused]] )
{
	KernelRaiseException(KMODE_EXCEPTION_NOT_HANDLED);
}

namespace std
{
	[[noreturn]] __declspec(noreturn) inline void __cdecl _Xbad_function_call() noexcept
	{
		KernelRaiseException(KMODE_EXCEPTION_NOT_HANDLED);
	}

	[[noreturn]] __declspec(noreturn) inline void __cdecl _Xbad_alloc() noexcept
	{
		KernelRaiseException(KMODE_EXCEPTION_NOT_HANDLED);
	}

	[[noreturn]] __declspec(noreturn) inline void __cdecl _Xinvalid_argument(_In_z_ char const*) noexcept
	{
		KernelRaiseException(KMODE_EXCEPTION_NOT_HANDLED);
	}

	[[noreturn]] __declspec(noreturn) inline void __cdecl _Xlength_error(_In_z_ char const*) noexcept
	{
		KernelRaiseException(KMODE_EXCEPTION_NOT_HANDLED);
	}

	[[noreturn]] __declspec(noreturn) inline void __cdecl _Xout_of_range(_In_z_ char const*) noexcept
	{
		KernelRaiseException(KMODE_EXCEPTION_NOT_HANDLED);
	}

	[[noreturn]] __declspec(noreturn) inline void __cdecl _Xoverflow_error(_In_z_ char const*) noexcept
	{
		KernelRaiseException(KMODE_EXCEPTION_NOT_HANDLED);
	}

	[[noreturn]] __declspec(noreturn) inline void __cdecl _Xruntime_error(_In_z_ char const*) noexcept
	{
		KernelRaiseException(KMODE_EXCEPTION_NOT_HANDLED);
	}

	void(__cdecl* std::_Raise_handler)(class stdext::exception const&);
}