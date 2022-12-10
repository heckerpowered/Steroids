/**
 * Copyright (C) 2022 Heckerpowered Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 * and associated documentation files (the ��Software��), to deal in the Software without
 * restriction, including without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED ��AS IS��, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include <Windows.h>
#include <string>
#include <format>
#include <sstream>

#include "Steroids Core.hpp"

_Struct_size_bytes_(sizeof(ReadProcessMemoryFunction))
struct ReadProcessMemoryFunction {
	/** An id to the process with the memory that is being read */
	SProcessId ProcessId;

	/** A pointer to the base address in the specified process from which to read.
		Before any data transfer occurs, the system verifies that all data in the
		base address and memory of the specified size is accessible for read access,
		and if it is not accessible the function fails */
	PVOID const BaseAddress;

	/** A pointer to a buffer that receives the contents from the address space of the specified process */
	PVOID Buffer;

	/** The number of bytes to be read from the specified process */
	SIZE_T Size;

	/** A pointer to a variable that receives the number of bytes transferred into the specified buffer,
		If NumberOfBytesRead is null, the parameter is ignored */
	SIZE_T* NumberOfBytesRead;
};

HANDLE SteroidsHandle = INVALID_HANDLE_VALUE;

extern "C" EXPORT bool InitializeSteroids() noexcept {
	if (SteroidsHandle != INVALID_HANDLE_VALUE) {
		return true;
	}
	
	SteroidsHandle = CreateFileA("\\\\.\\Steroids", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (SteroidsHandle == INVALID_HANDLE_VALUE) [[unlikely]] {
		auto const serviceManager = OpenSCManagerA(nullptr, nullptr, SC_MANAGER_ALL_ACCESS);
		if (serviceManager == nullptr) [[unlikely]] {
			return false;
		}

		constexpr auto serviceName = "Steroids";
		auto service = OpenServiceA(serviceManager, serviceName, SERVICE_ALL_ACCESS);
		if (service == nullptr) [[unlikely]] {
			std::string buffer;
			buffer.resize(GetCurrentDirectoryA(0, nullptr));
			buffer.resize(GetCurrentDirectoryA(static_cast<DWORD>(buffer.size()), buffer.data()));

			std::stringstream stream;
			stream.str().resize(buffer.size() + 13);
			stream << buffer << '\\' << "Steroids.sys";

			service = CreateServiceA(serviceManager, serviceName, serviceName, SERVICE_ALL_ACCESS, SERVICE_KERNEL_DRIVER, SERVICE_DEMAND_START,
				SERVICE_ERROR_IGNORE, stream.str().data(), nullptr, nullptr, nullptr, nullptr, nullptr);
			if (service == nullptr)
			{
				CloseServiceHandle(serviceManager);
				return false;
			}
		}

		auto const status = StartServiceA(service, 0, 0);
		SteroidsHandle = CreateFileA("\\\\.\\Steroids", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
		return status && (SteroidsHandle != INVALID_HANDLE_VALUE);
	}

	return true;
}

extern "C" EXPORT bool FinalizeSteroids() noexcept {
	CloseHandle(SteroidsHandle);

	SC_HANDLE const ServiceManager = OpenSCManagerA(nullptr, nullptr, SC_MANAGER_ALL_ACCESS);
	if (ServiceManager == nullptr) [[unlikely]] {
		return false;
	}

	SC_HANDLE const ServiceHandle = OpenServiceA(ServiceManager, "Steroids", SERVICE_ALL_ACCESS);

	[[maybe_unused]] SERVICE_STATUS ServiceStatus;

	return ControlService(ServiceHandle, SERVICE_CONTROL_STOP, &ServiceStatus) &&
		DeleteService(ServiceHandle) &&
		CloseServiceHandle(ServiceHandle) &&
		CloseServiceHandle(ServiceManager);
}

extern "C" EXPORT bool SReadProcessMemory(SProcessId const ProcessId, PVOID const BaseAddress, PVOID Buffer, SIZE_T Size, SIZE_T* NumberOfBytesRead) noexcept
{
	ReadProcessMemoryFunction Function{ .ProcessId = ProcessId, .BaseAddress = BaseAddress, .Buffer = Buffer, .Size = Size, .NumberOfBytesRead = NumberOfBytesRead };
	return DeviceIoControl(SteroidsHandle, CTL_CODE(FILE_DEVICE_UNKNOWN, 2, METHOD_IN_DIRECT, FILE_ANY_ACCESS), &Function, sizeof(ReadProcessMemoryFunction), nullptr, 0, nullptr, nullptr);
}

extern "C" EXPORT bool ProtectProcess(SProcessId ProcessId) noexcept
{
	return DeviceIoControl(SteroidsHandle, CTL_CODE(FILE_DEVICE_UNKNOWN, 3, METHOD_IN_DIRECT, FILE_ANY_ACCESS), &ProcessId, sizeof(ProcessId), nullptr, 0, nullptr, nullptr);;
}

extern "C" EXPORT bool IsSteroidsAvailable() noexcept {
	bool SteroidsAvailable{};
	DeviceIoControl(SteroidsHandle, CTL_CODE(FILE_DEVICE_UNKNOWN, 0, METHOD_IN_DIRECT, FILE_ANY_ACCESS), nullptr, 0, &SteroidsAvailable, sizeof(bool), nullptr, nullptr);
	return SteroidsAvailable;
}