#NAME: CODY HUBBARD
#EMAIL: CodySpHubbard@gmail.com
#UID: 004843389

default:
	gcc -g -Wall -Wextra  lab0.c -o lab0
check:
	#test input redirection argument
	@echo "hello world test" > intest.txt
	@./lab0 --input=intest.txt > outtest.txt
	@diff intest.txt outtest.txt
	@if [ $$? -eq 0 ] ; \
	then \
		echo "Input was successfully redirected."; \
	else \
		echo "Input redirection failed."; \
	fi
	@rm -f intest.txt outtest.txt
	#test output redirection argument
	@echo "hello world test" > intest.txt
	@./lab0 --input=intest.txt --output=outtest.txt
	@diff intest.txt outtest.txt
	@if [ $$? -eq 0 ] ; \
	then \
		echo "Output was successfully redirected."; \
	else \
		echo "Output redirection failed."; \
	fi
	@rm -f intest.txt outtest.txt
	#test segmentation fault argument
	@./lab0 --segfault
	@if [ $$? -eq 139 ] ; \
	then \
		echo "Segfault was successfuly created."; \
	else \
		echo "Segfault creation failed."; \
	fi
	#test catch argument
	@./lab0 --catch --segfault
	@if [ $$? -eq 4 ] ; \
	then \
		echo "Segfault was successfuly created."; \
	else \
		echo "Segfault creation failed."; \
	fi
clean:
	@rm -f lab0 intest.txt outtest.txt lab0-004843389.tar.gz 
dist:
	@tar -czf lab0-004843389.tar.gz lab0.c Makefile README *.png