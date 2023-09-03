#pragma once

namespace GE
{
	class Context
	{
	public:
		//Initializes Context
		virtual void Init() = 0;

		// Swaps Buffers
		virtual void SwapBuffers() = 0;
	};
}