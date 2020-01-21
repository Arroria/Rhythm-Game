#pragma once
#include <array>
#include <vector>
struct BeatNoteSheet
{
public:
	BeatNoteSheet();
	BeatNoteSheet(BeatNoteSheet& bns);
	BeatNoteSheet(BeatNoteSheet&& bns) noexcept;
	~BeatNoteSheet();

	BeatNoteSheet& operator=(BeatNoteSheet& bns);
	BeatNoteSheet& operator=(BeatNoteSheet&& bns) noexcept;


	std::pair<int, int> m_version;
	int m_bpm;
	int m_beatPerBar;
	std::array<std::vector<bool>, 4> m_beatData;
};


bool BNS_Load(std::string_view path, OUT BeatNoteSheet& out_bns);
bool BNS_Save(std::string_view path, const BeatNoteSheet& in_bns);
