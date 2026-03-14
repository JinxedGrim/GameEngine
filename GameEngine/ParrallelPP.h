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
#include <windows.h>
typedef unsigned __int32  uint32_t;

#else
#include <stdint.h>
#endif

#define CalculateSimdLoopLimit(TotalElements) TotalElements & ~3

class CodeTimer
{
	LARGE_INTEGER start;
public:
	inline void Start()
	{
		QueryPerformanceCounter(&start);
	}

	inline float Stop()
	{
		LARGE_INTEGER end;
		QueryPerformanceCounter(&end);

		static double invFreq = [] {
			LARGE_INTEGER f;
			QueryPerformanceFrequency(&f);
			return 1000.0 / (double)f.QuadPart;
		}();

		return float((end.QuadPart - start.QuadPart) * invFreq);
	}
};

struct SupportedInstructions
{
	bool SSE = false;
	bool SSE2 = false;
	bool SSE3 = false;
	bool SSSE3 = false;
	bool SSE41 = false;
	bool SSE42 = false;
	bool AVX = false;
	bool AVX2 = false;

	std::string GetSupportString() const
	{
	    std::string out = "Suupported instructions:";

		if (this->SSE) out += " [SSE] ";
		if (this->SSE2) out += " [SSE2] ";
		if (this->SSE3) out += " [SSE3] ";
		if (this->SSSE3) out += " [SSSE3] ";
		if (this->SSE41) out += " [SSE4.1] ";
		if (this->SSE42) out += " [SSE4.2] ";
		if (this->AVX) out += " [AVX] ";

		return out;
	}

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

	void GetCoreCount()
	{

	}

	static std::string GetProcessorName() 
	{
		char name[49] = { 0 }; // 48 chars + null terminator
		uint32_t* ptr = reinterpret_cast<uint32_t*>(name);

		for (unsigned i = 0; i < 3; ++i) {
			CPUID cpu(0x80000002 + i);
			ptr[i * 4 + 0] = cpu.EAX();
			ptr[i * 4 + 1] = cpu.EBX();
			ptr[i * 4 + 2] = cpu.ECX();
			ptr[i * 4 + 3] = cpu.EDX();
		}

		return std::string(name);
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

	void LogCpuInfo()
	{
		CPUID CpuID = *this;
		if (this->IdValue != 1)
		{
			CpuID = CPUID(1);
		}

		SupportedInstructions Out = CpuID.GetSupportedInstructions();
		std::cout << Out.GetSupportString() << "\n";
	}

	const uint32_t& EAX() const { return regs[0]; }
	const uint32_t& EBX() const { return regs[1]; }
	const uint32_t& ECX() const { return regs[2]; }
	const uint32_t& EDX() const { return regs[3]; }
};

namespace SIMDUtils
{
	// FOR SIMD FUNCTIONS CONVENTION IS AS FOLLOWS
	// Prefixes:
	//
	// __mm__    -- SSE Instruction
	// __mm256__ -- AVX Instruction
	// __mm512__ -- AVX Instruction
	// fm        -- FMA (fused multiply-add/subtract)
	//
	// Types:
	//
	// __m128   -- 128-bit vector of 4 floats
	// __m128d  -- 128-bit vector of 2 doubles
	// __m128i  -- 128-bit vector of integers
	// __m256   -- 256-bit vector of 8 floats
	// __m256i  -- 256-bit vector of integers
	// __m512   -- 512-bit vector of 16 floats
	//
	// Operations:
	//
	// add      -- addition
	// sub      -- subtraction
	// mul      -- multiplication
	// div      -- division
	// sqrt     -- square root
	// rsqrt    -- reciprocal square root (1/sqrt(x))
	// rcp      -- reciprocal (1/x)
	//
	// fmadd    -- fused multiply-add: a*b + c
	// fmsub    -- fused multiply-subtract: a*b - c
	//
	// and      -- bitwise AND
	// or       -- bitwise OR
	// xor      -- bitwise XOR
	// andnot   -- bitwise AND NOT (a & ~b)
	//
	// cmp      -- compare
	// cmpeq    -- compare equal
	// cmpgt    -- compare greater
	// cmplt    -- compare less
	// cmpge    -- compare greater or equal
	//
	// shuffle  -- reorder vector elements
	// permute  -- permute elements across lanes
	// blend    -- select elements from two vectors based on mask
	// alignr   -- align and shift bytes between two registers
	// insert   -- insert element into vector
	// extract  -- extract element from vector
	// movelh   -- move low halves of two vectors together
	// movehl   -- move high halves of two vectors together
	//
	// pack     -- shrink element size (with saturation if specified)
	// unpack   -- interleave low / high halves of two vectors
	//
	// cvt      -- convert between types
	// cvtt     -- convert with truncate (ignores fractional part)
	//
	// load     -- load vector from memory
	// loadu    -- load unaligned vector from memory
	// load_ps  -- load packed float
	// load_si128 -- load 128-bit integer vector
	// store    -- store vector to memory
	// storeu   -- store unaligned vector
	// stream   -- non-temporal (write directly to memory bypassing cache)
	// maskload -- load with mask (only selected lanes)
	// maskstore-- store with mask (only selected lanes)
	//
	// max/min  -- element-wise maximum/minimum
	//
	// Keywords:
	//
	// ps       -- packed single-precision float (4 in 128-bit, 8 in 256-bit, etc.)
	// pd       -- packed double-precision float
	// epi32    -- packed int32
	// epi16    -- packed int16
	// epi8     -- packed int8
	// si128    -- integer 128-bit register
	// ss       -- scalar single-precision float
	// sd       -- scalar double-precision float





	namespace SSE
	{
		static inline void LoadRGB4(const float* src, __m128& R, __m128& G, __m128& B)
		{
			R = _mm_setr_ps(src[0], src[3], src[6], src[9]);
			G = _mm_setr_ps(src[1], src[4], src[7], src[10]);
			B = _mm_setr_ps(src[2], src[5], src[8], src[11]);
		}


		static inline void ClampFloat4(__m128& Val, const __m128& Min, const __m128& Max)
		{
			Val = _mm_min_ps(Max, _mm_max_ps(Min, Val));
		}


		static inline void ClampRGBFloat4(__m128& R, __m128& G, __m128& B, const __m128& Min, const __m128& Max)
		{
			ClampFloat4(R, Min, Max);
			ClampFloat4(G, Min, Max);
			ClampFloat4(B, Min, Max);
		}


		static inline void RGB4ToByte4(const __m128& InR, const __m128& InG, const __m128& InB, __m128i& OutR, __m128i& OutG, __m128i& OutB)
		{
			__m128 max255 = _mm_set1_ps(255.0f);
			__m128 min0 = _mm_set1_ps(0.0f);

			OutR = _mm_cvtps_epi32(InR);
			OutG = _mm_cvtps_epi32(InG);
			OutB = _mm_cvtps_epi32(InB);

			OutR = _mm_packus_epi16(_mm_packs_epi32(OutR, _mm_setzero_si128()), _mm_setzero_si128());
			OutG = _mm_packus_epi16(_mm_packs_epi32(OutG, _mm_setzero_si128()), _mm_setzero_si128());
			OutB = _mm_packus_epi16(_mm_packs_epi32(OutB, _mm_setzero_si128()), _mm_setzero_si128());
		}


		static inline void RGB4StoreBGR4(const __m128i& Ri, const __m128i& Gi, const __m128i& Bi, unsigned __int8* Dst)
		{
			// store sequentially as B G R triplets
			alignas(16) uint8_t rTmp[16], gTmp[16], bTmp[16];
			_mm_storeu_si128((__m128i*)rTmp, Ri);
			_mm_storeu_si128((__m128i*)gTmp, Gi);
			_mm_storeu_si128((__m128i*)bTmp, Bi);

			for (int i = 0; i < 4; i++)
			{
				Dst[i * 3 + 0] = bTmp[i];
				Dst[i * 3 + 1] = gTmp[i];
				Dst[i * 3 + 2] = rTmp[i];
			}
		}


		static inline void GammaApproxSqrt(__m128& c)
		{
			const __m128 zero = _mm_set1_ps(0.0f);
			const __m128 one = _mm_set1_ps(1.0f);

			c = _mm_min_ps(one, _mm_max_ps(zero, c));

			__m128 s1 = _mm_sqrt_ps(c);
			__m128 s2 = _mm_sqrt_ps(s1);

			const __m128 k1 = _mm_set1_ps(0.585122381f);
			const __m128 k2 = _mm_set1_ps(0.783140355f);

			__m128 term1 = _mm_mul_ps(s2, k1);
			__m128 term2 = _mm_mul_ps(s1, k2);

			c = _mm_add_ps(term1, term2);
		}


		static inline void SlowGamma(__m128& c)
		{
			const __m128 zero = _mm_set1_ps(0.0f);
			const __m128 one = _mm_set1_ps(1.0f);
			const __m128 threshold = _mm_set1_ps(0.0031308f);
			const __m128 a = _mm_set1_ps(12.92f);
			const __m128 b = _mm_set1_ps(1.055f);
			const __m128 c055 = _mm_set1_ps(0.055f);

			c = _mm_min_ps(one, _mm_max_ps(zero, c));

			__m128 mask = _mm_cmple_ps(c, threshold);
			__m128 linear = _mm_mul_ps(a, c);

			// Non-linear portion (gamma 1/2.4)
			float tmp[4];
			_mm_storeu_ps(tmp, c);
			for (int i = 0; i < 4; i++)
				tmp[i] = 1.055f * powf(tmp[i], 1.0f / 2.4f) - 0.055f;
			__m128 nonlinear = _mm_loadu_ps(tmp);

			// Blend linear/non-linear
			c = _mm_blendv_ps(nonlinear, linear, mask);
		}


		static inline void LinearToSRGB4(__m128& c)
		{
			GammaApproxSqrt(c);
		}
	}

	namespace AVX
	{
		static inline void LoadRGB8(const float* src, __m256& R, __m256& G, __m256& B)
		{
			R = _mm256_setr_ps(src[0], src[3], src[6], src[9],
				src[12], src[15], src[18], src[21]);

			G = _mm256_setr_ps(src[1], src[4], src[7], src[10],
				src[13], src[16], src[19], src[22]);

			B = _mm256_setr_ps(src[2], src[5], src[8], src[11],
				src[14], src[17], src[20], src[23]);
		}

		static inline void ClampFloat8(__m256& Val, const __m256& Min, const __m256& Max)
		{
			Val = _mm256_min_ps(Max, _mm256_max_ps(Min, Val));
		}


		static inline void ClampRGBFloat8(__m256& R, __m256& G, __m256& B, const __m256& Min, const __m256& Max)
		{
			ClampFloat8(R, Min, Max);
			ClampFloat8(G, Min, Max);
			ClampFloat8(B, Min, Max);
		}


		static inline void RGB8ToByte8(const __m256& InR, const __m256& InG, const __m256& InB, __m256i& OutR, __m256i& OutG, __m256i& OutB)
		{
			__m256 max255 = _mm256_set1_ps(255.0f);
			__m256 min0 = _mm256_set1_ps(0.0f);

			OutR = _mm256_cvtps_epi32(InR);
			OutG = _mm256_cvtps_epi32(InG);
			OutB = _mm256_cvtps_epi32(InB);

			OutR = _mm256_packus_epi16(_mm256_packs_epi32(OutR, _mm256_setzero_si256()), _mm256_setzero_si256());
			OutG = _mm256_packus_epi16(_mm256_packs_epi32(OutG, _mm256_setzero_si256()), _mm256_setzero_si256());
			OutB = _mm256_packus_epi16(_mm256_packs_epi32(OutB, _mm256_setzero_si256()), _mm256_setzero_si256());
		}


		static inline void RGB8StoreBGR8(const __m256i& Ri, const __m256i& Gi, const __m256i& Bi, unsigned __int8* Dst)
		{
			// store sequentially as B G R triplets
			alignas(16) uint8_t rTmp[16], gTmp[16], bTmp[16];
			_mm256_storeu_si256((__m256i*)rTmp, Ri);
			_mm256_storeu_si256((__m256i*)gTmp, Gi);
			_mm256_storeu_si256((__m256i*)bTmp, Bi);

			for (int i = 0; i < 8; i++)
			{
				Dst[i * 3 + 0] = bTmp[i];
				Dst[i * 3 + 1] = gTmp[i];
				Dst[i * 3 + 2] = rTmp[i];
			}
		}

		static inline void GammaApproxSqrt(__m256& c)
		{
			const __m256 zero = _mm256_set1_ps(0.0f);
			const __m256 one = _mm256_set1_ps(1.0f);

			c = _mm256_min_ps(one, _mm256_max_ps(zero, c));

			__m256 s1 = _mm256_sqrt_ps(c);
			__m256 s2 = _mm256_sqrt_ps(s1);

			const __m256 k1 = _mm256_set1_ps(0.585122381f);
			const __m256 k2 = _mm256_set1_ps(0.783140355f);

			__m256 term1 = _mm256_mul_ps(s2, k1);
			__m256 term2 = _mm256_mul_ps(s1, k2);

			c = _mm256_add_ps(term1, term2);
		}

		static inline void LinearToSRGB8(__m256& c)
		{
			GammaApproxSqrt(c);
		}
	}
}

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
	alignas(64) std::atomic<size_t> TasksInProgress{ 0 };

	void WorkerLoop()
	{
		std::function<void()> Task = nullptr;

		while (true)
		{
			Task = nullptr;

			{
				std::unique_lock<std::mutex> QueueLock(QueueMutex);

				TaskConditionVar.wait(QueueLock, [this] {
					return JoinThreads || !Queue.empty();
					});

				if (JoinThreads && Queue.empty())
					return;

				Task = std::move(Queue.front());
				Queue.pop();	

				TasksInProgress.fetch_add(1, std::memory_order_relaxed);
			}

			if (this->JoinThreads)
			{
				return;
			}

			if (Task)
			{
				Task();
				
				{
					std::lock_guard<std::mutex> lock(QueueMutex);
					TasksInProgress.fetch_sub(1, std::memory_order_relaxed);
				}

				QueueSzConditionVar.notify_all(); // notify WaitUntilAllTasksFinished
			}
		}
	}


	public:
	std::queue<std::function<void()>> Queue;

	ThreadManager()
	{
		this->CoreCount = GetCpuCores();
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
		return std::max(1u, std::thread::hardware_concurrency());
	}

	template<typename FunctionToCall, typename... Arguments>
	void EnqueueTask(FunctionToCall&& Func, Arguments&&... Args)
	{
		if (JoinThreads)
			return;

		std::function<void()> task = std::bind(std::forward<FunctionToCall>(Func), std::forward<Arguments>(Args)...);
		
		{
			std::lock_guard<std::mutex> lock(QueueMutex);
			Queue.emplace(std::move(task));
		}

		this->TaskConditionVar.notify_one();
	}

	void WaitUntilAllTasksFinished()
	{
		std::unique_lock<std::mutex> Lock(QueueMutex);

		QueueSzConditionVar.wait(Lock, [this]() {
			return Queue.empty() && TasksInProgress.load(std::memory_order_relaxed) == 0;
			});
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
