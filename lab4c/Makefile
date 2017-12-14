#NAME: CODY HUBBARD
#EMAIL: CodySpHubbard@gmail.com
#UID: 004843389

default:
	gcc -g -Wall -Wextra -pthread -lm -lmraa lab4c_tcp.c -o lab4c_tcp
	gcc -g -Wall -Wextra -pthread -lm -lmraa -lssl -lcrypto lab4c_tls.c -o lab4c_tls
clean:
	-rm -rf lab4c_tcp lab4c_tls lab4b-004843389.tar.gz logfile
dist:
	@-rm lab4c-004843389.tar.gz
	tar -czf lab4c-004843389.tar.gz lab4c_tcp.c lab4c_tls.c Makefile README