#pragma once

// Windows OS
#ifdef _WIN32
#include <Mmsystem.h>
#include <mciapi.h>

// If not using Visual c++ include this lib
#pragma comment(lib, "Winmm.lib")

namespace TerraAL
{
	void PlayWav()
	{

	}

	void AsyncPlayWav()
	{

	}
}


#endif




// Linux OS
#ifdef __linux__



#endif