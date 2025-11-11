
HEADERS := $(wildcard *.h) anyobj/anyobj.h

pack_params_demo.exe : pack_params_demo.cpp ${HEADERS}
	${CXX} -o $@ $< -g -O2 -Wall

clean:
	rm -f *.o
	rm -f *.exe
