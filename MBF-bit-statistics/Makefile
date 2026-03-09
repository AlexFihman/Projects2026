CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -g -O3 -pthread
SHARED_OBJS = MonotoneBooleanFunction.o ShortList.o

all: bitstats measure_dist

bitstats: main.o $(SHARED_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

measure_dist: measure_dist.o $(SHARED_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f *.o bitstats measure_dist
