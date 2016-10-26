#include "Water.h"
using namespace std;

Water::Water(int colorSpacePointX, int colorSpacePointY) {
	timeCount=0;
	x = colorSpacePointX;
	y = colorSpacePointY;
	map[0] = { -1 ,  1 };	map[1] = { 0 ,  1 };		map[2] = { 1 ,  1 };
	map[3] = { -1 ,  0 };								map[4] = { 1 ,  0 };
	map[5] = { -1 , -1 };	map[6] = { 0 , -1 };		map[7] = { 1 , -1 };

}

Water::Way{

}