#pragma once
#include <string_view>
#include <vector>
#include <array>
class SheetEditor
{
public:
	using string_view_t = std::string_view;

	SheetEditor();
	~SheetEditor();


	void Initialize(size_t predictionMaxNode, size_t baseBeatPerBar);
	void Initialize(string_view_t filePath); // TODO


	void SetNote(size_t lane, size_t bar, size_t beat, bool state);
	void AddNote(size_t lane, size_t bar, size_t beat)		{ SetNote(lane, bar, beat, true); }
	void RemoveNote(size_t lane, size_t bar, size_t beat) { SetNote(lane, bar, beat, false); }
	bool NoteFlip(size_t lane, size_t bar, size_t beat);
	//void AddCustomNote(size_t node, size_t beatPerBar, size_t index);

	void ExtractFile(string_view_t filePath); // TODO


	const std::array<std::vector<bool>, 4> & __get_data() const { return m_note; }


private:
	size_t m_beatPerBar;
	std::array<std::vector<bool>, 4> m_note;
};
