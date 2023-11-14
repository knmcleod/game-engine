#pragma once
#include <string>

namespace GE
{
	class FileDialogs
	{
	public:
		/// <summary>
		/// Returns Empty String if Cancelled
		/// </summary>
		/// <param name="filter"></param>
		/// <returns></returns>
		static std::string LoadFile(const char* filter);
		
		/// <summary>
		/// Returns Empty String if Cancelled
		/// </summary>
		/// <param name="filter"></param>
		/// <returns></returns>
		static std::string SaveFile(const char* filter);
	};
}
