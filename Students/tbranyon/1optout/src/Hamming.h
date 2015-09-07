/*
	Thomas Branyon
	3 September 2015
	ECEN 489-501	
	
	Class to handle encoding and decoding (with error handling) (7,4) Hamming codes.
	Variables are 8-bit unsigned because the largest piece of data handled is a 7-bit vector.
	In the interest of memory conservation for scaling to large operations, the smallest
	data type that will accomodate this is used.
*/
#ifndef _HAMMING_H_
#define _HAMMING_H_
using namespace std;

//just call unsigned char "u8" so we can type less and avoid confusion
typedef unsigned char u8;

class Hamming74
{
	u8 G[7][4]; //Code generating matrix for (7,4) Hamming code
	public:
		u8 encode(u8 data);
		u8 decode(u8 codeword);
};

#endif
