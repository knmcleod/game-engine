#pragma once
#include <string>

namespace GE
{
	class FileDialogs
	{
	public:
		/// Returns Empty String if Cancelled
		static std::string LoadFile(const char* filter);
		

		/// Returns Empty String if Cancelled
		static std::string SaveFile(const char* filter);
	};
}
