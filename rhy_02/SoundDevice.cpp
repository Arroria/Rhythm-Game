#include "SoundDevice.h"

SoundDevice::SoundDevice()
	: m_system(nullptr)
{
	FMOD::System_Create(&m_system);
}

SoundDevice::~SoundDevice()
{
	m_system->release();
}


void SoundDevice::Initialize()
{
	m_system->init(512, FMOD_INIT_NORMAL, 0);
}

void SoundDevice::Release()
{
	m_system->close();
}

SoundSample SoundDevice::CreateSoundSample(std::string_view filePath)
{
	return SoundSample(*this, filePath);
}





SoundSample::SoundSample()
	: m_device(nullptr)
	, m_sound(nullptr)
{
}

SoundSample::SoundSample(SoundDevice& soundDevice, std::string_view filePath)
	: m_device(std::addressof(soundDevice))
	, m_sound(nullptr)
{
	soundDevice._get_primitive_system()->createSound(filePath.data(), FMOD_DEFAULT | FMOD_LOOP_OFF, nullptr, std::addressof(m_sound));
}

SoundSample::SoundSample(SoundSample&& soundSample)
	: m_device(soundSample.m_device)
	, m_sound(soundSample.m_sound)
{
	soundSample.m_device = nullptr;
	soundSample.m_sound = nullptr;
}

SoundSample::~SoundSample()
{
	if (m_sound)
		_release_unsafe();
}


SoundSample& SoundSample::operator=(SoundSample&& soundSample)
{
	if (m_sound)
		_release_unsafe();

	m_device = soundSample.m_device;
	m_sound = soundSample.m_sound;
	soundSample.m_device = nullptr;
	soundSample.m_sound = nullptr;
	return *this;
}

SoundChannel SoundSample::play()
{
	if (!m_sound)
		return SoundChannel();
	return SoundChannel(*this);
}

void SoundSample::release()
{
	if (m_sound)
	{
		_release_unsafe();
		_set_null();
	}
}


void SoundSample::_release_unsafe()
{
	m_sound->release();
}

void SoundSample::_set_null()
{
	m_device = nullptr;
	m_sound = nullptr;
}
