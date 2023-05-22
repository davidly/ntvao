rem make a subset of the C runtime hacked to work on the Apple 1
rem build using a DOS emulator like https://github.com/davidly/ntvdm

ntvdm ..\bin\as65 -L -o samain.o samain.a65
ntvdm ..\bin\as65 -L -o crt0.o crt0.a65
ntvdm ..\bin\as65 -L -o sup.o sup.a65
ntvdm ..\bin\as65 -L -o stack.o stack.a65
ntvdm ..\bin\as65 -L -o istack.o istack.a65
ntvdm ..\bin\as65 -L -o shift.o shift.a65
ntvdm ..\bin\lb65 lee sup crt0 samain stack istack shift

