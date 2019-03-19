CXX := clang++

# Use this for strict compilation check(will work on clang 3.8+)
EXTRA_CXXFLAGS := -fsanitize=address,undefined -Wall -Werror -Weverything -Wno-c++11-long-long -Wno-c++98-compat -Wno-padded

all:
	$(CXX)  $(EXTRA_CXXFLAGS) -I../../ -std=c++11 -g -O0 -o xpd_simple_spline_writer simple_spline_writer.cc
