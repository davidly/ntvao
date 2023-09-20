CXXFLAGS = -ggdb -Ofast -fno-builtin -D DEBUG -I .

ntvao: ntvao.o mos6502.o
	$(CXX) $(CXXFLAGS) ntvao.o mos6502.o -o $@

ntvao.o: ntvao.cxx mos6502.hxx
	$(CXX) $(CXXFLAGS) -c $<

mos6502.o: mos6502.cxx mos6502.hxx
	$(CXX) $(CXXFLAGS) -c $<

.PHONY: check install clean
check: ntvao
	./$< -c hello.hex

install: ntvao
	install -s $< /usr/local/bin

clean:
	$(RM) ntvao.o mos6502.o
