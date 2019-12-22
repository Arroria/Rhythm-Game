#pragma once
#include <string_view>
#include <vector>
#include <array>
class SheetEditor
{
public:
	SheetEditor();
	~SheetEditor();


	void Initialize(size_t predictionMaxNode, size_t baseBeatPerBar);


	void SetNote(size_t lane, size_t bar, size_t beat, bool state);
	void AddNote(size_t lane, size_t bar, size_t beat)		{ SetNote(lane, bar, beat, true); }
	void RemoveNote(size_t lane, size_t bar, size_t beat) { SetNote(lane, bar, beat, false); }
	bool NoteFlip(size_t lane, size_t bar, size_t beat);

	void Clear() { size_t size = m_note[0].size(); for (auto iter = m_note.begin(); iter != m_note.end(); ++iter) { iter->clear(); iter->resize(size); } }

	const std::array<std::vector<bool>, 4> & __get_data() const { return m_note; }


private:
	size_t m_beatPerBar;
	std::array<std::vector<bool>, 4> m_note;
};
