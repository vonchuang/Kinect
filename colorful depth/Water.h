#pragma once
#include <cstdio>
#include <cstdlib>
using namespacs std;

class Water {
public:
	Water(int,int);

	void Way(int);

private:
	int timeCount;
	int x, y;
	int map[8][2];
};