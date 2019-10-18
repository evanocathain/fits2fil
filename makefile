CC = gcc
CFLAGS = -c -O3 -D_GNU_SOURCE -D_FILE_OFFSET_BITS=64
INCL = -I./ #-I/home/pulsar/pgplot/
LIBS = -lcfitsio -lm
#LIBS = -L/usr/X11/lib -lX11 -L/home/pulsar/pgplot/ -lg2c -L/usr/lib/ -lpng -L/usr/lib64/ -L/usr/lib/gcc/x86_64-redhat-linux/3.4.6/ -lm

all: psrfits2fil strip clean

psrfits2fil: psrfits2fil.o read_psrfits.o send_stuff.o write_psrfits.o
	$(CC) psrfits2fil.o read_psrfits.o send_stuff.o write_psrfits.o -o psrfits2fil $(LIBS)

psrfits2fil.o: psrfits2fil.c read_psrfits.c send_stuff.c write_psrfits.c
	$(CC) $(CFLAGS) $(INCL) psrfits2fil.c

get_args.o: read_psrfts.c
	$(CC) $(CFLAGS) $(INCL) read_psrfits.c

read_data.o: send_stuff.c
	$(CC) $(CFLAGS) $(INCL) send_stuff.c

strings_equal.o: write_psrfits.c
	$(CC) $(CFLAGS) $(INCL) write_psrfits.c

strip:
	strip psrfits2fil

clean:
	rm -f *~ *.o
