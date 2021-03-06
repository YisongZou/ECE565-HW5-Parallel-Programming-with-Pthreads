TARGETS=rainfall_seq rainfall_pt

all: $(TARGETS)
clean:
	rm -f $(TARGETS)

rainfall_seq: rainfall_seq.cpp rainfall_seq.hpp
	g++ -O3 -std=c++11 -o rainfall_seq rainfall_seq.cpp

rainfall_pt: rainfall_pt.cpp rainfall_pt.hpp
	g++ -O3 -std=c++11 -pthread -o rainfall_pt rainfall_pt.cpp
