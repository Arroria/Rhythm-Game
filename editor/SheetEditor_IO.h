#pragma once
#include "SheetEditor.h"

#include "mywinapi.h"
class SheetEditor_IO
{
public:
	SheetEditor_IO();
	~SheetEditor_IO();


	void Initialize(size_t predictionMaxNode, size_t baseBeatPerBar);
	void Update();
	void Render();
	void Release();


	void __temp_draw(size_t laneIndex, size_t barIndex, size_t beatIndex) { _draw_note(laneIndex, barIndex, beatIndex, true); }
	MyRenderTarget& _get_result() { return m_resultRt; }


private:
	SheetEditor* m_editor;

	MyRenderTarget m_resultRt;
	MyRenderTarget m_gridRt;
	std::vector<MyRenderTarget> m_noteRt;

	MyRenderTarget m_editableScreenRt;

	int m_previewScreenScroll;
	float m_focusBox_barPosition;
	bool m_previewScreenRedraw;

	size_t m_edisc_focusBox_laneIndex;
	size_t m_edisc_focusBox_beatIndex;
	bool m_editableScreenRedraw;

	size_t m_beatForEdit; // Only 1, 2, 3, 4, 6, 8, 12, 16, 24 and 48 are supported.
	

	void _resize_note_rt(size_t size);
	void _draw_grid();
	void _draw_note(size_t laneIndex, size_t barIndex, size_t beatIndex, bool state);
};
