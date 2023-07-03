rem link with lee.lib instead of c.lib to reduce binary size and target Apple 1 instead of Apple II
rem also, use start.a to setup memory for the Apple 1.
rem use hd.exe to generate an Apple 1 hex input file. https://github.com/davidly/hd
rem generate a ".bin" file so the output doesn't have 4 bytes of filler at the start.
rem use ntvdm to run the MS-DOS Aztec binaries: https://github.com/davidly/ntvdm

ntvdm ..\bin\cg65 -A +B -I..\include -T -O ttt-6502.a ttt-6502.c

ntvdm ..\bin\as65 -L -O ttt-6502.o ttt-6502.a

ntvdm ..\bin\as65 -L -O start.o start.a

ntvdm ..\bin\ln65 -T -V -O ttt-6502.bin -B 1000 -C 1030 -D 1000 -U 2200 start.o ttt-6502.o ..\leelib\lee.lib

c:\ntbin\hd /w:0x1000 ttt-6502.bin >tttaztec.hex

