ntvdm ..\bin\cg65 -A +B -I..\include -T -O ttt-6502.a ttt-6502.c

ntvdm ..\bin\as65 -L -O ttt-6502.o ttt-6502.a

ntvdm ..\bin\as65 -L -O start.o start.a

ntvdm ..\bin\ln65 -T -V -O ttt-6502.bin -B 1000 -C 1030 -D 1000 -U 2200 start.o ttt-6502.o ..\leelib\lee.lib

c:\ntbin\hd /w:0x1000 ttt-6502.bin >tttaztec.hex

