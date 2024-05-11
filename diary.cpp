#include "diary.h"

Diary::Diary(char new_date[11], int new_time, char new_event[10]) {
  strcpy(date, new_date);
  time = new_time;
  strcpy(event, new_event);
}

Diary::Diary() {
  strcpy(date, "0000000000\0");
  time = 0;
}

int Diary::WriteDiary(int fd) {
  if ( (write(fd, date, sizeof(date)) != sizeof(date)) || 
    (write(fd, &time, sizeof(time)) != sizeof(time)) || 
    (write(fd, event, sizeof(event)) != sizeof(event)) ) {
    return -1;
  }
  return 0;
}

int Diary::ReadDiary(int fd) {
  if ( (read(fd, date, sizeof(date)) != sizeof(date)) || 
    (read(fd, &time, sizeof(time)) != sizeof(time)) || 
    (read(fd, event, sizeof(event)) != sizeof(event)) ) {
    return -1;
  }
  return 0;
}

void Diary::PrintDiary() const {
  printf("| %-20s | %4d | %-40s |\n", date, time, event);
}

void Database::Print() {
  printf("   date    |  time  |     event    \n");
  for (const Diary& elem : v) {
    printf("| %-20s | %6d | %-20s |\n", elem.date, elem.time, elem.event);
  }
}

void Database::GenerateRandomData(int size) {
  srand(time(0)); // or srand(1) for pseudo-random numbers;
  for (int j = 0; j < size; ++j) {
    int day, month, year, time;
    char temp_day[11];
    char event[10];
    day = 1 + rand()%31; month = 1 + rand()%12; year = 1917 + rand()%107; time = rand()%24;
    for (int k = 0; k < 9; ++k) {
      event[k] = 'a' + rand()%26;
    }
    event[9] = '\0'; // 10 element
    std::string date = std::to_string(day) + '.' + std::to_string(month) + '.' + std::to_string(year);
    for (int i = 0; i < 10; ++i) {
      temp_day[i] = date[i];
    }
    temp_day[10] = '\0';
    Diary diary(temp_day, time, event);
    v[j] = diary;
  }
}


Database::Database(int new_size) {
  size = new_size;
  v.resize(new_size);
}

void Database::ExtractDataStraightForward(std::vector<Diary>& res, int N) {
  for (int i = 0; i < N; ++i) {
    res.push_back(v[i]);
  }
}
