#NAME: CODY HUBBARD
#EMAIL: CodySpHubbard@gmail.com
#UID: 004843389

default:
	gcc -Wall -Wextra -g -pthread -o lab2_list SortedList.c lab2_list.c
tests:
	-./sample.sh
profile:
	-rm -f ./raw.gperf 
	CPUPROFILE=raw.gperf ./lab2_list --iterations=1000 --threads=12 --sync=s
	pprof --text ./lab2_list raw.gperf > profile.out
	pprof --list=thread_function --ignore=fprintf ./lab2_list raw.gperf > profile.out
	-rm -f ./raw.gperf 
graphs:
	./lab2_list.gp
dist:
	tar -czf lab2b-004843389.tar.gz sample.sh lab2_list.gp README Makefile lab2_list.c SortedList.c SortedList.h lab2b_list.csv profile.out lab2b_1.png lab2b_2.png lab2b_3.png lab2b_4.png lab2b_5.png
clean:
	@rm -rf lab2_list
	@rm -rf lab2b_list.csv profile.out lab2b_1.png lab2b_2.png lab2b_3.png lab2b_4.png lab2b_5.png
	@-rm -rf lab2b-004843389.tar.gz