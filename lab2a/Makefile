#NAME: CODY HUBBARD
#EMAIL: CodySpHubbard@gmail.com
#UID: 004843389

default:
	gcc -Wall -Wextra -g -pthread -o lab2_add lab2_add.c
	gcc -Wall -Wextra -g -pthread -o lab2_list SortedList.c lab2_list.c
tests:
	-./sample.sh
graphs:
	./lab2_add.gp
	./lab2_list.gp
dist:
	tar -czf lab2a-004843389.tar.gz sample.sh README Makefile lab2_add.c lab2_list.c SortedList.c SortedList.h lab2_add.csv lab2_list.csv lab2_add-1.png lab2_add-2.png lab2_add-3.png lab2_add-4.png lab2_add-5.png lab2_list-1.png lab2_list-2.png lab2_list-3.png lab2_list-4.png
clean:
	@rm -rf lab2_add
	@rm -rf lab2_list
	@rm -rf lab2_add.csv lab2_list.csv lab2_add-1.png lab2_add-2.png lab2_add-3.png lab2_add-4.png lab2_add-5.png lab2_list-1.png lab2_list-2.png lab2_list-3.png lab2_list-4.png
	@-rm -rf lab2a-004843389.tar.gz