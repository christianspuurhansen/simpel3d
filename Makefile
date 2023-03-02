default: start

start: start.cpp
	g++ -O3 -o start start.cpp -lSDL -lSDL_gfx
