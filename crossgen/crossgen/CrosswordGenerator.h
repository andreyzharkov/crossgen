#pragma once

//good example of bad-organized code
//but I was too lazy to make it more readable and understandable in the end
//long methods, bad structure, not absolutely clear naming...
//only having written this code I understood how important codestyle is

#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <regex>
#include <clocale>
#include <cassert>
#include <chrono>
#include <iterator>
#include <algorithm>
#include <queue>

#include "CMyGraph.h"

#define MIN_WORDS_REMAIN 8
#define MAX_ITERATION_NUM 100000000
#define BOOST_ALLOWED_SIZE 500
#define BOOST_REPEATS_REQUIRED 50
#define BOOSTED_LENGTH 6
#define BOOSTED_REPEATS_LONG 10

int myrandom(int i) {
	return std::rand() % i;
}

class Masks {
public:
	void setIterators(int length,
		std::vector<std::string>::const_iterator& it_begin,
		std::vector<std::string>::const_iterator& it_end) {
		auto it = masks.find(std::string(length, '.'));
		if (it != masks.end()) {
			it_begin = (*it).second.begin();
			it_end = (*it).second.end();
		}
		else {
			it_begin = masks.begin()->second.end();
			it_end = masks.begin()->second.end();
			//so in base there is no words with such length
		}
	}

	int getCount(int length) {
		return masks[std::string(length, '.')].size();
	}

	const std::vector<std::string>& getWords(int len) {
		return masks[std::string(len, '.')];
	}

	Masks() {}
	~Masks() {
		//log << "masks size: " << masks.size() << std::endl;
	}

	Masks(std::string inputFile) {
		std::ifstream is(inputFile);
		std::string s;
		std::map<int, std::string> baseTemplates;
		for (int i = 1; i < 40; ++i) {
			baseTemplates[i] = std::string(i, '.');
		}
		masks.reserve(300000);
		while (is >> s) {
			masks[baseTemplates[s.length()]].push_back(s);
		}

		for (int i = 1; i < 40; ++i) {
			std::random_shuffle(masks[baseTemplates[i]].begin(), masks[baseTemplates[i]].end(), myrandom);
		}
	}

	int getWords(std::string& parent_mask, std::string& mask,
		std::vector<std::string>::const_iterator& it_begin,
		std::vector<std::string>::const_iterator& it_end) {
		auto it = masks.find(mask);
		if (it != masks.end()) {
			if (it->second.size() == 0) {
				return 0;
			}
			it_begin = (*it).second.begin();
			it_end = (*it).second.end();
			return (*it).second.size();
		}
		else {
			std::regex regexpr(mask);
			auto parent_it = masks.find(parent_mask);
			////if size < ... return parent iterators
			masks[mask].reserve(parent_it->second.size() / 5);
			//assert(parent_it != masks.end());
			std::vector<std::string>& mask_vector = masks[mask];
			for (auto iter = (*parent_it).second.begin(); iter != (*parent_it).second.end(); ++iter) {
				if (std::regex_match(*iter, regexpr)) {
					mask_vector.push_back(*iter);
				}
			}
			if (mask_vector.size() == 0) {
				return 0;
			}
			it_begin = mask_vector.begin();
			it_end = mask_vector.end();
			return mask_vector.size();
		}
	}
private:
	//template -> words satisfyed
	std::unordered_map<std::string, std::vector<std::string>> masks;
};

//minor cheat
Masks refer;

//class describes space in the field for one word
class FieldWord {
public:
	static enum orientation {
		HORIZONTAL,
		VERTICAL
	};

	FieldWord() : masks(refer) {}
	FieldWord(std::pair<int, int> loc, int len, orientation or , Masks& masks) : words_intersect(len, -1),
		value(len, '.'), other_letter_nums(len, -1),
		forbiddenChars(len), masks(masks) {
		selection_values_cnt = 10000;
		masks.setIterators(len, curr_iter, end_iter);
		location = loc;
		length = len;
		orientation_ = or ;
		nLettersFixed = 0;
		priority = 0;
		toRecoil = -1;
		acceleratorLetterNum = -1;
		difficulty = 0;
	}

	void toStart() {
		wasIterated = false;
		nLettersFixed = 0;
		selection_values_cnt = 10000;
		masks.setIterators(length, curr_iter, end_iter);
		value = std::string(length, '.');
	}

	FieldWord& operator=(const FieldWord& arg) {
		selection_values_cnt = 10000;
		words_intersect = arg.words_intersect;
		value = arg.value;
		other_letter_nums = arg.other_letter_nums;
		forbiddenChars = arg.forbiddenChars;
		masks = arg.masks;
		curr_iter = arg.curr_iter;
		end_iter = arg.end_iter;
		location = arg.location;
		length = arg.length;
		orientation_ = arg.orientation_;
		nLettersFixed = arg.nLettersFixed;
		priority = arg.priority;
		toRecoil = arg.toRecoil;
		acceleratorLetterNum = arg.acceleratorLetterNum;
		difficulty = arg.difficulty;
		return *this;
	}

	friend std::ostream& operator<<(std::ostream& os, const FieldWord& obj)
	{
		os << (obj.orientation_ == obj.HORIZONTAL) ? "horizontal" : "vertical";
		os << " (" << obj.location.first << ", " << obj.location.second << ") ";
		os << "length=" << obj.length << "; "
			<< "value=" << obj.value << "; ";
		if (obj.isAccelerator) {
			os << "accelerator";
		}
		return os;
	}

	//used in initialization
	void rememberIntersections(std::vector<std::string>& field, std::vector<FieldWord>& otherWorlds) {
		//assert(orientation_ == orientation.VERTICAL);
		for (int k = 0; k < length; ++k) {
			//check if intersection on this cell exists
			if ((location.second + 1 < field[0].size() && field[location.first + k][location.second + 1] == '1')
				|| (location.second - 1 > 0 && field[location.first + k][location.second - 1] == '1')) {
				for (int i = 0; i < otherWorlds.size(); ++i) {
					if (otherWorlds[i].orientation_ == orientation::VERTICAL)
						break;
					if (otherWorlds[i].location.first == location.first + k) {
						if (otherWorlds[i].location.second <= location.second
							&& otherWorlds[i].location.second + otherWorlds[i].length >= location.second) {
							//assert (last element of other worlds is this)
							other_letter_nums[k] = location.second -
								otherWorlds[i].location.second;
							otherWorlds[i].other_letter_nums[other_letter_nums[k]] = k;

							otherWorlds[i].words_intersect[other_letter_nums[k]] = otherWorlds.size() - 1;
							words_intersect[k] = i;

							difficulty++;
							otherWorlds[i].difficulty++;
						}
					}
				}
			}
		}
	}

	int getLength() {
		return length;
	}

	std::pair<int, int> getLocation() {
		return location;
	}

	orientation getOrientation() {
		return orientation_;
	}

	//returns number of intersections with other words
	int getDifficulty() {
		return difficulty;
	}

	bool hasSolution() {
		return curr_iter != end_iter;
	}

	//used in initialization
	void setPriority(int pr) {
		priority = pr;
	}

	void setToRecoil(int to) {
		toRecoil = to;
	}

	int getToRecoil() {
		return toRecoil;
	}

	int getAcceleratorLetterNum() {
		return acceleratorLetterNum;
	}

	void setAcceleratorLetterNum(int num) {
		acceleratorLetterNum = num;
	}

	void setAccelerator() {
		isAccelerator = true;
	}

	int getSelectionValuesCount() {
		return selection_values_cnt;
	}

	//FieldWord tryes to fill itself
	//returns was this try successful
	bool fillYourself(std::vector<FieldWord>& words, std::unordered_set<std::string>& used,
		std::vector<std::string>& insertedWords) {
		std::regex regexpr(value);

		if (curr_iter == end_iter) {
			//so it's first call, no previous values
			std::string str;
			selection_values_cnt = masks.getWords(str, value, curr_iter, end_iter);
		}
		if (wasIterated) {
			++curr_iter;
		}

		while (curr_iter != end_iter) {
			std::string str(*curr_iter);
			if (std::regex_match(str, regexpr)
				&& used.find(str) == used.end()
				&& stopWords.find(str) == stopWords.end()) {

				if (hasStopLetters) {
					bool satisfyes = true;
					for (int i = 0; i < length; ++i) {
						if (forbiddenChars[i].find(str[i]) != forbiddenChars[i].end()) {
							satisfyes = false;
							break;
						}
					}
					if (!satisfyes) {
						++curr_iter;
						continue;
					}
				}

				if (setValue(words)) {
					used.insert(str);
					insertedWords.push_back(str);
					wasIterated = true;
					return true;
				}
			}
			++curr_iter;
		}
		wasIterated = false;
		return false;
	}

	//recoil this word to the state before word with lastRemovedWordPriority priority was set
	void recoil(int lastRemovedWordPriority, int removeItitiator, std::vector<FieldWord>& otherWorlds) {
		if (priority == lastRemovedWordPriority) {
			if (isAccelerator) {
				int j = otherWorlds[removeItitiator].getAcceleratorLetterNum();
				if (j != -1) {
					assert(value[j] != '.');
					forbiddenChars[j].insert(value[j]);
					hasStopLetters = true;
				}
			}
		}
		else {
			if (hasStopLetters) {
				hasStopLetters = false;
				for (int i = 0; i < length; ++i) {
					forbiddenChars[i].clear();
				}
			}
			if (removeItitiator - lastRemovedWordPriority > 1) {
				curr_iter = end_iter;
			}
			wasIterated = false;
		}

		if (removeItitiator != priority) {
			for (int i = 0; i < length; ++i) {
				if (words_intersect[i] == -1 || otherWorlds[words_intersect[i]].priority >= lastRemovedWordPriority) {
					value[i] = '.';
					//recoil changes in not inited words
					if (words_intersect[i] > priority) {
						otherWorlds[words_intersect[i]].unsetLetter(other_letter_nums[i]);
					}
				}
			}
		}
	}

	void unsetLetter(int at) {
		value[at] = '.';
		nLettersFixed--;
	}

	//tryes to set letter (word should still have possibilitys to be fit after that)
	//returns was trying successful
	bool setLetter(int at, char letter, std::vector<FieldWord>& allWordFields) {
		std::string prev = value;
		value[at] = letter;
		nLettersFixed++;
		std::regex regexpr(value);

		selection_values_cnt = masks.getWords(prev, value, curr_iter, end_iter);

		if ((selection_values_cnt < MIN_WORDS_REMAIN && nLettersFixed < 3
			&& difficulty > nLettersFixed + 2/* && difficulty > 4*/)
			|| (selection_values_cnt == 0)) {
			nLettersFixed--;
			value[at] = '.';
			return false;
		}
		return true;
	}

	//tryes to set value, returns was trying successful
	bool setValue(std::vector<FieldWord>& allwordfields) {
		std::string prev = value;
		value = *curr_iter;
		int fail_i = -1;
		for (int i = 0; i < length; ++i) {
			if (words_intersect[i] != -1 && priority < allwordfields[words_intersect[i]].priority) {
				if (!allwordfields[words_intersect[i]]
					.setLetter(other_letter_nums[i], value[i], allwordfields)) {
					fail_i = i;
					forbiddenChars[i].insert(value[i]);
					hasStopLetters = true;
					break;
				}
			}
		}
		if (fail_i == -1) return true;
		for (int i = 0; i < fail_i; ++i) {
			if (words_intersect[i] != -1 && priority < allwordfields[words_intersect[i]].priority) {
				allwordfields[words_intersect[i]].unsetLetter(other_letter_nums[i]);
			}
		}
		value = prev;
		return false;
	}

	void setStopWords(std::string val, const std::vector<std::string>& allPossibleValues) {
		std::regex regexpr(val);

		for (auto it = allPossibleValues.begin(); it != allPossibleValues.end(); ++it) {
			if (!std::regex_match(*it, regexpr)) {
				stopWords.insert(*it);
			}
		}
	}

	void updateWordsIntersections(std::map<int, int>& new_ids) {
		if (toRecoil != -1) toRecoil = new_ids[toRecoil];
		for (int i = 0; i < length; ++i) {
			if (words_intersect[i] != -1) {
				words_intersect[i] = new_ids[words_intersect[i]];
			}
		}
	}

	std::vector<int> getIntersectorsIds() {
		return words_intersect;
	}

	std::vector<int> getOtherLetterNums() {
		return other_letter_nums;
	}

	std::string getValue() {
		return value;
	}
private:
	Masks& masks;

	std::pair<int, int> location;
	int length;
	int difficulty;

	//number of words, from which value can be selected
	//it is initial difference between end_iter and current_iter
	int selection_values_cnt;

	//words that couldn't be set in that field at all
	std::unordered_set<std::string> stopWords;

	//from 0 to allwordfields.size() - 1 - moment when word in this position is selected
	int priority;
	int toRecoil;

	//accelerator traits
	bool hasStopLetters = false;
	bool isAccelerator = false;
	int acceleratorLetterNum;
	std::vector<std::set<char>> forbiddenChars;

	std::string value;
	orientation orientation_;

	//words_intersect[i] - index of FieldWord, which intersect this FieldWord in position i
	std::vector<int> words_intersect;
	//other_letter_nums[i] - number of letter (of intersected word) same as letter i
	std::vector<int> other_letter_nums;

	//template for matching
	//std::string word_template;

	//iterators on strings, that can be set as this->value
	bool wasIterated = false;
	std::vector<std::string>::const_iterator curr_iter;
	std::vector<std::string>::const_iterator end_iter;

	//possible_values[i] - set of possible values, when i letters of this FieldWord are set
	//std::vector<std::vector<std::set<std::string>::iterator>> possible_values;
	int nLettersFixed;
};

class CrosswordGenerator {
public:
	CrosswordGenerator(std::string fieldFile, std::string vocabeFile, ofstream& log) : masks(vocabeFile), log(log) {
		std::chrono::time_point<std::chrono::system_clock> start, end;
		int elapsed_time;

		std::string s;
		std::ifstream in(fieldFile);
		while (in >> s) {
			field.push_back(s);
		}

		start = std::chrono::system_clock::now();
		std::vector<std::pair<int, int>> horisontal_starts;
		std::vector<int> horizontal_lengths;
		std::vector<std::pair<int, int>> vertical_starts;
		std::vector<int> vertical_lengths;

		for (int i = 0; i < field.size(); ++i) {
			for (int j = 0; j < field[0].size(); ++j) {
				if (field[i][j] == '1' && (i == 0 || field[i - 1][j] == '0')
					&& (i != field.size() - 1 && field[i + 1][j] == '1')) {
					//this is start of vertical word
					vertical_starts.push_back(std::make_pair(i, j));
					//calc length
					vertical_lengths.push_back(1);
					for (int k = 1; i + k < field.size(); ++k) {
						if (field[i + k][j] == '1') {
							vertical_lengths.back()++;
						}
						else {
							break;
						}
					}
				}
				if (field[i][j] == '1' && (j == 0 || field[i][j - 1] == '0')
					&& j != field[0].size() && field[i][j + 1] == '1') {
					//this is start of horizontal word
					horisontal_starts.push_back(std::make_pair(i, j));
					//calc length
					horizontal_lengths.push_back(1);
					for (int k = 1; j + k < field[0].size(); ++k) {
						if (field[i][j + k] == '1') {
							horizontal_lengths.back()++;
						}
						else {
							break;
						}
					}
				}
			}
		}
		//cover intersections
		for (int i = 0; i < horisontal_starts.size(); ++i) {
			words.emplace_back(horisontal_starts[i], horizontal_lengths[i], FieldWord::orientation::HORIZONTAL, masks);
		}
		for (int i = 0; i < vertical_starts.size(); ++i) {
			words.emplace_back(vertical_starts[i], vertical_lengths[i], FieldWord::orientation::VERTICAL, masks);
			words.back().rememberIntersections(field, words);
		}

		end = std::chrono::system_clock::now();
		elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds> (start - end).count();
		//log << "basic init: " << elapsed_time << "ms" << std::endl;

		//check if in vocabe are words with such lengths
		for (int i = 0; i < words.size(); ++i) {
			if (!words[i].hasSolution()) {
				hasSolution = false;
				return;
			}
		}

		start = std::chrono::system_clock::now();
		make_order_optimal();
		end = std::chrono::system_clock::now();
		elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds> (start - end).count();
		//log << "reordering: " << elapsed_time << "ms" << std::endl;

		end = std::chrono::system_clock::now();
		setStopWords();
		end = std::chrono::system_clock::now();
		elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds> (start - end).count();
		//log << "setting templates: " << elapsed_time << "ms" << std::endl;
	}

	bool generate() {
		if (!hasSolution) {
			return false;
		}
		//print();

		for (int i = 0; i < words.size(); ++i) {
			words[i].toStart();
		}

		trying_iterations = 0;
		fail_iter_counter = 0;
		max_depth = 0;
		updated_min = 1000;
		used.clear();
		insertedWords.clear();
		//how many recoils to this priority were
		std::vector<int> returns(words.size(), 0);

		std::vector<int> counted_number(words.size(), 0);
		int priority = 0;
		std::chrono::time_point<std::chrono::system_clock> start, end;
		int elapsed_time;

		start = std::chrono::system_clock::now();
		int n_firsts = 0;
		int n_boosts = 0;
		while (priority < words.size()) {
			if (priority == 0) {
				if (words[priority].fillYourself(words, used, insertedWords)) {
					if (fail_iter_counter > 0) {
						log << fail_iter_counter << " iterations has passed" << std::endl;
						log << "max depth reached: " << max_depth << std::endl;
						log << "max depth changed: " << updated_min << std::endl;
						log << "returning to 1st word selection" << std::endl;
						fail_iter_counter = 0;
						max_depth = 0;
						updated_min = 1000;
					}
					if (n_firsts == 1) {
						end = std::chrono::system_clock::now();
						elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>
							(end - start).count() / 1000.0 * masks.getCount(words[0].getLength());
						log << "estimated calculation time: " << elapsed_time << " seconds ("
							<< elapsed_time / 3600.0 << " hours)" << std::endl;
					}
					fail_iter_counter = 0;
					max_depth = 0;
					updated_min = 1000;
					returns = std::vector<int>(returns.size(), 0);
					log << "first word chosen: " << words[0].getValue() << std::endl;
					n_firsts++;
					priority++;
				}
				else {
					return false;
				}
			}
			//for other prioritys
			else {
				if (priority > max_depth) {
					max_depth = priority;
				}
				if (priority < updated_min) {
					updated_min = priority;
				}
				//if limit has come reselect first word
				if (++fail_iter_counter >= MAX_ITERATION_NUM) {
					int pr = 0;
					for (int i = pr; i < priority; ++i) {
						used.erase(insertedWords.back());
						insertedWords.pop_back();
						words[i].recoil(pr, priority, words);
					}
					words[priority].recoil(pr, priority, words);
					priority = pr;
					end = std::chrono::system_clock::now();
					elapsed_time = std::chrono::duration_cast<std::chrono::seconds>
						(end - start).count();
					log << "time passed: " << elapsed_time << " seconds" << std::endl;
					continue;
				}
				//else try to recoil and continue
				if (words[priority].fillYourself(words, used, insertedWords)) {
					priority++;
				}
				else {
					int pr = words[priority].getToRecoil();

					//try to boost selection
					while ((++returns[pr] >= BOOST_REPEATS_REQUIRED &&
						words[pr].getSelectionValuesCount() > BOOST_ALLOWED_SIZE)
						|| (words[pr].getLength() > BOOSTED_LENGTH
							&& returns[pr] >= std::max(BOOSTED_REPEATS_LONG,
								words[pr].getSelectionValuesCount() / 10))) {
						pr = words[pr].getToRecoil();
						//log << "BOOST! from " << priority << " to " << pr << std::endl;
						for (int k = pr + 1; k < returns.size(); ++k) {
							returns[k] = 0;
						}
						++n_boosts;
					}

					for (int i = pr; i < priority; ++i) {
						used.erase(insertedWords.back());
						insertedWords.pop_back();
						words[i].recoil(pr, priority, words);
					}
					words[priority].recoil(pr, priority, words);
					priority = pr;
				}
			}
		}
		log << "max depth reached: " << max_depth << std::endl;
		log << "max depth changed: " << updated_min << std::endl;
		log << "iterations passed: " << fail_iter_counter << std::endl;
		log << "number of boosts: " << n_boosts << std::endl;
		max_depth = 0;
		updated_min = 1000;

		end = std::chrono::system_clock::now();
		elapsed_time = std::chrono::duration_cast<std::chrono::seconds>
			(end - start).count();
		log << "calculation time: " << elapsed_time << "seconds" << std::endl;
		generation_time = elapsed_time;
		/*make_field_pretty();
		print();*/
		/*log << "returns times:" << std::endl;
		for (int i = 0; i < returns.size(); ++i) {
		log << i << ": " << returns[i] << std::endl;
		}*/
		return true;
	}

	void print_generated_field() {
		make_field_pretty();
		for (int i = 0; i < words.size(); ++i) {
			log << i << ": " << words[i] << std::endl;
		}
		log << "field was:" << std::endl;
		for (int i = 0; i < field.size(); ++i) {
			log << field[i] << std::endl;
		}
	}

	static void setCoeff(int val) {
		coeff = val;
	}
	int getGenerationTime() {
		return generation_time;
	}
private:
	int max_depth, updated_min, fail_iter_counter;
	int trying_iterations;
	static int coeff;
	int generation_time;

	//debug log
	ofstream& log;

	//may be determined in analysys stage
	bool hasSolution = true;

	//crossword matrix
	std::vector<std::string> field;
	//all words in matrix
	std::vector<FieldWord> words;

	Masks masks;

	//words which are currently added
	std::vector<std::string> insertedWords;
	//same as insertedWords, needed to faster matching
	std::unordered_set<std::string> used;

	//change order in words: at the beginning are words locate which is harder
	void make_order_optimal() {
		//////1) greedy reordering
		// the longest word comes first
		// then neighbour of the clicue with the highest difficulty
		std::set<int> FieldWordIdsAdded;
		std::set<int> neighbours;

		struct comparator1 {
			bool operator()(std::pair<int, int> a, std::pair<int, int> b)
			{
				return a.first < b.first;
			}
		};
		std::priority_queue<std::pair<int, int>, std::vector<std::pair<int, int>>, comparator1> neighboursDifficulties;
		std::map<int, int> oldToNewId;
		std::vector<FieldWord> reordered;

		struct {
			bool operator()(FieldWord a, FieldWord b)
			{
				return a.getLength() + a.getDifficulty() < b.getLength() + b.getDifficulty();
			}
		} comparator;
		auto id = std::max_element(words.begin(), words.end(), comparator) - words.begin();
		for (int j = 0; j < words.size(); ++j) {
			FieldWordIdsAdded.insert(id);
			reordered.push_back(words[id]);
			neighbours.erase(id);
			oldToNewId[id] = j;
			std::vector<int> intersectors_ids = words[id].getIntersectorsIds();
			for (int i = 0; i < words[id].getLength(); ++i) {
				if (intersectors_ids[i] != -1 &&
					FieldWordIdsAdded.find(intersectors_ids[i]) == FieldWordIdsAdded.end()) {
					neighbours.insert(intersectors_ids[i]);
					neighboursDifficulties.push(std::make_pair(words[intersectors_ids[i]].getDifficulty()
						+ words[intersectors_ids[i]].getLength(), intersectors_ids[i]));
				}
			}
			if (neighbours.size() > 0) {
				while (FieldWordIdsAdded.find(id) != FieldWordIdsAdded.end()) {
					id = neighboursDifficulties.top().second;
					neighboursDifficulties.pop();
				}
			}
		}

		words = reordered;

		assert(oldToNewId.size() == words.size());
		for (int i = 0; i < words.size(); ++i) {
			words[i].updateWordsIntersections(oldToNewId);
			words[i].setPriority(i);
		}

		//here is another reordering, estimated for ordering words in fragments one by one
		std::vector<int> victim;
		for (int i = 0; i < words.size(); ++i) {
			victim.push_back(i);
		}
		std::vector<int> optimalOrder = findFragments(victim);
		std::map<int, int> new_ids;
		for (int i = 0; i < optimalOrder.size(); ++i) {
			new_ids[optimalOrder[i]] = i;
		}
		std::vector<FieldWord> optimal;
		for (int i = 0; i < optimalOrder.size(); ++i) {
			optimal.push_back(words[optimalOrder[i]]);
			optimal.back().updateWordsIntersections(new_ids);
			optimal.back().setPriority(i);
			if (optimal.back().getToRecoil() == -1) {
				optimal.back().setToRecoil(i - 1);
			}
		}
		words = optimal;
	}

	//returns reordered vector of vertexes (which was given as argument) ids in words classfield
	std::vector<int> findFragments(std::vector<int> vertexes) {
		// - > {7, 5, 4, ... } v[i-1] > v[i]
		// so FieldWords with more priority comes last
		std::sort(vertexes.begin(), vertexes.end(), std::isgreater<int, int>);

		CMyGraph<int, int> graph;
		for (int i = 0; i < vertexes.size(); ++i) {
			graph.AddVertex(vertexes[i]);
		}
		for (int i = 0; i < vertexes.size(); ++i) {
			std::vector<int> intersectors = words[vertexes[i]].getIntersectorsIds();
			for (int j = 0; j < intersectors.size(); ++j) {
				if (intersectors[j] != -1 && std::find(vertexes.begin(), vertexes.end(), intersectors[j]) != vertexes.end()) {
					graph.AddEdge(vertexes[i], intersectors[j], 1);
				}
			}
		}

		std::vector<int> optimalOrder;
		while (graph.size() > 0) {
			optimalOrder.push_back(vertexes.back());
			vertexes.pop_back();
			graph.DeleteVertex(graph.FindVertex(optimalOrder.back()));

			std::map<int, int> vert_map = graph.Kosarayu();
			std::map<int, std::vector<int>> components;
			for (std::pair<int, int> p : vert_map) {
				if (components.find(p.second) == components.end()) {
					components[p.second] = std::vector<int>();
				}
				components[p.second].push_back(p.first);
			}
			if (components.size() > 1) {
				int crackWordId = optimalOrder.back();
				for (std::pair<int, std::vector<int>> p : components) {
					std::vector<int> fragment = (p.second.size() > 2) ? findFragments(p.second) : p.second;
					int n_intersections = 0;
					int letter_intersected;
					for (int id : fragment) {
						optimalOrder.push_back(id);
						std::vector<int> intersection_ids = words[id].getIntersectorsIds();
						for (int i = 0; i < intersection_ids.size(); ++i) {
							if (intersection_ids[i] == crackWordId) {
								n_intersections++;
								letter_intersected = words[id].getOtherLetterNums()[i];
								break;
							}
						}
					}

					words[fragment[0]].setToRecoil(crackWordId);
					if (n_intersections == 1) {
						words[fragment[0]].setAcceleratorLetterNum(letter_intersected);
						words[crackWordId].setAccelerator();
					}
				}
				break;
			}
		}
		return optimalOrder;
	}

	//generate templates for words with such length
	void setStopWords() {
		std::vector<std::vector<std::set<char>>> baseTemplates;

		// fit base templetes
		for (int i = 0; i < 40; ++i) {
			const std::vector<std::string>& equalLength = masks.getWords(i);

			if (equalLength.size() == 0) {
				baseTemplates.push_back(std::vector<std::set<char>>());
			}
			else {
				int len = equalLength.begin()->size();
				std::vector<std::set<char>> positions(len);
				for (auto str : equalLength) {
					for (int j = 0; j < len; ++j) {
						positions[j].insert(str[j]);
					}
				}
				baseTemplates.push_back(positions);
			}
		}

		// calculate real templates
		for (int i = 0; i < words.size(); ++i) {
			std::vector<int> intersection_ids = words[i].getIntersectorsIds();
			std::vector<int> other_letter_nums = words[i].getOtherLetterNums();

			std::string str_template;
			for (int j = 0; j < words[i].getLength(); ++j) {
				std::vector<char> curr_letter_possible_chars;

				if (intersection_ids[j] == -1) {
					if (baseTemplates[words[i].getLength()].size()) {
						curr_letter_possible_chars = std::vector<char>(baseTemplates[words[i].getLength()][j].begin(),
							baseTemplates[words[i].getLength()][j].end());
					}
				}
				else {
					std::set_intersection(baseTemplates[words[i].getLength()][j].begin(),
						baseTemplates[words[i].getLength()][j].end(),
						baseTemplates[words[intersection_ids[j]].getLength()][other_letter_nums[j]].begin(),
						baseTemplates[words[intersection_ids[j]].getLength()][other_letter_nums[j]].end(),
						std::back_inserter(curr_letter_possible_chars));
				}

				if (curr_letter_possible_chars.size() == 0) {
					hasSolution = false;
					return;
				}
				std::string s;
				for (char c : curr_letter_possible_chars) {
					s += c;
				}
				str_template += "[" + s + "]";
			}
			words[i].setStopWords(str_template, masks.getWords(words[i].getLength()));
		}
	}

	//to debug
	void make_field_order_clear(int to) {
		for (int i = 0; i < to; ++i) {
			for (int j = 0; j < words[i].getLength(); ++j) {
				if (words[i].getOrientation() == FieldWord::orientation::HORIZONTAL) {
					field[words[i].getLocation().first][words[i].getLocation().second + j] = ' ';
				}
				else {
					field[words[i].getLocation().first + j][words[i].getLocation().second] = ' ';
				}
			}
		}
		for (int i = to; i < words.size(); ++i) {
			for (int j = 0; j < words[i].getLength(); ++j) {
				if (words[i].getOrientation() == FieldWord::orientation::HORIZONTAL) {
					field[words[i].getLocation().first][words[i].getLocation().second + j] = '1';
				}
				else {
					field[words[i].getLocation().first + j][words[i].getLocation().second] = '1';
				}
			}
		}
		for (int i = 0; i < field.size(); ++i) {
			for (int j = 0; j < field[0].size(); ++j) {
				if (field[i][j] == '0') {
					field[i][j] = ' ';
				}
			}
		}
	}

	//fill field with words
	void make_field_pretty() {
		for (int i = 0; i < words.size(); ++i) {
			for (int j = 0; j < words[i].getLength(); ++j) {
				if (words[i].getOrientation() == FieldWord::orientation::HORIZONTAL) {
					field[words[i].getLocation().first][words[i].getLocation().second + j] = words[i].getValue()[j];
				}
				else {
					field[words[i].getLocation().first + j][words[i].getLocation().second] = words[i].getValue()[j];
				}
			}
		}
		for (int i = 0; i < field.size(); ++i) {
			for (int j = 0; j < field[0].size(); ++j) {
				if (field[i][j] == '0') {
					field[i][j] = ' ';
				}
			}
		}
	}
};