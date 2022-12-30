rem This runs the functional test from Klaus Dormann
rem Let it run for a few minutes then ^c to exit
rem check ntvao.log (which will be large!) to see if it's looping on
rem   pc 347d, op 4c, op2 7d, op3 34, a f0, x 0e, y ff, sp ff, NVbdizC, jmp $347d
rem to check, use tail ntvao.log
rem if it is looping on the line above, it passed the tests
rem if it's looping on something else, there's a bug
rem if it's not looping, the test was still running, so give it more time

ntvao /a:400 /i /t /c 6502_functional_test.hex

