/* Copyright (C) 1985 by Manx Software Systems, Inc. */

extern unsigned short _Stksiz;

rsvstk(size)
{
	_Stksiz = size;
}

