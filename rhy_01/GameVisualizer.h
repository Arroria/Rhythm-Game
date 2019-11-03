#pragma once
#include "Lane.h"

class GameVisualizer
{
public:
	GameVisualizer(Lane* lane1, Lane* lane2, Lane* lane3, Lane* lane4, size_t speedBpm);
	~GameVisualizer();

	void Render(Lane::time_t timing) const;

private:
	Lane* m_lane[4];
	size_t m_speedBpm;
};

