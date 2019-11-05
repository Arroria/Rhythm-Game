#pragma once
#include "fmod.hpp"
#include <string_view>

#define SOUNDDEVICE_SAFETY true

class SoundDevice;
class SoundSample;
class SoundChannel
{
public:
	SoundChannel();
	SoundChannel(SoundSample& soundSample);
	SoundChannel(SoundChannel&& soundChannel);
	~SoundChannel();

	SoundChannel& operator=(SoundChannel&& soundChannel);


	void play();
	void pause();
	void stop();
	void release();
	void detach();

	bool is_available() const;
	bool is_playing() const;
	bool is_paused() const;

	unsigned int get_time() const;


private:
	FMOD::Channel* m_channel;

	FMOD::Sound* _get_primitive();


	void _release();
	void _pause();
	bool _is_paused() const;
};

class SoundSample
{
public:
	SoundSample();
	SoundSample(SoundDevice& soundDevice, std::string_view filePath);
	SoundSample(SoundSample&& soundSample);
	~SoundSample();

	SoundSample& operator=(SoundSample&& soundSample);

	SoundDevice* _get_device() { return m_device; }
	FMOD::Sound* _get_primitive_sound() { return m_sound; }

	SoundChannel play();
	void release();

private:
	SoundDevice* m_device;
	FMOD::Sound* m_sound;

	void _release_unsafe();
	void _set_null();

};

class SoundDevice
{
public:
	SoundDevice();
	~SoundDevice();

	void Initialize();
	void Release();

	SoundSample CreateSoundSample(std::string_view filePath);

	FMOD::System* _get_primitive_system() { return m_system; }

private:
	FMOD::System* m_system;
};
