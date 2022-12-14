#include <Windows.h>
#include <iostream>
#include <random>
#include <optional>

#include "../Steroids Core/Steroids Core.h"

Steroids SteroidsInstance;

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

	return SteroidsInstance.ReadProcessMemory(GetCurrentProcessId(), &Value, &ReadValue, sizeof(Value), nullptr) && (Value == ReadValue);
}

bool ProtectProcessFunction() noexcept {
	return SteroidsInstance.ProtectProcess(GetCurrentProcessId()) && !TerminateProcess(OpenProcess(PROCESS_ALL_ACCESS, false, GetCurrentProcessId()), EXIT_SUCCESS);
}

bool TerminateProcessFunction() noexcept {
	STARTUPINFO StartupInfo{};
	PROCESS_INFORMATION ProcessInformation{};
	if (!CreateProcessA("cmd.exe", nullptr, nullptr, nullptr, false, 0, nullptr, nullptr, &StartupInfo, &ProcessInformation)) [[unlikely]] {
		return false;
	}

	CloseHandle(ProcessInformation.hThread);
	CloseHandle(ProcessInformation.hProcess);

	return SteroidsInstance.TerminateProcess(ProcessInformation.dwProcessId);
}

class Type {
public:
	void Function() {

	}
};

int main() {
	std::cout.sync_with_stdio(false);

	std::cout << "Steroids Unit Test 1.0.0.0" << std::endl;
	ReportFunction("Initialize Steroids", SteroidsInstance.Initialize());
	ReportFunction("Is Steroids Available", SteroidsInstance.IsAvailable());
	ReportFunction("Read Process Memory", ReadProcessMemoryFunction());
	ReportFunction("Protect Process", ProtectProcessFunction());
	ReportFunction("TerminateProcess", TerminateProcessFunction());
	ReportFunction("Finalize Steroids", SteroidsInstance.Stop());

	static_cast<void>(getchar());
}