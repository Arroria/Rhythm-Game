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


	int MaxBeatCount() const;
	int MaxBarCount() const;
	int PreScnMaxChunkCount() const;

	// Preview Screen
	void PreScnScrollLeft();
	void PreScnScrollRight();
	int PreScnScrollMax() const;

private:
	bool ScreenToNote(int& out_lane, int& out_beat, POINT pos);
	bool ScreenToMusicCursor(int& out_musicCursor, POINT pos);

	bool DrawNoteScreenGrid();
	bool DrawNoteScreen();
	bool DrawPreviewScreenGreed();
	bool DrawPreviewScreen();

	void RenderNoteScreenGrid();
	void RenderNoteScreen();
	void RenderNoteScreenCursor();
	void RenderPreviewScreenGreed();
	void RenderPreviewScreen();
	void RenderPreviewScreenCursor();

	float NoteCursorToBar() const;


private:
	typedef enum RTName : int
	{
		NoteScreenGrid,
		NoteScreen,
		PreviewScreenGrid,
		PreviewScreen,
		__max
	} RTName_t;

	// BNS Data
	int m_bpm;
	int m_beatPerBar;
	std::array<std::vector<bool>, 4> m_beatNote;

	// Editor Data
	int m_noteCursor;
	int m_previewScroll;

	std::array<WNDRD_RenderTarget, __max> m_renderTarget;



	WNDRD_RenderTarget& _get_rt(RTName rtName);
};
