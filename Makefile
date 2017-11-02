FLAGS?=

CFLAGS := -fPIC -Wall -std=c++11 -O3 $(FLAGS)

example: ./example.cpp
	mkdir -p bin
	g++ -I include $(CFLAGS) $< -o bin/example
	bin/example

clean:
	rm -r bin 

.PHONY: clean example 
