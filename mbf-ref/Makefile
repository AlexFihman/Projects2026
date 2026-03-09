CXX = g++
CXXFLAGS = -std=c++17 -O2
TARGET = mbf-ref
SRCS = main.cpp func_lists.cpp mbf_generate.cpp time_sec.cpp
OBJS = $(SRCS:.cpp=.o)

COMMON_OBJS = func_lists.o mbf_generate.o time_sec.o

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

hamming6: hamming6.o $(COMMON_OBJS)
	$(CXX) $(CXXFLAGS) -pthread -o $@ $^

hamming7: hamming7.o $(COMMON_OBJS)
	$(CXX) $(CXXFLAGS) -pthread -o $@ $^

hamming8: hamming8.o $(COMMON_OBJS)
	$(CXX) $(CXXFLAGS) -pthread -o $@ $^

hamming_exact: hamming_exact.o $(COMMON_OBJS)
	$(CXX) $(CXXFLAGS) -pthread -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) hamming6.o hamming7.o hamming8.o hamming_exact.o $(TARGET) hamming6 hamming7 hamming8 hamming_exact

.PHONY: clean
