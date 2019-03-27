#include "Timer.hpp"

Timer::Timer()
{
	m_start = Clock::now();
	m_stop = Clock::now();
}


Timer::~Timer()
{
}

void Timer::start()
{
	m_start = Clock::now();
}

void Timer::stop()
{
	m_stop = Clock::now();
}

long long Timer::elapsed_time()
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(m_stop - m_start).count();
}
