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

double Timer::elapsed_time()
{
	auto mili_time = std::chrono::duration_cast<std::chrono::microseconds>(m_stop - m_start).count();
	return  mili_time / 1000.0f;
}
