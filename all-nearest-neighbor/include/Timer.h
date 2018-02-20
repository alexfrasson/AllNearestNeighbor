#pragma once
#include <string>
#include <iostream>
#include <chrono>

class Timer
{
public:
	explicit Timer(const std::string& name) : m_name(name)
	{
		m_start = std::chrono::high_resolution_clock::now();
	}

	~Timer()
	{
		auto end = std::chrono::high_resolution_clock::now();

		auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(end - m_start);

		std::cout << m_name.c_str() << " time:" << elapsedMs.count() << "ms" << std::endl;
	}
private:

	std::string m_name;
	std::chrono::high_resolution_clock::time_point m_start;
};