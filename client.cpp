#include "distributed.h"
#include "client.h"
#include "diary.h"

int main() {
  if(!signalIgnoring()) {
    return -1;
  }
  int fd = Connect();
  if (fd < 0) return -1;
  int check = Cycle(fd);
  if (check == -1) {
    return -1;
  }
  // now we connected to server - starting requests sequence
  if(Disconnect(fd) < 0) return -1;
  return 0;
}

static bool WriteRead(int fd, const std::vector<char>& str) {      // file descriptor created for communication
  const int size = str.size();
  char *txt = new char[size];
  for (int i = 0; i < size; ++i) {
    txt[i] = str[i]; // copying '\0' as well
  }
  uint32_t len = strlen(txt) + 1;                  // txt is string for sending to server
                                                    // +1 for '\0' character

  try {
    // 1. Send text
    if(write(fd, &len, sizeof len) != sizeof len) // trying to write 4-bytes sequence // for what?????????
      throw "Can't write a length\n";             // just coding number len in binary?
                                                  // for server to know sizeof(txt) ???
    if(write(fd, txt, len) != (ssize_t)len)       // trying to write string txt into socket
      throw "Can't write a text\n";

    // 2. Receive message - should get char
    char status;

    if (read(fd, &status, sizeof(status)) != sizeof(status)) {
      throw "Can't read parser status\n";
      return false;
    }
    printf("Status: %d\n", static_cast<int>(status));
    if (status != 0) {
        return true;
    }
    // there status = 0, should get amount of extracted records and read vector with size of this amount
    // seems like sequence of getting values from fd is very important (!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!)
    int extracted_size = 0;
    // read this size from server
    if (read(fd, &extracted_size, sizeof(extracted_size)) != sizeof(extracted_size)) {
        return false;
    }
    // now create vector and fill it from server data stored in fd
    std::vector<Diary> vect(extracted_size);

    for (int i = 0; i < extracted_size; ++i) {
      if (vect[i].ReadDiary(fd) != 0) {
        throw "Can't read diary record\n";
        return false; // it brokes my program!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1
      }
    }

    // 3. Now print what we've got
    for (const Diary& elem : vect) {
      elem.PrintDiary();
    }


    /*
    if(read(fd, &len, sizeof len) != sizeof len)  // first len to know size of server's message
      throw "Can't read a length";

    std::vector<char> buf(len, 0);                // array of bytes to save server's answer

    if(read(fd, &buf[0], len) != (ssize_t)len)    // read text message from server for one call
      throw "Can't read a text";                  // read and write from file(descriptor) to &buf
                                                  //returns number of readen bytes

    puts(&buf[0]);                                // gets pointer and write it into stdin adding '\n'
    */

    // OK NOW JUST THIS TEXT WE'VE SENT, IF ALL WORKS IT SHOULD BE VECTOR OF DATABASE RECORDS
    // get from server number of selected elements and print all of them

  }
  catch(const char *err) {
    fprintf(stderr, "%s\n", err);
    return false;
  }
  catch(std::bad_alloc const&) {
    fprintf(stderr, "Memory allocation error\n");
    return false;
  }

  delete[] txt;
  return true;
}

static int Connect() {
  const char        *host = "127.0.0.1";
  uint16_t           port = 8000;                         // arbitrary number from 5000 to 65535
  struct sockaddr_in addr;
  int                fd;

  // create socket with address and connect to the server
  // CONNECT
  memset(&addr, 0, sizeof addr);                          // lead memory area in addr to zero
  addr.sin_family = AF_INET;                              // AF_INET is const for addresation in iPv4 protocols
  addr.sin_port   = htons(port);
  if(inet_pton(AF_INET, host, &addr.sin_addr) < 1) {      // converts network address in iPv4 into integer
    fprintf(stderr, "Wrong host\n");                    // 1 if all ok, 0 if addr.sin_addr points to string with 
    return -1;                                          // ^|^ incorrect address, else returns -1
  }

  fd = socket(AF_INET, SOCK_STREAM, 0);                   // SOCK_STREAM - for flow communication, 0 for TCP
  if(fd == -1) {
    fprintf(stderr, "Can't create socket\n");
    return -1;
  }

  if(connect(fd, (struct sockaddr *)&addr, sizeof addr) == -1) { // socket descriptor, pointer to structure with
    fprintf(stderr, "Can't connect to the server\n");          // server address, number of bytes of this structure
    if(close(fd))
      fprintf(stderr, "Can't release file descriptor\n");  
    return -1;
  }

  return fd;
}
// IT DOESN'T WORK BECAUSE IT CAN'T CATCH NULLPTR WHICH ARISE WHEN I PUT ^C!!!!!!!!!!!!!!!!!!!
/*
static bool FillVector(std::vector<char>& v) {
  char buf[32];
  if (fgets(buf, 32, stdin) == nullptr) {
    return false;
  }
  while(1) {
    char c = fgetc(stdin);
    if (c == EOF) {
      break;
    }
    if (c == '\n') {               // so it's not for the file but only for console input
      v.push_back('\0');         // so v.size() includes '\0' character
      break;
    }
    v.push_back(c);
  }
  return true;
}
*/
bool ReadLine(std::vector<char> &line) {
  char buf[64];
  printf("> ");
  for (;;) {
    if (fgets(buf, 32, stdin) == nullptr) {
      return false;
    }
    for (int i = 0; buf[i] != '\0'; i++) {
      if (buf[i] == '\n' || buf[i] == '\r') {
        line.push_back('\0');
        return true;
      }
      line.push_back(buf[i]);
    }
  }
}

static int Cycle(int fd) {
  while (1) {
    std::vector<char> str;
    //FillVector(str); // fill it from console
    if (!ReadLine(str)) break;
    if (!WriteRead(fd, str)) break; // errors will be printe in inner calls, there we just end process
  }
  return 0;
}

static int Disconnect(int fd) {
  /*
  if(shutdown(fd, 2) == -1) {// 0 = socket read close, 1 = socket write close, 2 = both
    fprintf(stderr, "Can't shutdown socket\n");
  }
  if(close(fd)) {           // free descriptor
    fprintf(stderr, "Can't realese file descriptor\n");
    return -1;
  }
  return 0;
  */
  if (shutdown(fd, SHUT_RDWR) == -1 && errno != ENOTCONN) {
      fprintf(stderr, "Can't shutdown socket\n");
  }
  if (close(fd) == -1) {
    fprintf(stderr, "Can't release file descriptor\n");
    return -1;
  }
  return 0;
}
