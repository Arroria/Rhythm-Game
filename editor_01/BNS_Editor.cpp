#include "pch.h"
#include "BNS_Editor.h"

constexpr size_t _beat_per_bar = 48;

BNS_Editor::BNS_Editor()
	: m_bpm(180)
	, m_beatPerBar(_beat_per_bar)
	, m_beatNote()
	, m_musicData()
	, m_musicOffsetMillisecond(0)
	
	, m_editorCursor(0)
	, m_musicChannel()
	, m_musicSyncMode(false)

	// for sio
	, m_renderTarget()
	, m_prevRenderedBarPosition(0)
	, m_noteScn_barIndex(0)
	, m_previewScn_scrollLevel(0)
	, m_previewScn_scrolled(false)
	, m_noteFixed(false)
{
	for (auto& bnList : m_beatNote)
		bnList.resize(160 * m_beatPerBar, false);
}

BNS_Editor::~BNS_Editor()
{
}



bool BNS_Editor::Initialize()
{
	return true;
}

void BNS_Editor::Update()
{
	if (m_musicSyncMode)
	{
		if (m_musicChannel.is_available())
			_set_editorCursor(_time_to_barPosition(_music_current_time()));
		else
		{
			_music_channel_initialize();
			_set_editorCursor(0);
			m_musicSyncMode = false;
		}
	}
}

void BNS_Editor::Release()
{
	m_musicData.release();
	for (auto& rt : m_renderTarget)
		rt.Release();
}



void BNS_Editor::InjectBNS(const BeatNoteSheet& bns)
{
	m_bpm = bns.m_bpm;
	m_beatNote = bns.m_beatData;

	// safety sizing
	{
		int maxBeat = 0;
		for (const auto& vec : m_beatNote)
			maxBeat = std::max<int>(vec.size(), maxBeat);
		int maxBar = (maxBeat + _beat_per_bar - 1) / _beat_per_bar;
		
		int finalBeat = maxBar * _beat_per_bar;
		for (auto& list : m_beatNote)
		list.resize(finalBeat);
	}

	DrawRT_noteScn_note();
	DrawRT_previewScn_note();
}



bool BNS_Editor::GetNote(int laneIndex, int beatIndex) const { return m_beatNote[laneIndex][beatIndex]; }
bool BNS_Editor::FixNote(int laneIndex, int beatIndex, bool setNoteActive)
{
	m_beatNote[laneIndex][beatIndex] = setNoteActive;
	FixRT_noteScn_note(laneIndex, beatIndex, setNoteActive);
	FixRT_previewScn_note(laneIndex, beatIndex, setNoteActive);
	m_noteFixed = true;
	return true;
}



int BNS_Editor::MaximumBeat() const	{ return _maximum_beat(); }
int BNS_Editor::MaximumBar() const	{ return _maximum_bar(); }

void BNS_Editor::SetEditorCursor_ByBar(float barPosition)
{
	_set_editorCursor(barPosition);
	_set_music_time(_barPosition_to_time(barPosition));
}
void BNS_Editor::SetEditorCursor_ByTime(int millisecond)
{
	_set_editorCursor(_time_to_barPosition(millisecond));
	_set_music_time(millisecond);
}


// Inject중 safety sizing으로 별도 처리가 필요 없어짐
int BNS_Editor::_maximum_beat() const { return m_beatNote[0].size(); }
int BNS_Editor::_maximum_bar() const { return (_maximum_beat() + _beat_per_bar - 1) / _beat_per_bar; }



bool BNS_Editor::LoadMusic(std::string_view musicPath, std::chrono::milliseconds musicOffset)
{
	if (!_music_regist(musicPath))
		return false;

	m_musicOffsetMillisecond = musicOffset.count();
	return true;
}

void BNS_Editor::SetMusicSyncMode(bool active)
{
	if (!m_musicData._get_primitive_sound()) return;
	if (!m_musicChannel.is_available())	std::terminate();

	if (active)
	{
		_set_music_time(_barPosition_to_time(_editorCursor()));
		m_musicChannel.play();
	}
	else
		m_musicChannel.pause();
	m_musicSyncMode = active;
}
bool BNS_Editor::IsMusicSyncMode() { return m_musicSyncMode; }


bool BNS_Editor::_music_regist(std::string_view musicPath) { return _music_regist(g_soundDevice.CreateSoundSample(musicPath)); }
bool BNS_Editor::_music_regist(SoundSample&& musicSample)
{
	m_musicData = std::move(musicSample);
	_music_channel_initialize();
	return true;
}

bool BNS_Editor::_music_is_playing() const
{
	if (!m_musicChannel.is_available())
		std::terminate();

	return m_musicChannel.is_playing();
}


void BNS_Editor::_music_channel_initialize()
{
	m_musicChannel = std::move(m_musicData.play());
	m_musicChannel.stop();
}






void BNS_Editor::_set_editorCursor(float barPosition)	{ m_editorCursor = barPosition; }
float BNS_Editor::_editorCursor() const					{ return m_editorCursor; }

void BNS_Editor::_set_music_time(int millisecond)	{ m_musicChannel.set_time(std::chrono::milliseconds(millisecond + m_musicOffsetMillisecond)); }
int BNS_Editor::_music_current_time() const			{ return m_musicChannel.get_time() - m_musicOffsetMillisecond; }

float BNS_Editor::_time_to_barPosition(int millisecond) const	{ return float(millisecond) * m_bpm / 240 / 1000; }
int BNS_Editor::_barPosition_to_time(float barPosition) const	{ return barPosition * 1000 * 240 / m_bpm; }
