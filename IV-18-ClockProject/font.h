/*
 * font.h
 *
 *  Based on LaydaAda's source code - http://www.ladyada.net/make/icetube/design.html
 */

#ifndef FONT_H_
#define FONT_H_

const unsigned char specialchars[] = {
		0x02, //dash
		0xC6 //degree
};

const unsigned char alphatable[] = {
	0xEE, /* a 0*/
	0x3E, /* b 1*/
	0x1A, /* c 2*/
	0x7A, /* d 3*/
	0xDE, /* e 4*/
	0x8E, /* f 5*/
	0xF6, /* g 6*/
	0x2E, /* h 7*/
	0x60, /* i 8*/
	0x78, /* j 9*/
	0xAE, //k 10
	0x1C, // l 11
	0xAA, // m 12
	0x2A, // n 13
	0x3A, // o 14
	0xCE, //p 15
	0xF3, // q 16
	0x0A, //r 17
	0xB6, //s 18
	0x1E, //t 19
	0x38, //u 20
	0x38, //v 21
	0xB8, //w 22
	0x6E, //x 23
	0x76, // y 24
	0xDA, //z 25
	/* more */
};

const unsigned char numbertable[] =
{
		  0xFC /* 0 */,
		  0x60 /* 1 */,
		  0xDA /* 2 */,
		  0xF2 /* 3 */,
		  0x66 /* 4 */,
		  0xB6 /* 5 */,
		  0xBE, /* 6 */
		  0xE0, /* 7 */
		  0xFE, /* 8 */
		  0xE6, /* 9 */
};


#endif /* FONT_H_ */
