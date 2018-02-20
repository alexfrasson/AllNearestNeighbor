#include "../include/PointValidation.h"
#include "../include/Parser.h"
#include <algorithm>
#include <thread>
#include <vector>
#include <iostream>

void validateResults_Thread(const std::vector<std::pair<Point, Point>>& solution, const std::vector<std::pair<Point, Point>>& result, unsigned int offset, unsigned int size, unsigned char *isCorrect, unsigned int *errorIndex)
{
	size = std::min<unsigned int>(size, static_cast<unsigned int>(result.size()));

	for (unsigned int i = offset; i < size; i++)
	{
		if (std::find(solution.begin(), solution.end(), result[i]) == solution.end())
		{
			*errorIndex = i;
			*isCorrect = 0;
			return;
		}
	}
}

bool PointValidation::validatePoints(const std::string& solutionFilename, const std::vector<std::pair<Point, Point>>& result)
{
	//reads the solution in
	std::vector<std::pair<Point, Point>> solution = Parser::readSolution(solutionFilename);

	if (result.size() != solution.size())
	{
		std::cout << "The number of point pairs: " << result.size() << " does not match with the number of the solution: " << solution.size() << " ." << std::endl;
		return false;
	}

	unsigned int THREAD_NUMBER = std::thread::hardware_concurrency();

	std::vector<std::thread> threads(THREAD_NUMBER);
	std::vector<unsigned char> isCorrect(THREAD_NUMBER, 1);
	std::vector<unsigned int> errorIndex(THREAD_NUMBER, 1);

	unsigned int pointsPerThread = static_cast<unsigned int>(ceil(result.size() / float(THREAD_NUMBER)));

	for (unsigned int i = 0; i < THREAD_NUMBER; i++)
	{
		threads[i] = std::thread(&validateResults_Thread, solution, result, i * pointsPerThread, (i + 1) * pointsPerThread, &isCorrect[i], &errorIndex[i]);
	}

	for (unsigned int i = 0; i < THREAD_NUMBER; i++)
	{
		threads[i].join();
	}

	for (unsigned int i = 0; i < THREAD_NUMBER; i++)
	{
		if (isCorrect[i] == 0)
		{
			std::cout << "The solution: " << result[errorIndex[i]].first << result[errorIndex[i]].second << " is not contained in the solution." << std::endl;
			return false;
		}
	}

	return true;
}