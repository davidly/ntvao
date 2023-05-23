rem none defaults to a machine with RAM from 0 to $8000, perfect for the emulator and replica
cc65 -Oi -Or -t none ttt.c
ca65 ttt.s
ld65 -o ttt -t none ttt.o none.lib
hd -w:0x1000 ttt  >ttt.hex
ntvao -p -c /s:1022727 ttt.hex

