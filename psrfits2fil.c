#include <stdio.h>
#include <string.h>
#include <math.h>
#include "psrfits.h"
#include "header.h"
void send_coords(double raj, double dej, double az, double za) ;
void send_double (char *name, double double_precision) ;
void send_float(char *name,float floating_point) ;
void send_int(char *name, int integer) ;
void send_long(char *name, long integer) ;
void send_string(char *string) ;
FILE *input, *output;
int swapout;
int main (int argc, char **argv){
  unsigned char *data;
  int flip=0,i,j,k,l,x,status,first=1,startchan,endchan,idump=0,ndumps=1;
  struct psrfits pf;
  double toff,fcent,chbw;
  double rah,ram,ras,ded,dem,des,sgn;
  char filfile[1024];
  char extension[32]; // ek 16/09/2013;
  // ek 16/09/2013 By removing the weird file name thing these variables were no longer needed
  /*char *junk1,*junk2,*junk3,*junk4,*junk5;
  junk1=(char *) malloc(80);
  junk2=(char *) malloc(80);
  junk3=(char *) malloc(80);
  junk4=(char *) malloc(80);
  junk5=(char *) malloc(80);*/

  if (argc<2) {
    puts("usage: psrfits2fil fitsfile (flip) (fcentMHz)");
    exit(0);
  }
  if (argc>2){
    if (strstr(argv[2],"flip")!=NULL)  flip=1;
  }
  fcent=0.0;
  if (argc>3) {
    fcent=atof(argv[3]);
  }

  // ek 16/09/2013
  // Removed the below bit which was looking specifically for files of the form x_x_x.fits
  // for some reason I can't remember (made sense for an old Jodrell file naming convention
  // I guess). Changed this to read in X.<any extension> and write out X.fil
  //strcpy(junk1,strtok(argv[1],"_"));
  //strcpy(junk2,strtok(NULL,"_"));
  //strcpy(junk3,strtok(NULL,"_"));
  //sscanf(junk3,".fits");
  //printf("%s\n",&pf.filenum);
  //sprintf(pf.basefilename,"%s_%s_%s",junk1,junk2,junk3);
  //sprintf(filfile,"%s_%04d.fil",pf.basefilename,pf.filenum);
  strcpy(pf.basefilename,strtok(argv[1],".")); // ek 16/09/2013
  strcpy(extension,strtok(NULL,"."));          // ek 16/09/2013
  sprintf(filfile,"%s.fil",pf.basefilename);   // ek 16/09/2013
  printf("Output filterbank file %s\n",filfile);
  if (flip==1) printf("Flipping channels\n");
  output=fopen(filfile,"wb");
  if (output == NULL) printf("Pointer null! Maybe you don't have permission to write here?\n");
  //  sprintf(pf.filename,"%s_%s_%s",junk1,junk2,junk3);
  sprintf(pf.filename,"%s.%s",pf.basefilename,extension); // ek 16/09/2013
  //  printf("%s\n",pf.filename);
  fflush(stdout);

  if (psrfits_open(&pf) != 0) {
    fprintf(stderr,"error opening file\n");
    exit(0);
  }

  while ( psrfits_read_subint(&pf)== 0) {
    if (first) {
      if (fcent == 0.0) fcent=pf.hdr.fctr;
      fprintf(stderr,"cent frequency %f MHz\n",fcent);
      chbw=pf.hdr.BW/pf.hdr.nchan;
      /* broadcast header file */
      send_string("HEADER_START");
      send_string("rawdatafile");
      send_string(pf.filename);
      send_string("source_name");
      send_string(pf.hdr.source);
      send_int("data_type",1);
      startchan=1;
      endchan=pf.hdr.nchan;
      send_int("nchans",pf.hdr.nchan);
      chbw *= -1; // ek 16/09/2013, this should be done first, so that fchq 
                  // is middle of 1st chan, i.e. chbw/2 lower than top freq
      send_double("fch1",fcent+fabs(pf.hdr.BW)/2.+chbw/2.+(startchan-1)*chbw);
      //      chbw *= -1;
      /* if (flip)  chbw *= -1; */
      send_double("foff",chbw);
      send_int("nbits",pf.hdr.nbits);
      send_int("nifs",1);
      send_double("tsamp",pf.hdr.dt);
      send_double("tstart",pf.hdr.MJD_epoch);
      send_int("telescope_id",6);
      send_int("machine_id",9);

      rah=atof(strtok(pf.hdr.ra_str,":"));
      ram=atof(strtok(NULL,":"));
      ras=atof(strtok(NULL,":"));
      src_raj=rah*10000.0+ram*100.0+ras;

      ded=atof(strtok(pf.hdr.dec_str,":"));
      dem=atof(strtok(NULL,":"));
      des=atof(strtok(NULL,":"));
      if (ded<0.0) 
	sgn=-1.0;
      else
	sgn=1.0;
      src_dej=fabs(ded)*10000.0+sgn*dem*100.0+sgn*des;
      az_start=pf.sub.tel_az;
      za_start=pf.sub.tel_zen;
      send_coords(src_raj,src_dej,az_start,za_start);
      send_string("HEADER_END");
      first=0;
      data=(unsigned char *)malloc(ndumps*sizeof(char)*pf.hdr.nchan);
    }

    i=j=k=l=0;
    while (i<pf.sub.bytes_per_subint) {
      // printf("%d %d %d %d\n",i,j,k,l);
      if (j==0) {
	k++;
	if ((k>=startchan) && (k<=endchan)) data[l++]=pf.sub.data[i];
	if (k==pf.hdr.nchan * pf.hdr.nbits / 8) {
	  idump++;
	  if (idump==ndumps) {
	    if (flip) {
	      for (x=l-1;x>=0;x--)
		fwrite(&data[x],sizeof(char),1,output);
	    } else {
	      fwrite(data,sizeof(char),l,output);
		// printf("writing data\n");
	    }
	    idump=l=0;
	  }
	  k=0;
	}
      }
      i++;
      if (i%pf.hdr.nchan==0) j++;
      if (j== pf.hdr.npol) j=0;
    }
    free(pf.sub.data);
    if (idump>0) fwrite(data,sizeof(char),l,output);
    idump=0;
  }
  free(data);

  return(0);
}
