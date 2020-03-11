#pragma once
#include "BeatNoteSheet.h"
#include <array>

class BNS_Editor
{
public:
	BNS_Editor();
	~BNS_Editor();

	bool Initialize();
	void Update();
	void Release();

	void InjectBNS(const BeatNoteSheet& bns);


	// new func
	bool GetNote(int laneIndex, int beatIndex) const;
	bool FixNote(int laneIndex, int beatIndex, bool setNoteActive);

	int MaximumBeat() const;
	int MaximumBar() const;


	void SetEditorCursor_ByBar(float barPosition);
	void SetEditorCursor_ByTime(int millisecond);


private:
	// Sound
	int _maximum_beat() const;
	int _maximum_bar() const;


	void _set_editorCursor(float barPosition);
	float _editorCursor() const;

	void _set_music_time(int millisecond);
	int _music_current_time() const;


	float _time_to_barPosition(int millisecond) const;
	int _barPosition_to_time(float barPosition) const;
	

private:
	// BNS Data
	int m_bpm;
	int m_beatPerBar;
	std::array<std::vector<bool>, 4> m_beatNote;

	// Music Data
	SoundSample m_musicData;
	int m_musicOffsetMillisecond;
	
	// Editor Data
	float m_editorCursor;

	SoundChannel m_musicChannel;
	bool m_musicSyncMode;


	// Sound Management
public:
	bool LoadMusic(std::string_view musicPath, std::chrono::milliseconds musicOffset);
	void SetMusicSyncMode(bool active);
	bool IsMusicSyncMode();


private:
	bool _music_regist(std::string_view musicPath);
	bool _music_regist(SoundSample&& musicSample);
	bool _music_is_playing() const;

	void _music_channel_initialize();



	// Screen I/O
	// Define in SIO.cpp
private:
	typedef enum RTName : int
	{
		noteScn_master,
		noteScn_grid,
		noteScn_note,
		previewScn_master,
		previewScn_grid,
		previewScn_note,
		previewScn_noteScnCamera,
		__max
	} RTName_t;

public:
	void Render();

	// IO
	bool PreviewScreenScroll(int powerForRight);
	bool PreviewScreenScrollLeftOnce();
	bool PreviewScreenScrollRightOnce();

	// IO helper
	bool MouseInNoteScreen(POINT mousePosition) const;
	bool MouseInPreviewScreen(POINT mousePosition) const;

	bool MouseToNoteScreenNotePosition(POINT mousePosition, int& out_laneIndex, int& out_beatIndex) const;
	bool MouseToPreviewScreenBarPositon(POINT mousePosition, float& out_barPostion) const;


private:
	void DrawRT_noteScn_grid();
	void DrawRT_noteScn_note();
	void DrawRT_noteScn_master();
	void Render_noteScn();

	void DrawRT_previewScn_grid();
	void DrawRT_previewScn_note();
	void DrawRT_previewScn_noteScnCamera();
	void DrawRT_previewScn_master();
	void Render_previewScn();

	void FixRT_noteScn_note(int laneIndex, int beatIndex, bool active);
	void FixRT_previewScn_note(int laneIndex, int beatIndex, bool active);



	WNDRD_RenderTarget& _get_rt(RTName rtName);

	int _previewScn_maximum_chunk() const;
	int _previewScn_minimum_scrollLevel() const;
	int _previewScn_maximum_scrollLevel() const;
	int _previewScn_scrolledWidth() const;

private:
	std::array<WNDRD_RenderTarget, __max> m_renderTarget;
	float m_prevRenderedBarPosition;
	int m_noteScn_barIndex;
	int m_previewScn_scrollLevel;
	bool m_previewScn_scrolled;
	bool m_noteFixed;
};
