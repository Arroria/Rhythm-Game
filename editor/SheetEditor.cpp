#include "pch.h"
#include "SheetEditor.h"

SheetEditor::SheetEditor()
	: m_beatPerBar(NULL)
	, m_note()
{
}

SheetEditor::~SheetEditor()
{
}


void SheetEditor::Initialize(size_t predictionMaxNode, size_t baseBeatPerBar)
{
	m_beatPerBar = baseBeatPerBar;
	for (size_t i = 0; i < m_note.size(); i++)
		m_note[i].resize(predictionMaxNode * baseBeatPerBar, false);
}

void SheetEditor::SetNote(size_t lane, size_t bar, size_t beat, bool state)
{
	auto& noteContainer = m_note[lane];

	size_t notePosition = bar * m_beatPerBar + beat;
	if (noteContainer.size() <= notePosition)
		noteContainer.resize(notePosition + 1, false);
	noteContainer[notePosition] = state;
}

bool SheetEditor::NoteFlip(size_t lane, size_t bar, size_t beat)
{
	auto& noteContainer = m_note[lane];

	size_t notePosition = bar * m_beatPerBar + beat;
	if (noteContainer.size() <= notePosition)
		noteContainer.resize(notePosition + 1, false);

	return noteContainer[notePosition] = !noteContainer[notePosition];
}
