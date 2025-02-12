#pragma once


namespace GE
{
	class Thread
	{
	public:
		bool Add(const std::function<void()>& func);
		void Execute();
	private:
		std::vector<std::function<void()>> p_Thread;
		std::mutex p_ThreadMutex;
	};
}