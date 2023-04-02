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
//	* 10 points - Make the program produce three MultiXXX.txt files that match the equivalent files in CorrectOutput; it must be multi-threaded. DONE
//	* 20 points - Improve performance as much as possible on both single-threaded and multi-threaded versions; speed is more important than memory usage. DONE
//	* 10 points - Improve safety and stability; fix memory leaks and handle unexpected input and edge cases. DONE
////////////////////////////////////////////////////////////////////////////////////////////////////
#define _CRTDBG_MAP_ALLOC

#include <string>
#include <iostream>
#include <fstream>
#include <thread>
#include <atomic>
#include <ctime>
#include <vector>
#include <mutex>
#include <sstream>
#include <crtdbg.h>



#ifdef _DEBUG
	#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
	// Replace _NORMAL_BLOCK with _CLIENT_BLOCK if you want the
	// allocations to be of _CLIENT_BLOCK type
	#define USE_VS_CRTDBG 1
#else
	#define DBG_NEW new
	
#endif


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
#define MULTITHREADED_ENABLED 1
#define OPTIMIZED_FILE_READ 1
#define USE_MAGIC_NUMBER 0
#define USE_VS_CRTDBG 0


struct alignas(64) padded_vector {
	vector<string> value;
	padded_vector() { value = vector<string>(); }
};

enum class ESortType { AlphabeticalAscending, AlphabeticalDescending, LastLetterAscending };

class IStringComparer {
public:
	virtual bool Sort(string &_first, string &_second) = 0;

	~IStringComparer(){}
};

class AlphabeticalAscendingStringComparer : public IStringComparer {
public:
	virtual bool Sort(string &_first, string &_second);

	~AlphabeticalAscendingStringComparer() {
		
	}
};

class AlphabeticalDescendingStringComparer : public IStringComparer {
public:
	virtual bool Sort(string &_first, string &_second);

	~AlphabeticalDescendingStringComparer() {

	}
};

class AlphabeticalDescendingLastCharComparer : public IStringComparer {
public:
	virtual bool Sort(string &_first, string &_second);

	~AlphabeticalDescendingLastCharComparer() {

	}
};

void DoSingleThreaded(vector<string> &_fileList, ESortType _sortType, string _outputName);
void DoMultiThreaded(vector<string> &_fileList, ESortType _sortType, string _outputName);
void ReadFile(string &_fileName, vector<string> &ans);
void ThreadedReadFile(string _fileName, vector<string>* _listOut);
vector<string> BubbleSort(vector<string> _listToSort, ESortType _sortType);
void WriteAndPrintResults(const vector<string>& _masterStringList, string _outputName, int _clocksTaken);
void iterativeMergeSort(vector<string>& a, ESortType _sortType);
bool detectTextFile(string& _fileName);
void merge2(vector<string>& a, ESortType _sortType, int low, int mid, int high);
void mergeSort(vector<string>& a, ESortType _sortType, int low, int high);
void mergeTSort(vector<string>* a, ESortType _sortType, int thread_number, int max_threads);
void threadedMergeSort(vector<string>& a, ESortType _sortType);

////////////////////////////////////////////////////////////////////////////////////////////////////
// Main
////////////////////////////////////////////////////////////////////////////////////////////////////
int main() {
	ios_base::sync_with_stdio(0);
	cin.tie(NULL);
	cout.tie(NULL);
	
	// Enumerate the directory for input files
	vector<string> fileList;
	// vector reserve will reserve memory. Adding elements will be easier. 50 is an approximate number
	fileList.reserve(50);
    string inputDirectoryPath = "../InputFiles";
	// This will check is the input directory is valid
	if (!fs::is_directory(inputDirectoryPath)) {
		cout << "Input Directory is not a valid directory" << endl;
		return 0;
	}
    for (const auto & entry : fs::directory_iterator(inputDirectoryPath)) {
		if (!fs::is_directory(entry)) {
			// push back creates a copy. Emplace back is efficient
			string fileName = entry.path().string();
			int len = fileName.size();
			// check if file extension is of type .txt
			if (fileName[len - 4] == '.' && fileName[len - 3] == 't' && fileName[len - 2] == 'x' && fileName[len - 1] == 't') {
				fileList.emplace_back(fileName);
			}
		}
		
	}
	// the vector can have more reserved memory than needed. Free that extra memory
	fileList.shrink_to_fit();
	// Check is any files were found
	if (!fileList.size()) {
		cout << "No files were found in the input directory!" << endl;
		return 0;
	}
	// USE Visual Studio's CRT DBG for memory debugging
#if USE_VS_CRTDBG
	_CrtMemState s1;
	_CrtMemCheckpoint(&s1);
#endif
	/*for (int times = 0; times < 100; times++) {
		DoSingleThreaded(fileList, ESortType::AlphabeticalAscending, "SingleAscending");
	}*/
	cout << "Regular" << endl;
	for (int times = 0; times < 100; times++) {
		DoMultiThreaded(fileList, ESortType::AlphabeticalAscending, "MultiAscending");
	}

	// Do the stuff
	//DoSingleThreaded(fileList, ESortType::AlphabeticalAscending,	"SingleAscending");
	//DoSingleThreaded(fileList, ESortType::AlphabeticalDescending,	"SingleDescending");
	//DoSingleThreaded(fileList, ESortType::LastLetterAscending,		"SingleLastLetter");
#if MULTITHREADED_ENABLED
	//DoMultiThreaded(fileList, ESortType::AlphabeticalAscending,		"MultiAscending");
	//DoMultiThreaded(fileList, ESortType::AlphabeticalDescending,	"MultiDescending");
	//DoMultiThreaded(fileList, ESortType::LastLetterAscending,		"MultiLastLetter");
#endif

	// Wait
	cout << endl << "Finished...";
	getchar();
#if USE_VS_CRTDBG
	_CrtMemState s2;
	_CrtMemCheckpoint(&s2);
	_CrtMemState s3;
	if (_CrtMemDifference(&s3, &s1, &s2)) _CrtMemDumpStatistics(&s3);
	_CrtDumpMemoryLeaks();
#endif
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// The Stuff
////////////////////////////////////////////////////////////////////////////////////////////////////
void DoSingleThreaded(vector<string> &_fileList, ESortType _sortType, string _outputName) {
	try {
		clock_t startTime = clock();
		vector<string> masterStringList;
		// reserve for 1000 elements
		masterStringList.reserve(1000);
		for (unsigned int i = 0; i < _fileList.size(); ++i) {
			vector<string> fileStringList = {};
			// pass by reference. This avoids copy
			ReadFile(_fileList[i], fileStringList);
			//cout << _fileList[i] << endl;
			for (unsigned int j = 0; j < fileStringList.size(); ++j) {
				masterStringList.emplace_back(fileStringList[j]);
			}

		}
		// resize the vector to fit the actual data
		masterStringList.shrink_to_fit();

		//masterStringList = BubbleSort(masterStringList, _sortType);
		// use merge sort because the complexity is O(nlogn). 
		// Comment line 241 and uncomment line 242 to use the threaded merge sort
		mergeSort(masterStringList, _sortType, 0, masterStringList.size()-1);
		//threadedMergeSort(masterStringList, _sortType);
		clock_t endTime = clock();
		WriteAndPrintResults(masterStringList, _outputName, endTime - startTime);
	}
	catch (exception e) {
		cout << "An error has occured" << endl;
	}
	
}

void DoMultiThreaded(vector<string> &_fileList, ESortType _sortType, string _outputName) {
	clock_t startTime = clock();
	vector<string> masterStringList;
	// Initialize thread list at the begining 
	vector<thread> workerThreads(_fileList.size());
	// write to different sections of the memory so that insert is faster
	vector<padded_vector> master(_fileList.size(), padded_vector());
	for (unsigned int i = 0; i < _fileList.size(); ++i) {
		workerThreads[i] = thread(ThreadedReadFile, _fileList[i], &master[i].value);
	}
	// join all threads
	for (int i = 0; i < _fileList.size(); i++) 
		workerThreads[i].join();
	for (int i = 0; i < _fileList.size(); i++) {
		masterStringList.insert(masterStringList.end(), master[i].value.begin(), master[i].value.end());
	}
	
	
	
	 //mergeSort(masterStringList, _sortType, 0, masterStringList.size() - 1);
	// use threaded merge sort
	threadedMergeSort(masterStringList, _sortType);
	clock_t endTime = clock();

	WriteAndPrintResults(masterStringList, _outputName, endTime - startTime);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// File Processing
////////////////////////////////////////////////////////////////////////////////////////////////////

bool detectTextFile(string& _fileName) {

	return true;
}

void ReadFile(string &_fileName, vector<string> &ans) {
	// The optimized file read, reads the entire file buffer into the memory. 
	// file handling is known to be slow hence it is beneficial if the i/o is minimzed by reading large chunks
#if OPTIMIZED_FILE_READ

#if USE_MAGIC_NUMBER
	// Future: Get database of all magic numbers and see that the file's magic number doesn't exist in it
	if (!detectTextFile(_fileName)) return;
#endif 
	

	ifstream f(_fileName, ios::in | ios::binary);
	// return if the file is not open
	if (!f.is_open()) return;
	// Obtain the size of the file.
	const unsigned int sz = (unsigned int)fs::file_size(_fileName);

	// Create a buffer.
	string result(sz, '\0');

	// Read the whole file into the buffer.
	f.read(&result[0], sz);
	// close the open file handle
	f.close();
	// count new lines to allocate a vector of size countofNewLine that will store all the \n separated strings
	int countofNewLine = 1;
	for (int i = 0; i < result.size(); i++) {
		if (result[i] == '\n') countofNewLine++;
		
	}
	// resize the array
	ans.reserve(countofNewLine);
	string a = "";
	int j = 0;
	for (int i = 0; i < result.size(); i++) {
		#ifdef __unix__                    /* __unix__ is usually defined by compilers targeting Unix systems */
			if (result[i] == '\n') {
				if (a.size()) ans.emplace_back(a);
				a = "";
				a.clear();
			}
			else {
				a += result[i];
			}

		#elif defined(_WIN32) || defined(WIN32)     /* _Win32 is usually defined by compilers targeting 32 or   64 bit Windows systems */
			// windows uses \r\n to delimit a file
			if (result[i] == '\r') {
				if(a.size()) ans.emplace_back(a);
				i++;
				a = "";
				a.clear();
			}
			else {
				a += result[i];
			}

		#endif
		
	}
	ans.emplace_back(a);
	ans.shrink_to_fit();
	
	
	//for (int i = 0; i < ans.size(); i++) cout << ans[i]<<" "<<i<<endl;


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
	// exceptions do not propagate from threads to the main thread. 
	// Hence exception handling should be done in the child threads rather than main thread
	try {
		// initialize empty vector
		vector<string> temp = {};
		// read file data into vector
		ReadFile(_fileName, temp);
		// check is size is empty
		if (temp.size() == 0) return;
		
		
		(*_listOut).insert((*_listOut).end(), temp.begin(), temp.end());
		
		
	}
	catch (exception e) {
		cout << "An error has occured" << endl;
	}
	
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
// A common sort function that chooses the sort type based on _sortType
bool commonSort(string &a, string &b, ESortType _sortType) {
	bool ans = 0;
	if (_sortType == ESortType::AlphabeticalAscending) {
		// Rather than using the heap, which is dynamic allocation and uses the 'new' keyword, using memory on the stack is much faster
		// This is because the stack is sequential in the memory hence allocation is very quick
		// C++ will automatically free memory. This is called automatic storage
		// Stack should not be used if the variables are dynamic in nature since memory is limited
		AlphabeticalAscendingStringComparer stringSorter;
		ans = stringSorter.Sort(a, b);
	}
	else if (_sortType == ESortType::AlphabeticalDescending) {
		AlphabeticalDescendingStringComparer stringSorter;
		ans = stringSorter.Sort(a, b);
	}
	else if (_sortType == ESortType::LastLetterAscending) {
		AlphabeticalDescendingLastCharComparer stringSorter;
		ans = stringSorter.Sort(a, b);
	}
	return ans;
}


void merge2(vector<string>& a, ESortType _sortType, int low, int mid, int high) {
	int i, j, k;
	int n1 = mid - low + 1;
	int n2 = high - mid;
	vector<string> left(n1), right(n2);
	for (i = 0; i < n1; i++)
		left[i] = a[low + i];
	for (j = 0; j < n2; j++)
		right[j] = a[mid + 1 + j];
	i = 0, j = 0;
	k = low;
	while (i<n1 && j<n2) {
		if (commonSort(left[i], right[j], _sortType)) {
			a[k] = left[i];
			i++;
		}
		else {
			a[k] = right[j];
			j++;
		}
		k++;
	}
	while (i < n1) {
		a[k] = left[i];
		i++;
		k++;
	}
	while (j<n2) {
		a[k] = right[j];
		j++;
		k++;
	}

}
void mergeSort(vector<string>& a, ESortType _sortType, int low,int high) {
	
	
	if (low < high) {
		int mid = low + (high - low) / 2;
		mergeSort(a, _sortType, low, mid);
		mergeSort(a, _sortType, mid + 1, high);
		merge2(a, _sortType, low, mid, high);
	}
}
void mergeTSort(vector<string>* a, ESortType _sortType, int low, int high) {
		//mergeSort((*a), _sortType, 0,  ((*a).size() - 1 )/ 2);
		//mergeSort((*a), _sortType, ((*a).size() + 1)/ 2, (*a).size() - 1);
		mergeSort((*a), _sortType, low, high);
	
}

void threadedMergeSort(vector<string>& a, ESortType _sortType) {
	//int max_threads = 2;
	
	int low = 0, high = (a.size() - 1);
	
	int mid = low + (high - low) / 2;
	// Use 2 threads to sort the first half and second half of the array
	thread t1(mergeTSort, &a, _sortType, low, mid);
	thread t2(mergeTSort, &a, _sortType, mid + 1, high);
	t1.join();
	t2.join();
	// merge the first and second half
	merge2(a, _sortType, low, mid, high);
	

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
	cout << _clocksTaken << endl;
	
	ofstream fileOut(_outputName + ".txt", ofstream::trunc);
	// Instead of writing small chunks and utilizing many i/o file calls, making one large chunk
	// is more efficient since it minimizes the number of file i/o which is known to be slow
	string output = "";
	for (unsigned int i = 0; i < _masterStringList.size(); ++i) {
		//fileOut << _masterStringList[i]<<endl;
		output += _masterStringList[i] + '\n';
	}
	fileOut.write(output.data(), output.size());
	fileOut.close();
}
