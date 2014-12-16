psrfits2fil:
	cc -g -I../ psrfits2fil.c read_psrfits.c write_psrfits.c send_stuff.c -L/home/pulsar/./ph-pipeline/packages/cfitsio_3.0_200811120922/lib/ -lcfitsio -lm -o psrfits2fil

strip:
	rm psrfits2fil

clean:
	rm -f *.o *~
