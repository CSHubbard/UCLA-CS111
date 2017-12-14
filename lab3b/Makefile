#NAME: CODY HUBBARD
#EMAIL: CodySpHubbard@gmail.com
#UID: 004843389

default:
	echo 'python lab3b.py $$1' > lab3b
	chmod 700 lab3b
dist:
	echo 'python lab3b.py $$1' > lab3b
	chmod 700 lab3b
	tar -czf lab3b-004843389.tar.gz README Makefile lab3b.py lab3b
clean:
	@rm -rf lab3b
	@-rm -rf lab3b-004843389.tar.gz