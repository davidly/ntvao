@echo off
if "%1" == "" set PATH=C:\WINDOWS\system32;C:\WINDOWS
set CROOT=%1
if "%1" == "" set CROOT=C:\AZC6532C
set CLIB65=%CROOT%\LIB\
set INCL65=%CROOT%\INCLUDE
set CR65=%CROOT%\R\
set PATH=%CROOT%;%CROOT%\BIN;%CROOT%\TOOLS;%PATH%;