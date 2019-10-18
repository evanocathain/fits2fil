/* read_psrfits.c 
 * Paul Demorest, 05/2008
 */
#include <stdio.h>
#include <string.h>
#include "psrfits.h"

/* This function is similar to psrfits_create, except it
 * deals with reading existing files.  It is assumed that
 * basename and filenum are filled in correctly to point to 
 * the first file in the set.
 */
int psrfits_open(struct psrfits *pf) {

    int itmp;
    double dtmp;
    char ctmp[256];

    struct hdrinfo *hdr = &(pf->hdr);
    struct subint  *sub = &(pf->sub);

    int *status = &(pf->status);
    int numhdus;

    // sprintf(pf->filename, "%s_%04d.fits", pf->basefilename, pf->filenum);

    fits_open_file(&(pf->fptr), pf->filename, READONLY, status);

    // If file no exist, exit now
    if (*status) { return *status; }
    fprintf(stderr,"Opened file '%s'\n", pf->filename);

    // Move to main HDU
    fits_movabs_hdu(pf->fptr, 1, NULL, status);

    // Figure out obs mode
    fits_read_key(pf->fptr, TSTRING, "OBS_MODE", hdr->obs_mode, NULL, status);
    int mode = psrfits_obs_mode(hdr->obs_mode);

    printf("OBS_MODE = %s\n",hdr->obs_mode);

    // Read some stuff
    fits_read_key(pf->fptr, TSTRING, "TELESCOP", hdr->telescope, NULL, status);
    printf("TELESCOP %s\n",hdr->telescope);
    fits_read_key(pf->fptr, TSTRING, "OBSERVER", hdr->observer, NULL, status);
    fits_read_key(pf->fptr, TSTRING, "PROJID", hdr->project_id, NULL, status);
    fits_read_key(pf->fptr, TSTRING, "FRONTEND", hdr->frontend, NULL, status);
    fits_read_key(pf->fptr, TSTRING, "BACKEND", hdr->backend, NULL, status);
    fits_read_key(pf->fptr, TSTRING, "FD_POLN", hdr->poln_type, NULL, status);
    fits_read_key(pf->fptr, TSTRING, "DATE-OBS", hdr->date_obs, NULL, status);
    fits_read_key(pf->fptr, TDOUBLE, "OBSFREQ", &(hdr->fctr), NULL, status);
    fits_read_key(pf->fptr, TDOUBLE, "OBSBW", &(hdr->BW), NULL, status);
    fits_read_key(pf->fptr, TINT, "OBSNCHAN", &(hdr->orig_nchan), NULL, status);
    fits_read_key(pf->fptr, TSTRING, "SRC_NAME", hdr->source, NULL, status);
    printf("SOURCE %s\n",hdr->source);
    fits_read_key(pf->fptr, TSTRING, "TRK_MODE", hdr->track_mode, NULL, status);
    // TODO warn if not TRACK?
    fits_read_key(pf->fptr, TSTRING, "RA", hdr->ra_str, NULL, status);
    fits_read_key(pf->fptr, TSTRING, "DEC", hdr->dec_str, NULL, status);
    fits_read_key(pf->fptr, TDOUBLE, "BMAJ", &(hdr->beam_FWHM), NULL, status);
    fits_read_key(pf->fptr, TSTRING, "CAL_MODE", hdr->cal_mode, NULL, status);
    fits_read_key(pf->fptr, TDOUBLE, "CAL_FREQ", &(hdr->cal_freq), NULL, status);
    fits_read_key(pf->fptr, TDOUBLE, "CAL_DCYC", &(hdr->cal_dcyc), NULL, status);
    fits_read_key(pf->fptr, TDOUBLE, "CAL_PHS", &(hdr->cal_phs), NULL, status);
    fits_read_key(pf->fptr, TSTRING, "FD_MODE", hdr->feed_mode, NULL, status);
    fits_read_key(pf->fptr, TDOUBLE, "FA_REQ", &(hdr->feed_angle), NULL, status);
    fits_read_key(pf->fptr, TDOUBLE, "SCANLEN", &(hdr->scanlen), NULL, status);
    //printf("Status %d\n",*status);
    //printf("SCAN LENGTH %lf\n",hdr->scanlen);
    fits_read_key(pf->fptr, TINT, "STT_IMJD", &itmp, NULL, status);
    //printf("Status %d\n",*status);
    hdr->MJD_epoch = (long double)itmp;
    //printf("MJD %Lf %d\n",(long double)itmp,itmp);
    fits_read_key(pf->fptr, TDOUBLE, "STT_SMJD", &dtmp, NULL, status);
    //    printf("Status %d\n",*status);
    hdr->MJD_epoch += dtmp/86400.0L;
    //    printf("MJD %f\n",dtmp);
    fits_read_key(pf->fptr, TDOUBLE, "STT_OFFS", &dtmp, NULL, status);
    //printf("Status %d\n",*status);
    hdr->MJD_epoch += dtmp/86400.0L;
    //printf("MJD %f\n",dtmp);
    //printf("MJD %Lf\n",hdr->MJD_epoch);
    fits_read_key(pf->fptr, TDOUBLE, "STT_LST", &(hdr->start_lst), NULL, status);
    //printf("Status %d\n",*status);
    
    //printf("Status %d\n",*status);
    // CFITSIO Error Status Codes Here: https://heasarc.gsfc.nasa.gov/docs/software/fitsio/c/c_user/node128.html
    if (*status) { return *status; }

    // Quick check of HDus
    fits_get_num_hdus(pf->fptr,&numhdus,status);
    printf("There are %d hdus at status %d\n",numhdus,*status);

    printf("About to move to 1st SUBINT\n");
    if (*status) { return *status; }
    // Move to first subint
    fits_movnam_hdu(pf->fptr, BINARY_TBL, "SUBINT", 0, status);

    if (*status) { return *status; }

    // Read some more stuff
    fits_read_key(pf->fptr, TINT, "NPOL", &(hdr->npol), NULL, status);
    printf("npol = %d\n",hdr->npol);
    fits_read_key(pf->fptr, TSTRING, "POL_TYPE", ctmp, NULL, status);
    if (strncmp(ctmp, "AA+BB", 6)==0) hdr->summed_polns=1;
    else hdr->summed_polns=0;
    fits_read_key(pf->fptr, TDOUBLE, "TBIN", &(hdr->dt), NULL, status);
    fits_read_key(pf->fptr, TINT, "NBIN", &(hdr->nbin), NULL, status);
    fits_read_key(pf->fptr, TINT, "NSUBOFFS", &(hdr->offset_subint), NULL, status);
    fits_read_key(pf->fptr, TINT, "NCHAN", &(hdr->nchan), NULL, status);
    fits_read_key(pf->fptr, TDOUBLE, "CHAN_BW", &(hdr->df), NULL, status);
    fits_read_key(pf->fptr, TINT, "NSBLK", &(hdr->nsblk), NULL, status);
    fits_read_key(pf->fptr, TINT, "NBITS", &(hdr->nbits), NULL, status);


    if (mode==SEARCH_MODE) 
        {
	sub->bytes_per_subint = 
            (hdr->nbits * hdr->nchan * hdr->npol * hdr->nsblk) / 8;
	printf("%d %d %d %d\n",hdr->nbits,hdr->nchan,hdr->npol,hdr->nsblk);
	printf("sub->bytes_per_subint = %d\n",sub->bytes_per_subint);
	}
    else if (mode==FOLD_MODE) 
        sub->bytes_per_subint = 
            (hdr->nbin * hdr->nchan * hdr->npol); // XXX data type??

    // Init counters
    pf->rownum = 1;
    fits_read_key(pf->fptr, TINT, "NAXIS2", &(pf->rows_per_file), NULL, status);

    printf("Made it through reading in the file\n");
    fflush(stdout);
    return *status;
}

/* Read next subint from the set of files described
 * by the psrfits struct.  It is assumed that all files
 * form a consistent set.  Read automatically goes to the
 * next file when one ends.  Arrays should be allocated
 * outside this routine.
 */
int psrfits_read_subint(struct psrfits *pf) {

    struct hdrinfo *hdr = &(pf->hdr);
    struct subint  *sub = &(pf->sub);
    int *status = &(pf->status);

    // See if we need to move to next file
    printf("rows per file %d \n",pf->rows_per_file);
    if (pf->rownum > pf->rows_per_file) {
      fits_close_file(pf->fptr, status);
      fprintf(stderr,"Closed file '%s'\n", pf->filename);
      pf->filenum++;
      if (psrfits_open(pf) != 0) {
	return *status;
      }
    }

    int mode = psrfits_obs_mode(hdr->obs_mode);
    int nchan = hdr->nchan;
    int nivals = hdr->nchan * hdr->npol;
    int row = pf->rownum;
    int colnum;

    // TODO: bad! really need to base this on column names
    fits_read_col(pf->fptr, TDOUBLE, 2, row, 1, 1, NULL, &(sub->tsubint), 
            NULL, status);
    fits_read_col(pf->fptr, TDOUBLE, 3, row, 1, 1, NULL, &(sub->offs), 
            NULL, status);
    fits_read_col(pf->fptr, TDOUBLE, 4, row, 1, 1, NULL, &(sub->lst), 
            NULL, status);
    fits_read_col(pf->fptr, TDOUBLE, 5, row, 1, 1, NULL, &(sub->ra), 
            NULL, status);
    fits_read_col(pf->fptr, TDOUBLE, 6, row, 1, 1, NULL, &(sub->dec), 
            NULL, status);
    fits_read_col(pf->fptr, TDOUBLE, 7, row, 1, 1, NULL, &(sub->glon), 
            NULL, status);
    fits_read_col(pf->fptr, TDOUBLE, 8, row, 1, 1, NULL, &(sub->glat), 
            NULL, status);
    fits_read_col(pf->fptr, TDOUBLE, 9, row, 1, 1, NULL, &(sub->feed_ang), 
            NULL, status);
    fits_read_col(pf->fptr, TDOUBLE,10, row, 1, 1, NULL, &(sub->pos_ang), 
            NULL, status);
    fits_read_col(pf->fptr, TDOUBLE, 11, row, 1, 1, NULL, &(sub->par_ang), 
            NULL, status);
    fits_read_col(pf->fptr, TDOUBLE, 12, row, 1, 1, NULL, &(sub->tel_az), 
            NULL, status);
    fits_read_col(pf->fptr, TDOUBLE, 13, row, 1, 1, NULL, &(sub->tel_zen), 
            NULL, status);
    fits_read_col(pf->fptr, TFLOAT, 14, row, 1, nchan, NULL, sub->dat_freqs,
            NULL, status);
    fits_read_col(pf->fptr, TFLOAT, 15, row, 1, nchan, NULL, sub->dat_weights,
            NULL, status);
    fits_read_col(pf->fptr, TFLOAT, 16, row, 1, nivals, NULL, sub->dat_offsets,
            NULL, status);
    fits_read_col(pf->fptr, TFLOAT, 17, row, 1, nivals, NULL, sub->dat_scales,
            NULL, status);
    printf("Before read:  %d %d %f %lf %d %d %d\n",hdr->npol,hdr->nsblk,sub->dat_freqs[100],sub->ra,row,mode,sub->bytes_per_subint);
    if (mode==SEARCH_MODE) {
      sub->data=malloc(sizeof(char)*sub->bytes_per_subint);
      fits_read_col(pf->fptr, TBYTE, 18, row, 1, (sub->bytes_per_subint),
                    NULL, sub->data, NULL, status);
    } else if (mode==FOLD_MODE) {
      fits_read_col(pf->fptr, TFLOAT, 18, row, 1, (sub->bytes_per_subint),
                NULL, sub->data, NULL, status);
    }
    // Complain on error
    fits_report_error(stderr, *status);

    if ( pf->rownum == pf->rows_per_file )
	*status = 1;

   // Update counters
    if (!(*status)) {
        pf->rownum++;
        pf->tot_rows++;
        pf->N += hdr->nsblk;
        pf->T = pf->N * hdr->dt;
    }

    return *status;
}
