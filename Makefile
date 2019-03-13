CXX := clang++

# Use this for strict compilation check(will work on clang 3.8+)
EXTRA_CXXFLAGS := -fsanitize=address,undefined -Wall -Werror -Weverything -Wno-c++11-long-long -Wno-c++98-compat -Wno-padded

all:
	$(CXX)  $(EXTRA_CXXFLAGS) -std=c++11 -g -O0 -o xpd_reader_example xpd_reader_example.cc

lint:
	./cpplint.py tiny_xpd.h
