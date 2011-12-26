
include ../Makefile.inc

PROGS = wfa_dut wfa_dut_init.o
#PROGS = wfa_dut

all: ${PROGS}

wfa_dut_init.o : wfa_dut_init.c

wfa_dut: wfa_dut.o wfa_dut_init.o ../lib/libwfa_dut.a 
	${CC} ${CFLAGS} -o $@ wfa_dut.o wfa_dut_init.o ${DUTLIBS}

clean:
	rm -f ${PROGS} ${CLEANFILES}
