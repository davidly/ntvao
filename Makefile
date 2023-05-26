CXXFLAGS = -ggdb -Ofast -fno-builtin -D DEBUG -I .

ntvao: ntvao.o mos6502.o
	$(CXX) $(CXXFLAGS) ntvao.o mos6502.o -o $@

ntvao.o: ntvao.cxx 
	$(CXX) $(CXXFLAGS) -c $<

mos6502.o: mos6502.cxx 
	$(CXX) $(CXXFLAGS) -c $<

check: ntvao
	./ntvao hello.hex

clean:
	$(RM) ntvao.o mos6502.o
