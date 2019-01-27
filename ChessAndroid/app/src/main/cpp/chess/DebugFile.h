#pragma once

#ifdef CR_PLATFORM_WINDOWS
#include <fstream>
#include <memory>

template<typename... Args>
void writeTime(Args&&... args)
{
	const static auto deleter = [](std::ofstream *file) {
		*file << "END OF GAME" << std::endl; // a flush is required
	};
	static auto file = std::unique_ptr<std::ofstream, decltype(deleter)>(new std::ofstream("C:/Users/razva/Desktop/Chess/output/time.txt"), deleter);

	((*file << std::forward<Args>(args)), ...);
	*file << std::endl;
}
#endif
