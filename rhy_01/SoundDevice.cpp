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
	: m_myDevice(nullptr)
	, m_sound(nullptr)
{
}

SoundSample::SoundSample(SoundDevice& soundDevice, std::string_view filePath)
{
	m_myDevice = std::addressof(soundDevice);
	soundDevice._get_fmod_system()->createSound(filePath.data(), FMOD_DEFAULT | FMOD_LOOP_OFF, nullptr, std::addressof(m_sound));
}

SoundSample::SoundSample(SoundSample&& soundSample)
	: m_myDevice(soundSample.m_myDevice)
	, m_sound(soundSample.m_sound)
{
	soundSample.m_myDevice = nullptr;
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

	m_myDevice = soundSample.m_myDevice;
	m_sound = soundSample.m_sound;
	soundSample.m_myDevice = nullptr;
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
	m_sound = nullptr;
	m_myDevice = nullptr;
}





SoundChannel::SoundChannel()
	: m_channel(nullptr)
{
}

SoundChannel::SoundChannel(SoundSample& soundSample)
	: m_channel(nullptr)
{
	soundSample._get_sound_device()->_get_fmod_system()->playSound(soundSample._get_primitive_sound(), nullptr, false, std::addressof(m_channel));
}

SoundChannel::SoundChannel(SoundChannel&& soundChannel)
	: m_channel(soundChannel.m_channel)
{
	soundChannel.m_channel = nullptr;
}

SoundChannel::~SoundChannel()
{
	stop();
}


SoundChannel& SoundChannel::operator=(SoundChannel&& soundChannel)
{
	detach();

	m_channel = soundChannel.m_channel;
	soundChannel.m_channel = nullptr;
	return *this;
}


void SoundChannel::play()
{
	if (m_channel)
		m_channel->setPaused(false);
}

void SoundChannel::pause()
{
	if (m_channel)
		m_channel->setPaused(true);
}

void SoundChannel::stop()
{
	if (m_channel)
		m_channel->stop();
}

void SoundChannel::detach()
{
	m_channel = nullptr;
}
