all: client server

client: client.o distributed.o diary.o
	g++ client.o distributed.o diary.o -o client

client.o: client.cpp client.h
	g++ -g -Wall -Wextra -Wfloat-equal -Werror -pedantic -std=c++11 -c client.cpp

server: server.o diary.o parser.o distributed.o
	g++ server.o diary.o parser.o distributed.o -o server

diary.o: diary.cpp diary.h
	g++ -g -Wall -Wextra -Wfloat-equal -Werror -pedantic -std=c++11 -c diary.cpp

parser.o: parser.cpp parser.h
	g++ -g -Wall -Wextra -Wfloat-equal -Werror -pedantic -std=c++11 -c parser.cpp

distributed.o: distributed.cpp distributed.h
	g++ -g -Wall -Wextra -Wfloat-equal -Werror -pedantic -std=c++11 -c distributed.cpp

server.o: server.cpp server.h
	g++ -g -Wall -Wextra -Wfloat-equal -Werror -pedantic -std=c++11 -c server.cpp

clean:
	rm -f *.o client server diary parser distributed
