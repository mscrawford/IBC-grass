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
//	RandomGenerator() : rng(1) {}
};

#endif /* SRC_RANDOMGENERATOR_H_ */
