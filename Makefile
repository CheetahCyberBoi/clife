CFLAGS = -std=gnu11 -O2
# LDFLAGS = 
clife: 
	clang $(CFLAGS) -o main src/*.c -g

.PHONY: test clean

test: clife
	./main


clean: 
	rm -f main
