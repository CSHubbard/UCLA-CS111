#NAME: CODY HUBBARD
#EMAIL: CodySpHubbard@gmail.com
#UID: 004843389

default:
	gcc -g -Wall -Wextra -lmcrypt --save-temps lab1b-client.c -o lab1b-client
	gcc -g -Wall -Wextra -lmcrypt --save-temps lab1b-server.c -o lab1b-server
client:
	gcc -g -Wall -Wextra -lmcrypt --save-temps lab1b-client.c -o lab1b-client
server:
	gcc -g -Wall -Wextra -lmcrypt --save-temps lab1b-server.c -o lab1b-server
clean:
	@rm -f lab1b-client lab1b-server lab1b-004843389.tar.gz  *.s *.i *.o log.txt
dist:
	@tar -czf lab1b-004843389.tar.gz lab1b-client.c lab1b-server.c Makefile README my.key