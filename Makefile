DEFAULT_GOAL := rdate

rdate : rdate.o
	gcc -o $@ $<

rdate.o : $(wildcard src/*.c)
	gcc -c -o $@ src/rdate.c -Isrc -ansi

.PHONY : clean
clean :
	-@rm rdate.o rdate
