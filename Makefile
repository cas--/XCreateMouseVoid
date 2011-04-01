XCreateMouseVoid : XCreateMouseVoid.c
	gcc $(CFLAGS) XCreateMouseVoid.c -lX11 -o XCreateMouseVoid

debug:
	$(MAKE) XCreateMouseVoid "CFLAGS=-g -DDEBUG"

clean:
	rm XCreateMouseVoid
