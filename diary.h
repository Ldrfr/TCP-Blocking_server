#ifndef __DIARY_H__
#define __DIARY_H__

#include <iosfwd>
#include <string>
#include <sstream>
#include <cstring>
#include <vector>
#include <ctime>
#include <unistd.h>
#include <cstdio>

struct Diary {
	char date[11]; // 10 + 1 characters; 08.05.2024\0
	int time;
	char event[10];
	Diary(char date[11], int time, char event[10]);
	Diary();
	int WriteDiary(int fd);
	int ReadDiary(int fd);
	void PrintDiary() const;
};

class Date {
private:
	int year, month, day;
public:
	Date();
	Date(const int& new_year, const int& new_month, const int& new_day);
	friend std::ostream& operator << (std::ostream& out, const Date& date);
	friend bool operator < (const Date& lhs, const Date& rhs);
};

class Database {
private:
	int size;
	std::vector<Diary> v;
public:
	Database(int n);
	void GenerateRandomData(int n);
	void Print();
	void ExtractDataStraightForward(std::vector<Diary>& res, int N);
	int GetSize() {return size;}
};

void CheckDateIsFormat(std::stringstream& stream);

#endif//
