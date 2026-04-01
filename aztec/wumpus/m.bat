@echo off
setlocal

if "%~1"=="" (
    echo Error: No .c source file specified
    exit /b 1
)

rem link with lee.lib instead of c.lib to reduce binary size and target Apple 1 instead of Apple II
rem also, use start.a to setup memory for the Apple 1.
rem generate a ".bin" file so the output doesn't have 4 bytes of filler at the start.
rem use ntvdm to run the MS-DOS Aztec binaries: https://github.com/davidly/ntvdm

del %1.hex >nul 2>&1
del %1.bin >nul 2>&1

copy ..\shared\start.a /Y >nul 2>&1
copy ..\shared\a1.c /Y >nul 2>&1

ntvdm ..\bin\cg65 -A +B -I..\include -T -O %1.a %1.c

rem the optimizer fails with "out of space"
rem ntvdm -t ..\bin\optint65 -o opt.a -a -v %1.a

ntvdm ..\bin\as65 -L -O %1.o %1.a

ntvdm ..\bin\as65 -L -O start.o start.a

ntvdm ..\bin\ln65 -T -V -O %1.bin -B 1000 -C 1030 -D 7000 -U 7300 start.o %1.o ..\lib\c.lib

a1hex -a:0x1000 %1.bin

