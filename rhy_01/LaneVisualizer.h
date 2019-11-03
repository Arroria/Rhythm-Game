#pragma once
#include "Lane.h"

class LaneVisualizer
{
public:
	LaneVisualizer(Lane* lane, size_t speedBpm);
	~LaneVisualizer();

	void Render(size_t line, Lane::time_t timing) const;

private:
	Lane* m_lane;
	size_t m_speedBpm;
};

