#pragma once
#include <thread>
#include <vector>
#include <queue>
#include <functional>
#include <condition_variable>
#include <mutex>
#include <iostream>

#pragma once
#include <chrono>
#include <smmintrin.h>

#ifdef _WIN32
#include <limits.h>
#include <intrin.h>
typedef unsigned __int32  uint32_t;

#else
#include <stdint.h>
#endif

#define CalculateSimdLoopLimit(TotalElements) TotalElements & ~3

class CodeTimer
{
	std::chrono::steady_clock::time_point ClockStart;
	std::chrono::steady_clock::time_point ClockStop;
	float Time = 0.0f;

	public:

	CodeTimer()
	{

	}

	void Start()
	{
		ClockStart = std::chrono::high_resolution_clock::now();
	}

	void Stop()
	{
		this->ClockStop = std::chrono::high_resolution_clock::now();
		this->Time = std::chrono::duration<float, std::milli>(ClockStop - ClockStart).count();
	}

	double StopAndPrint()
	{
		this->ClockStop = std::chrono::high_resolution_clock::now();
		this->Time = std::chrono::duration<float, std::milli>(ClockStop - ClockStart).count();
		std::cout << "Took " << this->Time << " ms";
		return Time;
	}
};

typedef struct SupportedInstructions
{
	bool SSE = false;
	bool SSE2 = false;
	bool SSE3 = false;
	bool SSSE3 = false;
	bool SSE41 = false;
	bool SSE42 = false;
	bool AVX = false;
	bool AVX2 = false;
};

class CPUID
{
	uint32_t regs[4];
	unsigned int IdValue = 0;

	public:
	explicit CPUID(unsigned i) {
		this->IdValue = i;
#ifdef _WIN32
		__cpuid((int*)regs, (int)i);

#else
		asm volatile
			("cpuid" : "=a" (regs[0]), "=b" (regs[1]), "=c" (regs[2]), "=d" (regs[3])
				: "a" (i), "c" (0));
		// ECX is set to zero for CPUID function 4
#endif
	}

	SupportedInstructions GetSupportedInstructions()
	{
		SupportedInstructions Out;

		CPUID CpuID = *this;
		if (this->IdValue != 1)
		{
			CpuID = CPUID(1);
		}

		if (CpuID.EDX() & (1 << 25)) Out.SSE = true;
		if (CpuID.EDX() & (1 << 26)) Out.SSE2 = true;
		if (CpuID.ECX() & (1 << 0)) Out.SSE3 = true;
		if (CpuID.ECX() & (1 << 9)) Out.SSSE3 = true;
		if (CpuID.ECX() & (1 << 19)) Out.SSE41 = true;
		if (CpuID.ECX() & (1 << 20)) Out.SSE42 = true;
		if (CpuID.ECX() & (1 << 28)) Out.AVX = true;

		return Out;
	}

	void PrintSupportedInstructions()
	{
		CPUID CpuID = *this;
		if (this->IdValue != 1)
		{
			CpuID = CPUID(1);
		}

		SupportedInstructions Out = CpuID.GetSupportedInstructions();

		if (Out.SSE) std::cout << "SSE supported" << std::endl;
		if (Out.SSE2) std::cout << "SSE2 supported" << std::endl;
		if (Out.SSE3) std::cout << "SSE3 supported" << std::endl;
		if (Out.SSSE3) std::cout << "SSSE3 supported" << std::endl;
		if (Out.SSE41) std::cout << "SSE4.1 supported" << std::endl;
		if (Out.SSE42) std::cout << "SSE4.2 supported" << std::endl;
		if (Out.AVX) std::cout << "AVX supported" << std::endl;
	}

	const uint32_t& EAX() const { return regs[0]; }
	const uint32_t& EBX() const { return regs[1]; }
	const uint32_t& ECX() const { return regs[2]; }
	const uint32_t& EDX() const { return regs[3]; }
};

class ThreadManager
{
	unsigned long long CoreCount;
	std::atomic<bool> JoinThreads{ false };
	std::vector<std::thread> ThreadPool;
	std::mutex QueueMutex;
	std::mutex ThreadMutex;
	std::mutex CoutStream;
	std::condition_variable TaskConditionVar;
	std::condition_variable QueueSzConditionVar;


	void WorkerLoop()
	{
		std::function<void()> Task = nullptr;

		while (true)
		{
			Task = nullptr;

			{
				std::unique_lock<std::mutex> QueueLock(this->QueueMutex);

				if (!Queue.empty())
				{
					Task = std::move(this->Queue.front());
					Queue.pop();
				}
				else
				{
					QueueLock.unlock();
					QueueSzConditionVar.notify_one();
					QueueLock.lock();
					this->TaskConditionVar.wait(QueueLock, [=]() mutable { return (!this->Queue.empty()) || this->JoinThreads; });

					if (this->JoinThreads)
					{
						return;
					}
				}
			}

			if (this->JoinThreads)
			{
				return;
			}

			if (Task)
			{
				Task();
			}
		}
	}


	public:
	std::queue<std::function<void()>> Queue;

	ThreadManager()
	{
		this->CoreCount = std::thread::hardware_concurrency() - 1;
		this->ThreadMutex.lock();
		this->ThreadPool.reserve(this->CoreCount);

		for (int i = 0; i < this->CoreCount; i++)
		{
			auto BindedThread = [=]() mutable {this->WorkerLoop(); };
			this->ThreadPool.push_back(std::thread(BindedThread));
		}
		this->ThreadMutex.unlock();
	}

	ThreadManager(size_t ThreadPoolSz)
	{
		this->CoreCount = ThreadPoolSz;
		this->ThreadMutex.lock();
		this->ThreadPool.reserve(this->CoreCount);

		for (int i = 0; i < this->CoreCount; i++)
		{
			auto BindedThread = [=]() mutable {this->WorkerLoop(); };
			this->ThreadPool.push_back(std::thread(BindedThread));
		}
		this->ThreadMutex.unlock();
	}

	static const int GetCpuCores()
	{
		return std::thread::hardware_concurrency();
	}

	template<typename FunctionToCall, typename... Arguments>
	void EnqueueTask(FunctionToCall&& Func, Arguments&&... Args)
	{
		auto BindedFunc = [=]() mutable
		{
			Func(std::forward<Arguments>(Args)...);
		};

		this->QueueMutex.lock();
		this->Queue.emplace(BindedFunc);
		this->QueueMutex.unlock();

		this->TaskConditionVar.notify_one();
	}

	void WaitUntilAllTasksFinished()
	{
		size_t CurrentTasks = 0;
		do
		{
			std::unique_lock<std::mutex> QueueLock(this->QueueMutex);
			QueueSzConditionVar.wait(QueueLock, [=]() { return Queue.empty(); });

			CurrentTasks = Queue.size();
		} while (CurrentTasks != 0);
	}

	~ThreadManager()
	{
		this->JoinThreads = true;
		this->TaskConditionVar.notify_all();

		this->ThreadMutex.lock();
		for (int i = 0; i < this->ThreadPool.size(); i++)
		{
			if (this->ThreadPool.at(i).joinable())
			{
				this->ThreadPool.at(i).join();
			}
		}
		this->ThreadMutex.unlock();
	}
};
