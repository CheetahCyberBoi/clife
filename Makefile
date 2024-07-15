CFLAGS = -std=c17 -O2
# LDFLAGS = 
clife: 
	clang $(CFLAGS) -o main src/*.c -g $(LDFLAGS)

.PHONY: test clean

test: clife
	./main


clean: 
	rm -f main
