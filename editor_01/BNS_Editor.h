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
	void _test_update();
	void Render();
	void Release();

	void InjectBNS(const BeatNoteSheet& bns);

private:
	bool ScreenToNote(int& out_lane, int& out_beat, POINT pos);

	bool CreateNoteScreenGrid();
	bool CreateNoteScreen();

	void DrawNoteScreenGrid();
	void DrawNoteScreen();


private:
	typedef enum RTName : int
	{
		NoteScreenGrid,
		NoteScreen,
		__max
	} RTName_t;

	// BNS Data
	int m_bpm;
	int m_beatPerBar;
	std::array<std::vector<bool>, 4> m_beatNote;

	// Editor Data
	int m_noteCursor;

	std::array<WNDRD_RenderTarget, __max> m_renderTarget;



	WNDRD_RenderTarget& _get_rt(RTName rtName);
};
