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
#include <cstdint>
#include <string>
#include <format>
#include <sstream>

#include "Steroids Core.hpp"

HANDLE SteroidsHandle;

bool DllMain(HMODULE const module, std::uint32_t const callReason, void* reserved [[maybe_unused]] ) noexcept {
	DisableThreadLibraryCalls(module);

	return true;
}

extern "C" bool RequestSteroids() noexcept {
	if (SteroidsHandle != INVALID_HANDLE_VALUE) {
		return true;
	}

	SteroidsHandle = CreateFileA("\\\\.\\Steroids", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (SteroidsHandle == INVALID_HANDLE_VALUE) {
		auto const serviceManager = OpenSCManagerA(nullptr, nullptr, SC_MANAGER_ALL_ACCESS);
		if (serviceManager == nullptr) [[unlikely]] {
			return false; 
		}

		constexpr auto serviceName = "Steroids";
		auto service = OpenServiceA(serviceManager, serviceName,SERVICE_ALL_ACCESS);
		if (service == nullptr) [[unlikely]] {
			std::string buffer;
			buffer.resize(GetCurrentDirectoryA(0, nullptr));
			buffer.resize(GetCurrentDirectoryA(buffer.size(), buffer.data()));
			
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
		return status;
	}

	return true;
}