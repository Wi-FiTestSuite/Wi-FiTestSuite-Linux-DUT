CC=gcc
CFLAGS= -Wall
LIB=-lpthread

SigmaWTG: SigmaWTG.c
	$(CC) $(CFLAGS) -o $@ $< $(LIB)

clean:
	rm -f SigmaWTG
install:
	sh installSigmaWTG.sh
uninstall:
	sh uninstallSigmaWTG.sh
