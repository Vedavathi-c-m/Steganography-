#ifndef DECODE_H
#define DECODE_H

#include <stdio.h>
#include "types.h"

/* Structure to store information required for decoding */
typedef struct _DecodeInfo
{
    /* Stego Image info */
    char *stego_image_fname; //to store stego image name
    FILE *fptr_stego_image; //to store address of stego image

    /* Output file info */
    char *output_fname; //store output file name
    FILE *fptr_output; //store address of output file

    /* Decoding data */
    char extn_secret_file[10]; //store secret file extension
    int extn_size; //store secret file extention  size

    int size_secret_file; //store secret file size

} DecodeInfo;

/* Function prototypes */

/* Read and validate decode arguments */
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo);

/* Open decode files */
Status open_decode_files(DecodeInfo *decInfo);

/* Decode functions */
Status decode_byte_from_lsb(char *data, unsigned char *image_buffer);
/* Decode size from LSB */

Status decode_size_from_lsb(int *size, unsigned char *image_buffer);
/* Decode magic string */

Status decode_magic_string(const char *magic_string, DecodeInfo *decInfo);
/* Decode secret file extension size */

Status decode_secret_file_extn_size(DecodeInfo *decInfo);
/* Decode secret file extension */

Status decode_secret_file_extn(DecodeInfo *decInfo);
/* Decode secret file size */

Status decode_secret_file_size(DecodeInfo *decInfo);
/* Decode secret file data */

Status decode_secret_file_data(DecodeInfo *decInfo);
/* Do decoding */

Status do_decoding(DecodeInfo *decInfo);

#endif
