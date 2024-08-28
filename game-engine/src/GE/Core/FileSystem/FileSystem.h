#pragma once
#include <string>

namespace GE
{
	class FileSystem
	{
	public:
		/// Returns Empty String if Cancelled
		static std::string LoadFromFileDialog(const char* filter);

		/// Returns Empty String if Cancelled
		static std::string SaveFromFileDialog(const char* filter);
	};

}
