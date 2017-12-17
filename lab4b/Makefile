#NAME: CODY HUBBARD
#EMAIL: CodySpHubbard@gmail.com
#UID: 004843389

default:
	gcc -g -Wall -Wextra -pthread -lm -lmraa lab4b.c -o lab4b
check:
	echo OFF | ./lab4b --log=logfiletest --period=2 --scale=C
	-rm -rf logfiletest
clean:
	-rm -rf lab4b lab4b-004843389.tar.gz logfile
dist:
	@-rm lab4b-004843389.tar.gz
	tar -czf lab4b-004843389.tar.gz lab4b.c Makefile README