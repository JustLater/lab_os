.PHONY: all sender receiver clean
all: receiver sender
sender: sender.c
	gcc sender.c -o sender
receiver: receiver.c
	gcc receiver.c -o receiver
clean:
	rm -rf *.o sender receiver