#include "pch.h"
#include "BeatNoteSheet.h"

#include <fstream>
#include <string>
#include <string_view>
#include <array>
constexpr size_t _token_reserve_size = 20;
bool BNS_Load(std::string_view path, OUT BeatNoteSheet& out_bns)
{
	std::fstream bnsFile(path.data(), std::ios::in);
	if (!bnsFile.is_open())
		return false;

	std::string token;
	token.reserve(_token_reserve_size);
	auto UpdateToken = [&bnsFile, &token]()->std::string & {
		token.clear();
		if (!bnsFile.eof())
			bnsFile >> token;
		return token;
	};

	std::array<bool, 4> validation;
	validation.fill(false);

	while (!UpdateToken().empty())
	{
		if (false);
		else if (token == "edv")
		{
			if (UpdateToken().empty())
				return false;

			int* mainVer = &out_bns.m_version.first;
			int* subVer = &out_bns.m_version.second;
			sscanf(token.data(), "%d.%dv", mainVer, subVer);
			validation[0] = true;
		}
		else if (token == "bpm")
		{
			if (UpdateToken().empty())
				return false;
			out_bns.m_bpm = std::stoi(token);
			validation[1] = true;
		}
		else if (token == "beat_unit")
		{
			if (UpdateToken().empty())
				return false;
			out_bns.m_beatPerBar = std::stoi(token);
			validation[2] = true;
		}
		else if (token == "beat_data_begin")
		{
			while (true)
			{
				if (UpdateToken().empty())
					return false;
				if (token == "beat_data_end")
					break;

				int lane = 0, beat = 0;
				lane = std::stoi(token);
				if (UpdateToken().empty())
					return false;
				beat = std::stoi(token);

				auto& laneDatalist = out_bns.m_beatData[lane];
				if (laneDatalist.size() <= beat)
					laneDatalist.resize(beat + 1);
				laneDatalist[beat] = true;
			}
			validation[3] = true;
		}
	}

	for (auto iter = validation.begin(); iter != validation.end(); ++iter)
	{
		if (!(*iter))
			return false;
	}
	return true;
}

bool BNS_Save(std::string_view path, const BeatNoteSheet& in_bns)
{
	std::fstream bnsFile(path.data(), std::ios::out | std::ios::trunc);
	if (!bnsFile.is_open())
		return false;

	constexpr char escSeq = '\n';
	constexpr size_t verStrMaxLength = 20;
	char versionStr[verStrMaxLength + 1] = { NULL };
	if (int verStrResult = snprintf(versionStr, verStrMaxLength + 1, "%d.%dv", in_bns.m_version.first, in_bns.m_version.second);
		verStrResult < 0 || verStrMaxLength < verStrResult)
		return false;

	bnsFile << "edv" << ' ' << versionStr << escSeq;
	bnsFile << "bpm" << ' ' << in_bns.m_bpm << escSeq;
	bnsFile << "beat_unit" << ' ' << in_bns.m_beatPerBar << escSeq;
	bnsFile << "beat_data_begin" << escSeq;
	for (size_t lane = 0; lane < in_bns.m_beatData.size(); ++lane)
	{
		auto& noteList = in_bns.m_beatData[lane];
		for (size_t beat = 0; beat < noteList.size(); ++beat)
		{
			if (noteList[beat])
				bnsFile << lane << ' ' << beat << escSeq;
		}
	}
	bnsFile << "beat_data_end" << escSeq;
	return true;
}
