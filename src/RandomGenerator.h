#ifndef SRC_RANDOMGENERATOR_H_
#define SRC_RANDOMGENERATOR_H_

#include <random>

class RandomGenerator
{

public:
	std::mt19937 rng;

	int getUniformInt(int thru);
	double get01();
	double getGaussian(double mean, double sd);

	RandomGenerator() : rng(std::random_device()()) {}

	inline std::mt19937 getRNG() { return rng; };
};

#endif /* SRC_RANDOMGENERATOR_H_ */

