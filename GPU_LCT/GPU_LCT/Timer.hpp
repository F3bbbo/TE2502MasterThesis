#pragma once
#include <chrono>

class Timer
{
	using Clock = std::chrono::high_resolution_clock;
	using TimePoint = std::chrono::time_point<Clock>;

public:
	Timer();
	~Timer();

	void start();
	void stop();

	double elapsed_time();

private:
	TimePoint m_start;
	TimePoint m_stop;
};

