.DEFAULT_GOAL := all

.PHONY : all
all : man rdate

.PHONY : man
man : rdate.1

rdate.1 : MANUAL.markdown
	md2man-roff $< > $@

rdate : rdate.o
	gcc -o $@ $<
	strip $@

rdate.o : $(wildcard src/*.c)
	gcc -c -o $@ src/rdate.c -Isrc -ansi

.PHONY : clean
clean :
	-@rm rdate.o rdate rdate.1
