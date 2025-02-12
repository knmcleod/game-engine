#include "GE/GEpch.h"

#include "Thread.h"

namespace GE
{
	bool Thread::Add(const std::function<void()>& func)
	{
		// Lock for this scope, won't lock again till unlocked
		std::scoped_lock<std::mutex> lock(p_ThreadMutex);

		p_Thread.emplace_back(func);
		return true;
	}

	void Thread::Execute()
	{
		std::vector<std::function<void()>> copy;
		{
			// Lock for this scope, won't lock again till unlocked(finished)
			std::scoped_lock<std::mutex> lock(p_ThreadMutex);
			copy = p_Thread;
			p_Thread.clear();
			p_Thread = std::vector<std::function<void()>>();
		}

		for (auto& func : copy)
			func();

		copy.clear();
		copy = std::vector<std::function<void()>>();
	}
}