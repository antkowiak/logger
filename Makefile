BOOST_ROOT=/mnt/e/boost/boost_1_73_0/

all : run_client run_server

run_client : Makefile client/src/*.h client/src/*.cpp
	g++ -O3 -std=c++17 -I$(BOOST_ROOT) -Wall -Wextra -Wpedantic -Wno-unknown-pragmas client/src/*.cpp -lpthread -o run_client

run_server : Makefile server/src/*.h server/src/*.cpp
	g++ -O3 -std=c++17 -I$(BOOST_ROOT) -Wall -Wextra -Wpedantic -Wno-unknown-pragmas server/src/*.cpp -lpthread -o run_server

clean :
	\rm -f run_client run_server

