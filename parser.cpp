#include "parser.h"


int SkipSpace(const char* txt, int pos) {
	int i = pos;
	while(isblank(txt[i])) { // non-zero if char is blank
		++i;
	}
	return i - pos;
}

int Parse(std::vector<char> v, int database_size, int& status) { // returns number of filters in query
	const char* uk = &v[0]; // why i can use txt += 5; if it's const ?????????????????????
	uk += 0; // just for not getting compilation warning
	// select time[1,2a] gives infinite loop, total = 0
	bool is_select = false, is_delete = false, is_something = false; // is_filter
	int pos = 0, pos1 = 0, pos2 = 0, cnt_select = 0, cnt_delete = 0, cnt_filters = 0;
	int date_min = INT_MAX, date_max = INT_MIN, time_min = INT_MAX, time_max = INT_MIN;
	char* txt = new char[v.size()];
	memcpy(txt, &v[0], v.size());
	char* filters = new char[v.size()]; // but starts after "select" or "delete"
	
	int skip_begin = SkipSpace(txt, 0);
	sscanf(txt + skip_begin, "select%n", &pos1);
	int select_pos = skip_begin + pos1; // current position after sscanf
	is_select = (select_pos > 0) && (txt[select_pos] == ' ' || txt[select_pos] == '\0' || txt[select_pos] == '\n');
	if(is_select) {
		is_something = true;
		++cnt_select;
		select_pos += SkipSpace(txt, select_pos);
		//txt += pos1;
		//printf("select\n");
	}
	
	sscanf(txt + skip_begin, "delete%n", &pos2);
	int delete_pos = skip_begin + pos2; // it will incorrectly update
	is_delete = (delete_pos > 0) && (txt[delete_pos] == ' ' || txt[delete_pos] == '\0' || txt[delete_pos] == '\n');
	if(is_delete) {
		is_something = true;
		++cnt_delete;
		delete_pos += SkipSpace(txt, delete_pos);
		//txt += pos2;
		//printf("delete\n");
	}
	
	// tail of our string - filters, don't wanna check if there is another query next
	// or it will be checked automatically after all if statements
	
	memcpy(filters, txt, v.size()); // filters will have min correct size
	pos = (select_pos < delete_pos ? delete_pos : select_pos);  // current position
	
	if ((cnt_delete > 0 && cnt_select > 0) || (pos == 0)) {
		// i suppose it's incorrect input
		// pos = 0 means incorrect beginning of the string
		printf("There is delete and select or incorrect input at the beginning of the string\n");
		return -1;
	}
	
	int total = 0;
	
	while(true) {
		if (txt[pos] == '\n') { // so i shouldn't write txt += pos1 or txt += pos2
			break;
		}
		int c = sscanf(filters + pos, "date=[%n%d,%n%d]%n", &pos1, &date_min, &pos2, &date_max, &total);
		if(pos1 > 0 && pos2 > pos1 && total == 0) {
			// 0 < pos1 < pos2 but in total mistake - can't read second integer
			// 0 < pos1 guarantee that it's begins with date and not intersect with "time=[]" case
			fprintf(stderr, "Can't read second integer, have space after comma or braces isn't correct\n");
			return -1;
		}
		if (c == 2 && filters[pos + pos1] != ' ' && filters[pos + pos2] != ' ') {
			++cnt_filters;
			pos += total;
			//txt += pos;
			pos += SkipSpace(txt, pos);
			continue;
		}
		/*
		else if (c < 2) {
				is_something = false; // incorrect output
				cnt_select = cnt_delete = 0;
				break;
			}
		*/
		
		c = sscanf(filters + pos, "time=[%n%d,%n%d]%n", &pos1, &time_min, &pos2, &time_max, &total);
		if(pos1 > 0 && pos2 > pos1 && total == 0) {
			// 0 < pos1 < pos2 but in total mistake - can't read second integer
			// 0 < pos1 guarantee that it's begins with date and not intersect with "time=[]" case
			fprintf(stderr, "Can't read second integer, have space after comma or braces isn't correct\n");
			return -1;
		}
		if (c == 2 && filters[pos + pos1] != ' ' && filters[pos + pos2] != ' ') {
			++cnt_filters;
			pos += total;
			//txt += pos;
			pos += SkipSpace(txt, pos);
			continue;
		}
		/*
		else if (c < 2) {
				is_something = false; // incorrect output
				cnt_select = cnt_delete = 0;
				break;
			}
		 */
		
		// seems like always gives "Error"
		char trash[100];
		c = sscanf(filters + pos, "%[A-Za-z]%n", trash, &total);
		if (c != EOF && total) {
			is_delete = is_select = 0;
			is_something = false; // incorrect input
			break;
		} else break;
		
	}
	
	delete[] filters;
	delete[] txt;
	// nothing was read
	if (!is_something) {
		fprintf(stderr, "Nothing needed were at input\n");
		status = -1;
	}
	
	if (is_select) {
		printf("select");
	} else if (!is_select) {
		fprintf(stderr, "No 'select' filters\n");
		status = -2;
	}
	if (is_delete) {
		printf("delete");
	}
	if (is_something) {
		if (cnt_filters > database_size) {
			fprintf(stderr, "Number of filters greater than size of database\n");
			status = -1;
		}
		printf(", %d\n", cnt_filters);
	}
	return cnt_filters;
}

int FillVector(std::vector<char>& v) {
	int count = 0; // how many chars were read
	while(1) {
		char c = fgetc(stdin);
		if (c == EOF) {
			break;
		}
		v.push_back(c);
		++count;
		if (c == '\n') {
			v.push_back('\0');
			break;
		}
	}
	return count;
}
/*
int main() {
	while(true) {
		printf("> ");
		std::vector<char> v;
		int len = FillVector(v);
		if (len > 1) { // always '\0'
			Parse(v);
		} else {
			printf("\n");
			break;
		}
	}
	return 0;
}
*/