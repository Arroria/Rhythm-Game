#include <Windows.h>
#include <string_view>

#include "SoundDevice.h"
constexpr std::string_view _path_drum = "../../../../FMOD Studio API Windows/api/core/examples/media/drumloop.wav";

int main()
{
	SoundDevice soundDevice;
	soundDevice.Initialize();

	{
		SoundSample sound1(soundDevice.CreateSoundSample(_path_drum));

		SoundChannel channel(sound1);
		Sleep(5000);
	}

	soundDevice.Release();
	return 0;
}
