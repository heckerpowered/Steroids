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
#pragma once

#include "Core/Core.h"
#include <optional>

template<typename T>
struct Result {

private:
	struct NontrivialDummyType {
		constexpr NontrivialDummyType() noexcept {
			// This default constructor is user-provided to avoid zero-initialization when objects are value-initialized
		}
	};

	union {
		T Value;
		NontrivialDummyType Dummy;
	};

	NTSTATUS Status;

public:

	constexpr Result(NTSTATUS const Status) noexcept : Status(Status), Dummy() {}
	constexpr Result(T&& Value) noexcept : Status(STATUS_SUCCESS), Value(std::forward<T>(Value)) {}
	constexpr ~Result() noexcept {}

	constexpr T& GetValue() noexcept {
		return Value;
	}

	constexpr NTSTATUS GetStatus() const noexcept {
		return Status;
	}

	/* Returns a bool that indicates if the action is performed successfully */
	constexpr bool IsSuccess() const noexcept {
		return NT_SUCCESS(GetStatus());
	}

	constexpr operator NTSTATUS() const {
		return GetStatus();
	}

	constexpr operator T() noexcept {
		return GetValue();
	}

	constexpr T* operator->() noexcept {
		return &GetValue();
	}

	constexpr T& operator*() noexcept {
		return GetValue();
	}

	constexpr operator bool() const noexcept {
		return NT_SUCCESS(GetStatus());
	}

	template<typename T>
	constexpr NTSTATUS IfSuccess(T&& Function) noexcept {
		if (IsSuccess()) {
			std::invoke(Function, GetValue());
		}

		return GetStatus();
	}
};

template<bool Const, typename Class, typename Function>
struct MemberFunctionReference;

template<typename Class, typename RetType, typename... ArgTypes>
struct MemberFunctionReference<false, Class, RetType(ArgTypes...)> {
	using Type = RetType(Class::*&)(ArgTypes...);
};

template<typename Class, typename RetType, typename... ArgTypes>
struct MemberFunctionReference<true, Class, RetType(ArgTypes...)> {
	using Type = RetType(Class::*&)(ArgTypes...) const;
};