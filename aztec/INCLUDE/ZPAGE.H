*:ts=8
*
* Copyright (C) 1985 by Manx Software Systems, Inc.
*

*	These are set up for the Apple // Dos 3.3

STK	EQU	$00		;8 byte stack area
VAL	EQU	STK
SP	EQU	STK+2
AFRAME	EQU	STK+4
FRAME	EQU	STK+4
LFRAME	EQU	STK+6
PC	EQU	STK+6

TREGS	equ	$08		;24 byte tmp register area
R0	EQU	TREGS+0
R1	EQU	TREGS+4
R2	EQU	TREGS+8
R3	EQU	TREGS+12
R4	EQU	TREGS+16

ACC	EQU	TREGS+20
SEC	EQU	TREGS+22

REGS	EQU	$80		;16 byte user register area

TMP	EQU	$40		;4 byte zero page temporary area

