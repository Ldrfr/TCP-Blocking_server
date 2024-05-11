#include "distributed.h"
#include "server.h"
#include "parser.h"
#include "diary.h"

static void Loop(int ld, Database& db, int database_size);

typedef struct {
  union {
    int l;
    double d;
  } val;
  int has_value;
} maybe_t;

maybe_t FieldToInteger(const char *str) {
  int i = 0;
  char *e;
  maybe_t result;
  errno = 0;
  i = strtol(str, &e, 10);
  if ((!errno) && (*e == '\0')) { 
    if ((INT_MIN <= i) && (i <= INT_MAX)) {
      result.val.l = i;
      result.has_value = 1;
    }
    else {
      result.has_value = -1;
    }
    return result;
  }
  result.has_value = 0;
  return result;
}


int main(int argc, char** argv) {
  if (argc < 2) {
    fprintf(stderr, "Programm call should be ./server N; where N database size and N >= 0\n");
    return -1;
  }
  maybe_t check;
  check = FieldToInteger(argv[1]);
  if (argc != 2 || check.has_value == -1 || check.has_value == 0 || check.val.l < 0) {
    // not exactly needed amount of arguments or argv[1] not integer
    fprintf(stderr, "Programm call should be:   ./server N; where N database size and N >= 0\n");
    return -1;
  }
  int database_size = check.val.l;

  Database db(database_size);
  db.GenerateRandomData(database_size);
  //db.Print();
  if(!signalIgnoring()) {
    return -1; // now it just ruin program
  }
  int ld = Connect();
  if (ld < 0) return -1;

  Loop(ld, db, database_size);
  // in each iteration processing connection request
  if (Disconnect(ld) < 0) return -1;

  return 0;
}



static int Connect() {
  const char *host = "127.0.0.1";
  uint16_t    port = 8000;
  sockaddr_in addr;
  int         ld   = -1;   // descriptor of receiving socket
  int         on;          // to set on this socket constant SO_REUSEADDR 
  
  // create socket address and call socket() to create socket itself
  memset(&addr, 0, sizeof addr);    
  addr.sin_family = AF_INET;
  addr.sin_port   = htons(port);
  if(inet_pton(AF_INET, host, &addr.sin_addr) < 1) {
    fprintf(stderr, "Wrong host\n");
    return -1;
  }

  ld = socket(AF_INET, SOCK_STREAM, 0);
  if(ld == -1) {
    fprintf(stderr, "Can't create socket\n");
    return -1;
  }

  // set OS settings to shutdown network address immediately after server's work
  // for this we set SO_REUSEADDR on next socket
  on = 1;
  if(setsockopt(ld, SOL_SOCKET, SO_REUSEADDR, &on, sizeof on) == -1) { // setting
    fprintf(stderr, "Can't setsockopt\n");
    if(close(ld))
      fprintf(stderr, "Can't realese file descriptor\n");
    return -1;
  }
                                                        // bind socket to network address
                                                        // pointer to structure which contain socket receiver address
  if(bind(ld, (sockaddr *)&addr, sizeof addr) == -1) {  // returns 0 if all ok, ld is descriptor of receiving socket
    fprintf(stderr, "Can't bind\n");                  // pointer at structure contains address of socket
    if(close(ld))
      fprintf(stderr, "Can't realese file descriptor\n");
    return -1;
  }

  // socket created and tied to network address, now it should be transferred to waiting state
  // by calling listen(socket descriptor, max length of queue connection request)
  // if all is ok returns 0, else returns -1
  // if success, ld is listening socket
  if(listen(ld, 5) == -1) {
    fprintf(stderr, "Can't listen\n");
    if(close(ld))
      fprintf(stderr, "Can't realese file descriptor\n");
    return -1;
  }

  return ld;
}

static int Disconnect(int fd) {
  // free descriptor of created socket and print status 
  if(close(fd)) {
    fprintf(stderr, "Can't realese file descriptor\n");
    return -1;
  }
  puts("Done.");
  return 0;
}

// IDK FROM WHERE THOS FUNCTIONS ARISE - IT TOOK ME 20 HOURS TO FIND THEM (JUST STEAL)
static int ConnectClient(int ld) {
  sockaddr_in addr;
  socklen_t addrlen;
  for (;;) {
    memset(&addr, 0, sizeof(addr));
    addrlen = sizeof(addr);
    // accept descriptor of listening socket, pointer to structure
    // which contains client address(who was connected), size of this structure
    //If no error occurs, accept returns a value of type SOCKET that is a descriptor for the new socket.
    // This returned value is a handle for the socket on which the actual connection is made.
    int fd = accept(ld, (sockaddr *)&addr, &addrlen);
    if (fd == -1) {
      if (errno == EINTR) return -1;
      if (errno != EAGAIN) fprintf(stderr, "Can't accept"); // IDK WTF is this, just from the site
      continue;
    }
    return fd;
  }
}

static void DisconnectClient(int fd) {
  if (fd == -1) return;
  if (shutdown(fd, SHUT_RDWR) == -1 && errno != ENOTCONN) {
    fprintf(stderr, "Can't shutdown socket\n");
  }
  if (close(fd) == -1) {
    fprintf(stderr, "Can't release file descriptor\n");
  }
}

static bool Process(int fd, Database& db, int database_size) {

  //just for no error
  db.GetSize();

  // 1. Read string from client
  uint32_t len;// for length client's text message and length of answer 
  //int n;
  if(read(fd, &len, sizeof len) != sizeof len) {
    fprintf(stderr, "Can't read a length\n");
    return false;
  }

  printf("Conn: %d Header: %" PRIu32 "\n", fd, len);
  // text messages themselve saves in buf
  std::vector<char> buf(len, 0);

  if(read(fd, &buf[0], len) != (ssize_t)len) {
    fprintf(stderr, "Can't read a text\n");
    return false;
  }

  if(!buf[len - 1])                                  // write to stdin - done
    printf("Conn: %d Body: %s\n", fd, &buf[0]);

  // THERE SHOULD BE PARSE AND RETURN QUERIES
  // 2. Process input data
  int status = 0;
  int extract_number = Parse(buf, database_size, status);// ~!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! Parse call
  // send to client status of select query - just as said in task (but obviously it's useless)
  char tmp = static_cast<char>(status);
  if(write(fd, &tmp, sizeof tmp) != sizeof tmp) {
    fprintf(stderr, "Can't send status to client\n");
    return false;
  }
  if (status != 0) {
    return true;// just continue getting queries
  }
  // create vector and fill it according to number of filters (BUT NOT WHAT THEY ARE CONTAINS, LOL)
  std::vector<Diary> v; // local variable 
  db.ExtractDataStraightForward(v, extract_number);
  if (write(fd, &extract_number, sizeof(extract_number)) != sizeof(extract_number) ) {
    fprintf(stderr, "Can't send size of vector with extracted values to client\n");
    return false;
  }
  for (int i = 0; i < extract_number; ++i) {
    if (v[i].WriteDiary(fd) != 0) {
      fprintf(stderr, "Extracted diary writing wasn't correct\n");
      return false; // can't send diary record to client -> terminate TCP
    }
  }
  /*
  // send (number code) length of message to client
  if(write(fd, &len, sizeof len) != sizeof len) {
    fprintf(stderr, "Can't write a length\n");
    return false;
  }
  // send message to client in address buf
  if(write(fd, &buf[0], len) != (ssize_t)len) {
    fprintf(stderr, "Can't write a text\n");
    return false;
  }
  */
  return true;
}

static void Loop(int ld, Database& db, int database_size) {
  // in each iteration processing connection request
  // set connection, get text from client, prepare and send answer, close connection
  for(;;) {
    int fd = ConnectClient(ld);
    if (fd == -1) {
      if (errno == EINTR) {
        return;
      } else {
        fprintf(stderr, "Can't accept\n");
        continue;
      }
      break;
    }
    // there i have in buf correctly written text
    while(Process(fd, db, database_size));
    printf("Disconnecting %d\n", fd);
    DisconnectClient(fd);    
  }
}

