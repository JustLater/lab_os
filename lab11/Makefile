.PHONY: all rwlock clean
all: rwlock
rwlock: rwlock.c
	gcc -lpthread rwlock.c -o rwlock
clean:
	rm -rf *.o rwlock
