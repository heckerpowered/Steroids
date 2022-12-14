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

#include "Steroids Core.h"
#include "sal.h"

_Struct_size_bytes_(sizeof(ReadProcessMemoryFunction))
struct ReadProcessMemoryFunction {
	/** An id to the process with the memory that is being read */
	Steroids::ProcessIDType ProcessID;

	/** A pointer to the base address in the specified process from which to read.
		Before any data transfer occurs, the system verifies that all data in the
		base address and memory of the specified size is accessible for read access,
		and if it is not accessible the function fails */
	Steroids::PointerType const BaseAddress;

	/** A pointer to a buffer that receives the contents from the address space of the specified process */
	Steroids::PointerType Buffer;

	/** The number of bytes to be read from the specified process */
	Steroids::SizeType Size;

	/** A pointer to a variable that receives the number of bytes transferred into the specified buffer,
		If NumberOfBytesRead is null, the parameter is ignored */
	Steroids::SizeType* NumberOfBytesRead;
};