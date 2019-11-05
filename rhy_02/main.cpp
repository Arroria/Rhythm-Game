#include <Windows.h>
#include <string_view>

#include "SoundDevice.h"
SoundDevice g_soundDevice;

#include "InputDevice/InputDevice.h"
#if _DEBUG
#pragma comment (lib, "InputDevice/Debug/x64/InputDevice.lib")
#else
#pragma comment (lib, "InputDevice/Release/x64/InputDevice.lib")
#endif
InputDevice g_inputDevice;



constexpr std::string_view _path_test = "../../../../FMOD Studio API Windows/api/core/examples/media/standrews.wav";
constexpr std::string_view _path_song = "697873717765.mp3";

int main()
{
	g_soundDevice.Initialize();
	{
		SoundSample sound_test(g_soundDevice, _path_test);
		SoundChannel channel(sound_test);
		while (channel.is_available());
	}
	g_soundDevice.Release();
	return 0;
}
