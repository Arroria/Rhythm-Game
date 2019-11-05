#include "SoundDevice.h"


SoundChannel::SoundChannel()
	: m_channel(nullptr)
{
}

SoundChannel::SoundChannel(SoundSample& soundSample)
	: m_channel(nullptr)
{
	soundSample._get_device()->_get_primitive_system()->playSound(soundSample._get_primitive_sound(), nullptr, false, std::addressof(m_channel));
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
	if constexpr (SOUNDDEVICE_SAFETY)
		if (!is_available())
			return;
	m_channel->setPaused(false);
}

void SoundChannel::pause()
{
	if constexpr (SOUNDDEVICE_SAFETY)
		if (!is_available())
			return;

	_pause();
}

void SoundChannel::stop()
{
	if constexpr (SOUNDDEVICE_SAFETY)
		if (!is_available())
			return;

	_pause();
	m_channel->setPosition(0, FMOD_TIMEUNIT_MS);
}

void SoundChannel::release()
{
	if constexpr (SOUNDDEVICE_SAFETY)
		if (!is_available())
			return;

	_release();
}

void SoundChannel::detach()
{
	if constexpr (SOUNDDEVICE_SAFETY)
		if (!is_available())
		{
			m_channel = nullptr;
			return;
		}

	if (_is_paused())
		_release();
	else
		m_channel = nullptr;
}



bool SoundChannel::is_available() const
{
	bool available(false);
	if (m_channel)
		m_channel->isPlaying(&available);
	return available;
}

bool SoundChannel::is_playing() const { return !is_paused(); }
bool SoundChannel::is_paused() const
{
	if constexpr (SOUNDDEVICE_SAFETY)
		if (!is_available())
			return false;

	return _is_paused();
}

unsigned int SoundChannel::get_time() const
{
	if constexpr (SOUNDDEVICE_SAFETY)
		if (!is_available())
			return NULL;

	unsigned int time;
	m_channel->getPosition(&time, FMOD_TIMEUNIT_MS);
	return time;
}



FMOD::Sound* SoundChannel::_get_primitive()
{
	if constexpr (SOUNDDEVICE_SAFETY)
		if (!is_available())
			return nullptr;

	FMOD::Sound* sound(nullptr);
	m_channel->getCurrentSound(std::addressof(sound));
	return sound;
}



void SoundChannel::_release() { m_channel->stop(); }

void SoundChannel::_pause() { m_channel->setPaused(true); }

bool SoundChannel::_is_paused() const
{
	bool paused(false);
	m_channel->getPaused(&paused);
	return paused;
}
