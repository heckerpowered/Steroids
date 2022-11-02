#include <Windows.h>
#include <iostream>
#include "../Steroids Core/Steroids Core.hpp"

void ReportFunction(char const* FunctionName, bool const Success) noexcept {
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

int main() {
	std::cout.sync_with_stdio(false);

	std::cout << "Steroids Unit Test 1.0.0.0" << std::endl;
	ReportFunction("Initialize Steroids", InitializeSteroids());

	static_cast<void>(getchar());
}