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
#include <Windows.h>
#include <string>
#include <format>
#include <sstream>

#include "Steroids Core.h"
#include "SteroidsFunctionCode.h"
#include "SteroidsFunction.h"

 /** Called when the Steroids is constructing */
Steroids::Steroids() noexcept : SteroidsHandle(INVALID_HANDLE_VALUE) {
	// Do nothing
}

/** Called when the Steroids is deconstructing */
Steroids::~Steroids() noexcept {
	// Try to close Steroids's file handle
	CloseHandle(SteroidsHandle);
}

bool Steroids::Initialize() noexcept {
	if (SteroidsHandle != INVALID_HANDLE_VALUE) {
		return true;
	}

	SteroidsHandle = CreateFileA("\\\\.\\Steroids", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (SteroidsHandle == INVALID_HANDLE_VALUE) [[unlikely]] {
		SC_HANDLE const ServiceManager = OpenSCManagerA(nullptr, nullptr, SC_MANAGER_ALL_ACCESS);
		if (ServiceManager == nullptr) [[unlikely]] {
			return false;
		}

		constexpr auto ServiceName = "Steroids";
		SC_HANDLE Service = OpenServiceA(ServiceManager, ServiceName, SERVICE_ALL_ACCESS);
		if (Service == nullptr) [[unlikely]] {
			std::string Buffer;
			Buffer.resize(GetCurrentDirectoryA(0, nullptr));
			Buffer.resize(GetCurrentDirectoryA(static_cast<DWORD>(Buffer.size()), Buffer.data()));

			std::stringstream Stream;
			Stream.str().resize(Buffer.size() + 13);
			Stream << Buffer << '\\' << "Steroids.sys";

			Service = CreateServiceA(ServiceManager, ServiceName, ServiceName, SERVICE_ALL_ACCESS, SERVICE_KERNEL_DRIVER, SERVICE_DEMAND_START,
				SERVICE_ERROR_IGNORE, Stream.str().data(), nullptr, nullptr, nullptr, nullptr, nullptr);
			if (Service == nullptr) [[unlikely]] {
				CloseServiceHandle(ServiceManager);
				return false;
			}
		}

		bool const Status = StartServiceA(Service, 0, 0);
		SteroidsHandle = CreateFileA("\\\\.\\Steroids", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL, nullptr);
		return Status && (SteroidsHandle != INVALID_HANDLE_VALUE);
	}

	return true;
}

bool Steroids::Stop() noexcept {
	// Try to close Steroids' file handle
	if (!CloseHandle(SteroidsHandle)) [[unlikely]] /* Closing Steroids's file handle will not fail under normal circumstances */ {
		return false;
	} 

	// Invalidate 
	SteroidsHandle = INVALID_HANDLE_VALUE;

	SC_HANDLE const ServiceManager = OpenSCManagerA(nullptr, nullptr, SC_MANAGER_ALL_ACCESS);
	if (ServiceManager == nullptr) [[unlikely]] {
		return false;
	}

	SC_HANDLE const ServiceHandle = OpenServiceA(ServiceManager, "Steroids", SERVICE_ALL_ACCESS);

	[[maybe_unused]] SERVICE_STATUS ServiceStatus;

	return ControlService(ServiceHandle, SERVICE_CONTROL_STOP, &ServiceStatus) && DeleteService(ServiceHandle) 
		&& CloseServiceHandle(ServiceHandle) && CloseServiceHandle(ServiceManager);
}

bool Steroids::IsAvailable() noexcept {
	bool SteroidsAvailable{};
	DeviceIoControl(SteroidsHandle, CTL_CODE(FILE_DEVICE_UNKNOWN, static_cast<int>(SteroidsFunction::SteroidsAvailable), METHOD_IN_DIRECT, FILE_ANY_ACCESS),
		nullptr, 0, &SteroidsAvailable, sizeof(bool), nullptr, nullptr);
	return SteroidsAvailable;
}

bool Steroids::ReadProcessMemory(ProcessIDType const ProcessID, PVOID const BaseAddress, PVOID Buffer, SIZE_T Size, SIZE_T* NumberOfBytesRead) noexcept {
	ReadProcessMemoryFunction Function{ .ProcessID = ProcessID, .BaseAddress = BaseAddress, .Buffer = Buffer, .Size = Size, .NumberOfBytesRead = NumberOfBytesRead };
	return DeviceIoControl(SteroidsHandle, CTL_CODE(FILE_DEVICE_UNKNOWN, static_cast<int>(SteroidsFunction::ReadProcessMemory), METHOD_IN_DIRECT, FILE_ANY_ACCESS),
		&Function, sizeof(ReadProcessMemoryFunction), nullptr, 0, nullptr, nullptr);
}

bool Steroids::ProtectProcess(ProcessIDType const ProcessID) noexcept {
	return DeviceIoControl(SteroidsHandle, CTL_CODE(FILE_DEVICE_UNKNOWN, static_cast<int>(SteroidsFunction::ProtectProcess), METHOD_IN_DIRECT, FILE_ANY_ACCESS),
		const_cast<ProcessIDType*>(&ProcessID), sizeof(ProcessID), nullptr, 0, nullptr, nullptr);
}

bool Steroids::TerminateProcess(ProcessIDType const ProcessID) noexcept {
	return DeviceIoControl(SteroidsHandle, CTL_CODE(FILE_DEVICE_UNKNOWN, static_cast<int>(SteroidsFunction::TerminateProcess), METHOD_IN_DIRECT, FILE_ANY_ACCESS),
		const_cast<ProcessIDType*>(&ProcessID), sizeof(ProcessID), nullptr, 0, nullptr, nullptr);
}