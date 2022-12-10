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

template<typename T>
class AVLTree {

private:
	RTL_AVL_TABLE Table;

public:
	[[nodiscard]] AVLTree() noexcept {
		RtlInitializeGenericTableAvl(&Table, &AVLTree<T>::CompareRoutine, &AVLTree<T>::AllocateRoutine, &AVLTree<T>::FreeRoutine, nullptr);
	}

private:
	RTL_GENERIC_COMPARE_RESULTS CompareRoutine(
		struct _RTL_AVL_TABLE* Table [[maybe_unused]], /** A pointer to the avl table */
		PVOID FirstStruct, /** A pointer to the first item to be compared */
		PVOID SecondStruct /** A pointer to the second item to be compared */
	) noexcept {
		// Cast pointer to integer to compare
		T const& FirstValue = *static_cast<T*>(FirstStruct);
		T const& SecondValue = *static_cast<T*>(SecondStruct);

		// Compare FirstValue and SecondValue
		if (FirstValue == SecondValue) {
			return RTL_GENERIC_COMPARE_RESULTS::GenericEqual; // First value equals to second value
		}
		else if (FirstValue < SecondValue) {
			return RTL_GENERIC_COMPARE_RESULTS::GenericLessThan; // First value less than second value
		}
		else {
			return RTL_GENERIC_COMPARE_RESULTS::GenericGreaterThan; // First value greater than second value
		}
	}

	PVOID AllocateRoutine(
		struct _RTL_AVL_TABLE* Table [[maybe_unused]], /** A pointer to the generic table */
		CLONG ByteSize /** The number of bytes to allocate */
	) noexcept {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmultichar"
		return ExAllocatePool2(POOL_FLAG_NON_PAGED, ByteSize, 'AVL');
#pragma clang diagnostic pop
	}

	void FreeRoutine(
		struct _RTL_AVL_TABLE* Table [[maybe_unused]], /** A pointer to the generic table */
		PVOID Buffer /** A pointer to the element that is being deleted */
	) noexcept {
		// Free memory
		ExFreePool(Buffer);
	}
};