CXX ?= g++

DEBUG ?= 1
ifeq ($(DEBUG), 1)
    CXXFLAGS += -g
else
    CXXFLAGS += -O2

endif

server: main.cpp  ./timer/lst_timer.cpp ./http/http_conn.cpp ./threadpool/threadpool.cpp ./log/log.cpp ./mysql/sql_connection_pool.cpp  ./webserver/webserver.cpp ./config/config.cpp
	$(CXX) -o server  $^ $(CXXFLAGS) -lpthread -L//usr/lib64/mysql -lmysqlclient

clean:
	rm  -r server
