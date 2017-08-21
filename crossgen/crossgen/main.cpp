#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>

#include "CrosswordGenerator.h"

#define FIELD_FILENAME "..\\..\\input_fields\\field_5.txt"
#define VOCABE_FILENAME "..\\..\\dictionarys\\litw-win-words.txt"

int main() {
	ofstream log("log_voc2.txt", std::ofstream::out | std::ofstream::app);
	
	std::string field(FIELD_FILENAME);
	std::vector<int> calc_times;

	int n_tests = 1;
	for (int i = 0; i < n_tests; ++i) {
		int seed = time(0);
		srand(seed);

		log << "field: " << field << std::endl;
		log << "MIN_WORDS_REMAIN: " << MIN_WORDS_REMAIN << std::endl;
		log << "random seed: " << seed << std::endl;
		std::setlocale(LC_ALL, "rus");
		CrosswordGenerator generator(field, VOCABE_FILENAME, log);
		//log << "generation started..." << std::endl;

		if (!generator.generate()) {
			log << "no solution" << std::endl;
		}
		calc_times.push_back(generator.getGenerationTime());
		log << "field generated:" << std::endl;
		generator.print_generated_field();
		log << std::endl;
		log << std::endl;
	}
	int max_time = 0, min_time = 10000, sum = 0;
	for (int time : calc_times) {
		max_time = std::max(max_time, time);
		min_time = std::min(min_time, time);
		sum += time;
	}
	std::cout << "field: " << field << std::endl;
	std::cout << "n_tests: " << n_tests << std::endl;
	std::cout << "min time: " << min_time << std::endl;
	std::cout << "max time: " << max_time << std::endl;
	std::cout << "mean time: " << sum / (double)calc_times.size() << std::endl;
	system("pause");
	return 0;
}