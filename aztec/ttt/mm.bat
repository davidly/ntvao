ntvdm /t ..\bin\cg65 -A +B -I..\include -T -O min.a min.c

ntvdm ..\bin\as65 -L -O min.o min.a

ntvdm ..\bin\as65 -L -O start.o start.a

ntvdm ..\bin\ln65 -T -V -O min.bin -B 1000 -C 1030 -D 1000 -U 2000 start.o min.o ..\leelib\lee.lib

c:\ntbin\hd /w:0x1000 min.bin >min.hex


