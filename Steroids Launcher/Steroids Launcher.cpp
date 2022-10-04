#include <Windows.h>
#include "../Steroids Core/Steroids Core.hpp"

int main() {
	if (!RequestSteroids()) {
		auto lastError = GetLastError();
		char* buffer{};
		FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, lastError,
								 MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<char*>(&buffer), 0, nullptr);
		MessageBoxA(nullptr, buffer, nullptr, MB_ICONERROR);
		return lastError;
	}

	return EXIT_SUCCESS;
}