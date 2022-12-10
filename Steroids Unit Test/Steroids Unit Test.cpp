﻿#include <Windows.h>
#include <iostream>
#include <random>
#include <optional>

#include "../Steroids Core/Steroids Core.hpp"

void ReportFunction(char const* FunctionName, bool const Success) noexcept {
	if (Success) [[likely]] {
		SetLastError(0);
	}

	char* ErrorMessage{};
	FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<char*>(&ErrorMessage), 0, nullptr);

	if (Success) [[likely]] {
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN);
		std::cout << "[Success] " << FunctionName << " - " << ErrorMessage;
	}
	else {
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED);
		std::cout << "[Fail] " << FunctionName << " - " << ErrorMessage;
	}

	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 0);
	LocalFree(ErrorMessage);
}

bool ReadProcessMemoryFunction() noexcept {
	std::uniform_int_distribution Distribution(std::numeric_limits<int>::min(), std::numeric_limits<int>::max());
	std::default_random_engine Engine;

	int Value = Distribution(Engine);
	int ReadValue{};

	return SReadProcessMemory(GetCurrentProcessId(), &Value, &ReadValue, sizeof(Value), nullptr) && (Value == ReadValue);
}

bool ProtectProcessFunction() noexcept {
	return ProtectProcess(GetCurrentProcessId()) && !TerminateProcess(OpenProcess(PROCESS_ALL_ACCESS, false, GetCurrentProcessId()), EXIT_SUCCESS);
}

class Type {
public:
	void Function() {

	}
};

int main() {
	std::cout.sync_with_stdio(false);

	std::cout << "Steroids Unit Test 1.0.0.0" << std::endl;
	ReportFunction("Initialize Steroids", InitializeSteroids());
	ReportFunction("Is Steroids Available", IsSteroidsAvailable());
	ReportFunction("Read Process Memory", ReadProcessMemoryFunction());
	ReportFunction("Protect Process", ProtectProcessFunction());
	ReportFunction("Finalize Steroids", FinalizeSteroids());

	sizeof(std::optional<bool>);

	static_cast<void>(getchar());
}