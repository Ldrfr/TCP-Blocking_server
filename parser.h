#ifndef __PARSER_H__
#define __PARSER_H__


#include <cstdio>
#include <vector>
#include <cstring> // for memcpy
#include <string>  // for isblank
#include <climits>

int SkipSpace(const char* txt, int pos);
int Parse(std::vector<char> v, int database_size, int& status); // returns number of filters in query
int FillVector(std::vector<char>& v);

#endif