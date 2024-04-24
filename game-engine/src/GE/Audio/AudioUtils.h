#pragma once

#include <al.h>

namespace GE
{
	struct AudioBuffer
	{
		ALuint Buffer = 0;

		uint8_t Channels = 2;
		int32_t SampleRate = 16;
		// Bits per Sample
		uint8_t BPS = 0;
		ALsizei Size = 0;

		char* Data = 0;
		ALenum Format = 0;

		std::filesystem::path FilePath;
	};

	struct LongAudioBuffer
	{
		static const uint32_t NUM_BUFFERS = 4;
		static const uint32_t BUFFER_SIZE = 65536; // 32kb of data in each buffer

		std::size_t Cursor = BUFFER_SIZE * NUM_BUFFERS;

		ALuint Buffers[NUM_BUFFERS];

		uint8_t Channels = 2;
		int32_t SampleRate = 16;
		// Bits per Sample
		uint8_t BPS = 0;

		std::vector<char*> Data = std::vector<char*>(NUM_BUFFERS);
		ALenum Format = 0;

		std::filesystem::path FilePath;
	};
}
