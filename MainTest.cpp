// Copyright Mass Media. All rights reserved. DO NOT redistribute.

////////////////////////////////////////////////////////////////////////////////////////////////////
// Task List
////////////////////////////////////////////////////////////////////////////////////////////////////
// Notes
//	* This test requires a compiler with C++17 support and was built for Visual Studio 2017.
// 		* Tested on Linux (Ubuntu 20.04) with: g++ -Wall -Wextra -pthread -std=c++17 MainTest.cpp
//		* Tested on Mac OS Big Sur, 11.0.1 and latest XCode updates.
//	* Correct output for all three sorts is in the CorrectOutput folder. BeyondCompare is recommended for comparing output.
//	* Functions, classes, and algorithms can be added and changed as needed.
//	* DO NOT use std::sort().
// Objectives
//	* 20 points - Make the program produce a SingleAscending.txt file that matches CorrectOutput/SingleAscending.txt. DONE
//	* 10 points - Make the program produce a SingleDescending.txt file that matches CorrectOutput/SingleDescending.txt. DONE
//	* 10 points - Make the program produce a SingleLastLetter.txt file that matches CorrectOutput/SingleLastLetter.txt. DONE
//	* 20 points - Write a brief report on what you found, what you did, and what other changes to the code you'd recommend. DONE
//	* 10 points - Make the program produce three MultiXXX.txt files that match the equivalent files in CorrectOutput; it must be multi-threaded. //DONE
//	* 20 points - Improve performance as much as possible on both single-threaded and multi-threaded versions; speed is more important than memory usage.
//	* 10 points - Improve safety and stability; fix memory leaks and handle unexpected input and edge cases.
////////////////////////////////////////////////////////////////////////////////////////////////////

#include <string>
#include <iostream>
#include <fstream>
#include <thread>
#include <atomic>
#include <ctime>
#include <vector>
#include <mutex>
#include <sstream>

#ifndef INCLUDE_STD_FILESYSTEM_EXPERIMENTAL
#   if defined(__cpp_lib_filesystem)
#       define INCLUDE_STD_FILESYSTEM_EXPERIMENTAL 0
#   elif defined(__cpp_lib_experimental_filesystem)
#       define INCLUDE_STD_FILESYSTEM_EXPERIMENTAL 1
#   elif !defined(__has_include)
#       define INCLUDE_STD_FILESYSTEM_EXPERIMENTAL 1
#   elif __has_include(<filesystem>)
#       ifdef _MSC_VER
#           if __has_include(<yvals_core.h>)
#               include <yvals_core.h>
#               if defined(_HAS_CXX17) && _HAS_CXX17
#                   define INCLUDE_STD_FILESYSTEM_EXPERIMENTAL 0
#               endif
#           endif
#           ifndef INCLUDE_STD_FILESYSTEM_EXPERIMENTAL
#               define INCLUDE_STD_FILESYSTEM_EXPERIMENTAL 1
#           endif
#       else
#           define INCLUDE_STD_FILESYSTEM_EXPERIMENTAL 0
#       endif
#   elif __has_include(<experimental/filesystem>)
#       define INCLUDE_STD_FILESYSTEM_EXPERIMENTAL 1
#   else
#       error Could not find system header "<filesystem>" or "<experimental/filesystem>"
#   endif
#   if INCLUDE_STD_FILESYSTEM_EXPERIMENTAL
#       include <experimental/filesystem>
     	namespace fs = std::experimental::filesystem;
#   else
#       include <filesystem>
#		if __APPLE__
			namespace fs = std::__fs::filesystem;
#		else
			namespace fs = std::filesystem;
#		endif
#   endif
#endif

using namespace std;

////////////////////////////////////////////////////////////////////////////////////////////////////
// Definitions and Declarations
////////////////////////////////////////////////////////////////////////////////////////////////////
#define MULTITHREADED_ENABLED 0
#define OPTIMIZED_FILE_READ 1

mutex g_lock;


enum class ESortType { AlphabeticalAscending, AlphabeticalDescending, LastLetterAscending };

class IStringComparer {
public:
	virtual bool Sort(string &_first, string &_second) = 0;
};

class AlphabeticalAscendingStringComparer : public IStringComparer {
public:
	virtual bool Sort(string &_first, string &_second);
};

class AlphabeticalDescendingStringComparer : public IStringComparer {
public:
	virtual bool Sort(string &_first, string &_second);
};

class AlphabeticalDescendingLastCharComparer : public IStringComparer {
public:
	virtual bool Sort(string &_first, string &_second);
};

void DoSingleThreaded(vector<string> &_fileList, ESortType _sortType, string _outputName);
void DoMultiThreaded(vector<string> &_fileList, ESortType _sortType, string _outputName);
vector<string> ReadFile(string &_fileName);
void ThreadedReadFile(string _fileName, vector<string>* _listOut);
vector<string> BubbleSort(vector<string> _listToSort, ESortType _sortType);
void WriteAndPrintResults(const vector<string>& _masterStringList, string _outputName, int _clocksTaken);
vector<string> iterativeMergeSort(vector<string>& a, ESortType _sortType);

////////////////////////////////////////////////////////////////////////////////////////////////////
// Main
////////////////////////////////////////////////////////////////////////////////////////////////////
int main() {
	ios_base::sync_with_stdio(0);
	cin.tie(NULL);
	cout.tie(NULL);

	// Enumerate the directory for input files
	vector<string> fileList;
    string inputDirectoryPath = "../InputFiles";
    for (const auto & entry : fs::directory_iterator(inputDirectoryPath)) {
		if (!fs::is_directory(entry)) {
			fileList.push_back(entry.path().string());
		}
	}

	// Do the stuff
	//DoSingleThreaded(fileList, ESortType::AlphabeticalAscending,	"SingleAscending");
	//DoSingleThreaded(fileList, ESortType::AlphabeticalDescending,	"SingleDescending");
	DoSingleThreaded(fileList, ESortType::LastLetterAscending,		"SingleLastLetter");
#if MULTITHREADED_ENABLED
	//DoMultiThreaded(fileList, ESortType::AlphabeticalAscending,		"MultiAscending");
	//DoMultiThreaded(fileList, ESortType::AlphabeticalDescending,	"MultiDescending");
	DoMultiThreaded(fileList, ESortType::LastLetterAscending,		"MultiLastLetter");
#endif

	// Wait
	cout << endl << "Finished...";
	getchar();
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// The Stuff
////////////////////////////////////////////////////////////////////////////////////////////////////
void DoSingleThreaded(vector<string> &_fileList, ESortType _sortType, string _outputName) {
	clock_t startTime = clock();
	vector<string> masterStringList;
	for (unsigned int i = 0; i < _fileList.size(); ++i) {
		vector<string> fileStringList = ReadFile(_fileList[i]);
		//cout << _fileList[i] << endl;
		for (unsigned int j = 0; j < fileStringList.size(); ++j) {
			masterStringList.push_back(fileStringList[j]);
		}		
	}
	
	//masterStringList = BubbleSort(masterStringList, _sortType);
	masterStringList = iterativeMergeSort(masterStringList, _sortType);
	//_fileList.clear();
	clock_t endTime = clock();
	WriteAndPrintResults(masterStringList, _outputName, endTime - startTime);
}

void DoMultiThreaded(vector<string> &_fileList, ESortType _sortType, string _outputName) {
	clock_t startTime = clock();
	vector<string> masterStringList;
	vector<thread> workerThreads;
	for (unsigned int i = 0; i < _fileList.size(); ++i) {
		vector<string> temp;
		workerThreads.push_back(thread(ThreadedReadFile, _fileList[i], &masterStringList));
	}
	for (int i = 0; i < _fileList.size(); i++) 
		workerThreads[i].join();
	

	
	//masterStringList = BubbleSort(masterStringList, _sortType);
	masterStringList = iterativeMergeSort(masterStringList, _sortType);
	clock_t endTime = clock();

	WriteAndPrintResults(masterStringList, _outputName, endTime - startTime);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// File Processing
////////////////////////////////////////////////////////////////////////////////////////////////////


vector<string> ReadFile(string &_fileName) {
#if OPTIMIZED_FILE_READ
	

	ifstream f(_fileName, ios::in | ios::binary);

	// Obtain the size of the file.
	const unsigned sz = fs::file_size(_fileName);

	// Create a buffer.
	string result(sz, '\0');

	// Read the whole file into the buffer.
	f.read(&result[0], sz);

	//cout << result << endl;
	int countofNewLine = 1;
	for (int i = 0; i < result.size(); i++) {
		if (result[i] == '\n') countofNewLine++;
		
	}
	//cout << countofNewLine << endl;
	vector<string> ans(countofNewLine);
	string a = "";
	int j = 0;
	for (int i = 0; i < result.size(); i++) {
		#ifdef __unix__                    /* __unix__ is usually defined by compilers targeting Unix systems */
			if (result[i] == '\n') {
				ans[j++] = a;
				a = "";
				a.clear();
			}
			else {
				a += result[i];
			}

		#elif defined(_WIN32) || defined(WIN32)     /* _Win32 is usually defined by compilers targeting 32 or   64 bit Windows systems */
			if (result[i] == '\r') {
				ans[j++] = a;
				i++;
				a = "";
				a.clear();
			}
			else {
				a += result[i];
			}

		#endif
		
	}
	ans[j++] = a;
	f.close();
	
	//for (int i = 0; i < ans.size(); i++) cout << ans[i]<<" "<<i<<endl;

	return ans;
#else
	vector<string> listOut;
	streampos positionInFile = 0;
	bool endOfFile = false;
	while (!endOfFile) {
		ifstream fileIn(_fileName, ifstream::in);
		fileIn.seekg(positionInFile, ios::beg);

		string* tempString = new string();
		getline(fileIn, *tempString);

		endOfFile = fileIn.peek() == EOF;
		positionInFile = endOfFile ? ios::beg : fileIn.tellg();
		listOut.push_back(*tempString);
		if (endOfFile) {
			fileIn.close();
			break;
		}
}
	return listOut;
#endif
	
}

void ThreadedReadFile(string _fileName, vector<string>* _listOut) {
	vector<string> temp = ReadFile(_fileName);
	g_lock.lock();
	(*_listOut).insert((*_listOut).end(), temp.begin(), temp.end());
	g_lock.unlock();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Sorting
////////////////////////////////////////////////////////////////////////////////////////////////////
bool AlphabeticalAscendingStringComparer::Sort(string &_first, string &_second) {
	unsigned int i = 0;
	while (i < _first.length() && i < _second.length()) {
		if (_first[i] < _second[i])
			return true;
		else if (_first[i] > _second[i])
			return false;
		++i;
	}
	return (i == _first.length());
}

bool AlphabeticalDescendingStringComparer::Sort(string &_first, string &_second) {
	unsigned int i = 0;
	while (i < _first.length() && i < _second.length()) {
		if (_first[i] < _second[i])
			return false;
		else if (_first[i] > _second[i])
			return true;
		++i;
	}
	return (i == _second.length());
}

bool AlphabeticalDescendingLastCharComparer::Sort(string &_first, string &_second) {
	unsigned int i = 0;
	while (i < _first.length() && i < _second.length()) {
		if (_first[_first.size()-i-1] < _second[_second.size() - i - 1])
			return true;
		else if (_first[_first.size() - i - 1] > _second[_second.size() - i - 1])
			return false;
		++i;
	}
	return (i == _second.length());
}

bool commonSort(string &a, string &b, ESortType _sortType) {
	if (_sortType == ESortType::AlphabeticalAscending) {
		AlphabeticalAscendingStringComparer* stringSorter = new AlphabeticalAscendingStringComparer();
		return stringSorter->Sort(a, b);
	}
	else if (_sortType == ESortType::AlphabeticalDescending) {
		AlphabeticalDescendingStringComparer* stringSorter = new AlphabeticalDescendingStringComparer();
		return stringSorter->Sort(a, b);
	}
	else if (_sortType == ESortType::LastLetterAscending) {
		AlphabeticalDescendingLastCharComparer* stringSorter = new AlphabeticalDescendingLastCharComparer();
		return stringSorter->Sort(a, b);
	}
}

vector<string> merge(vector<string>& a, int l1, int r1, int l2, int r2, ESortType _sortType) {
	vector<string> temp(a.size());
	int index = 0;
	while (l1 <= r1 && l2 <= r2) {
		if (commonSort(a[l1], a[l2], _sortType)) {
			temp[index] = a[l1];
			++index;
			++l1;
		}
		else {
			temp[index] = a[l2];
			++index;
			++l2;
		}
	}
	while (l1 <= r1) {
		temp[index] = a[l1];
		++index;
		++l1;
	}
	while (l2 <= r2) {
		temp[index] = a[l2];
		++index;
		++l2;
	}
	return temp;
}


vector<string> iterativeMergeSort(vector<string>& a, ESortType _sortType) {
	int len = 1;
	while (len < a.size()) {
		int i = 0;
		while (i < a.size()) {
			int l1 = i;
			int r1 = i + len - 1;
			int l2 = i + len;
			int r2 = i + 2 * len - 1;
			if (l2 >= a.size()) {
				break;
			}
			if (r2 >= a.size()) {
				r2 = a.size() - 1;
			}
			vector<string> temp = merge(a, l1, r1, l2, r2, _sortType);
			for (int j = 0; j <= r2 - l1 ; j++) {
				a[i + j] = temp[j];
			}
			i = i + 2 * len;

		}
		len *= 2;
	}
	return a;
}
vector<string> BubbleSort(vector<string> _listToSort, ESortType _sortType) {
	
	
	vector<string> sortedList = _listToSort;
	for (unsigned int i = 0; i < sortedList.size() - 1; ++i) {
		for (unsigned int j = 0; j < sortedList.size() - 1; ++j) {
			if (!commonSort(sortedList[j], sortedList[j+1], _sortType)) {
				string tempString = sortedList[j];
				sortedList[j] = sortedList[j+1];
				sortedList[j+1] = tempString;
			}
		}
	}
	return sortedList; 
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Output
////////////////////////////////////////////////////////////////////////////////////////////////////
void WriteAndPrintResults(const vector<string>& _masterStringList, string _outputName, int _clocksTaken) {
	cout << endl << _outputName << "\t- Clocks Taken: " << _clocksTaken << endl;
	
	ofstream fileOut(_outputName + ".txt", ofstream::trunc);
	string output = "";
	for (unsigned int i = 0; i < _masterStringList.size(); ++i) {
		//fileOut << _masterStringList[i]<<endl;
		output += _masterStringList[i] + '\n';
	}
	fileOut.write(output.data(), output.size());
	fileOut.close();
}
