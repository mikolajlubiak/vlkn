CFLAGS = -std=c++20 -Ofast -mtune=native
LDFLAGS = -lglfw -lvulkan

vlkn: *.cpp *.hpp
	clang++ $(CFLAGS) -o vlkn *.cpp $(LDFLAGS)

clean:
	rm -f vlkn
