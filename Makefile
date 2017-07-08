all: clean diskinfo diskget disklist diskput

diskinfo: diskinfo.c utilities.h 
	gcc -Wall diskinfo.c -o diskinfo

diskget: diskget.c utilities.h
	gcc -Wall diskget.c -o diskget

disklist: disklist.c utilities.h
	gcc -Wall disklist.c -o disklist

diskput: diskput.c utilities.h
	gcc -Wall diskput.c -o diskput

clean:
	-rm -rf *.o *.exe
