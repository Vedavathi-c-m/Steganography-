#include <stdio.h>
#include <string.h>
#include "decode.h"
#include "types.h"
#include "common.h"

//read_and_validate_decode_args
/*Decoding steps
1.Validate input arguments
    check for valid file fprmats
    check for NULL pointers
    check for file existence
    check for read/write permissions
2.Open stego image file and output file
    open stego image file in raed mode
    open output file in write mode
    check for errors
    return success/failure status
3.Decode and verify magic string
    read 8 bytes at a time from stego image
    decode each byte from 8 bytes of image data
    compare with original magoc string
    return success/ failure status
4.Decode secret file extn size
    read 32 bytes at a time from stego image 
    decode size from 32 bytes of image data
    store extn size in decode info structure
    return success/failure status
5.Decode secret file extn
    read 8 bytes at a time from stego image
    decode each byte from 8 bytes of image data
    store extn in decode info structure
    return success/failure status
6.Decode secret file size
    read 32 bytes at a time from stego image 
    decode size from 32 bytes of image data
    store extn size in decode info structure
    return success/failure status
7.Decode secret file data
   read 32 bytes at a time from stego image 
    decode size from 32 bytes of image data
    write to output file
    return success/failure status
8.close all files
    close stego image file and output file */
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
    if (argv[2] == NULL || strstr(argv[2], ".bmp") == NULL)
        return e_failure;

    decInfo->stego_image_fname = argv[2];
    //decInfo->output_fname = (argv[3] != NULL) ? argv[3] : "decoded.txt";
    if (argv[3] != NULL)
    {
    decInfo->output_fname = argv[3];
    }
    else
    {
        decInfo->output_fname = "decoded.txt";
    }

    return e_success;
}// open_decode_files

Status open_decode_files(DecodeInfo *decInfo)
{
    decInfo->fptr_stego_image = fopen(decInfo->stego_image_fname, "rb"); 
    if (decInfo->fptr_stego_image == NULL)
    {
        perror("fopen");
        return e_failure;
    }

    decInfo->fptr_output = fopen(decInfo->output_fname, "w");
    if (decInfo->fptr_output == NULL)
    {
        perror("fopen");
        fclose(decInfo->fptr_stego_image);
        return e_failure;
    }

    return e_success;
}
 // decode_byte_from_lsb

Status decode_byte_from_lsb(char *data, unsigned char *image_buffer)
{
    unsigned char ch = 0;
    for (int i = 0; i < 8; i++)
    {
        ch |= (image_buffer[i] & 1) << i; // match LSB-first encode
    }
    *data = ch;
    return e_success;
}

Status decode_size_from_lsb(int *size, unsigned char *image_buffer)
{
    *size = 0;
    for (int i = 0; i < 32; i++)
    {
        *size |= (image_buffer[i] & 1) << i; // match encode order
    }
    return e_success;
} 


//decode_magic_string

Status decode_magic_string(const char *magic_string, DecodeInfo *decInfo)
{
    fseek(decInfo->fptr_stego_image, 54, SEEK_SET);

    char buffer[strlen(magic_string) + 1];
    unsigned char image_buffer[8];

    for (int i = 0; i < strlen(magic_string); i++)
    {
        if (fread(image_buffer, 1, 8, decInfo->fptr_stego_image) != 8)
        { 
            return e_failure;
        }
        

        unsigned char ch = 0;
        for (int bit = 0; bit < 8; bit++)
        {
            ch = (ch << 1) | (image_buffer[bit] & 1);
        }

        buffer[i] = ch;
    }

    buffer[strlen(magic_string)] = '\0';
   // printf("DEBUG: Decoded magic string = \"%s\"\n", buffer);

    return (strcmp(buffer, magic_string) == 0) ? e_success : e_failure;
}


 // Function: decode_secret_file_extn_size

Status decode_secret_file_extn_size(DecodeInfo *decInfo)
{
    unsigned char image_buffer[32];
    if (fread(image_buffer, 1, 32, decInfo->fptr_stego_image) != 32)
    {
        return e_failure;
    }
    decode_size_from_lsb(&decInfo->extn_size,image_buffer);
    return e_success;
}

// decode_secret_file_extn
 
Status decode_secret_file_extn(DecodeInfo *decInfo)
{
    for (int i = 0; i < decInfo->extn_size; i++)
    {
        unsigned char image_buffer[8];
        if (fread(image_buffer, 1, 8, decInfo->fptr_stego_image) != 8)
        {
            return e_failure;
        }
        decode_byte_from_lsb(&decInfo->extn_secret_file[i], image_buffer);
    }
    decInfo->extn_secret_file[decInfo->extn_size] = '\0';
    return e_success;
}


//decode_secret_file_size
 
Status decode_secret_file_size(DecodeInfo *decInfo)
{
    unsigned char image_buffer[32];
    if (fread(image_buffer, 1, 32, decInfo->fptr_stego_image) != 32)
    {
        return e_failure;
    }
    decode_size_from_lsb(&decInfo->size_secret_file, image_buffer);
    return e_success;

}

// decode_secret_file_data

Status decode_secret_file_data(DecodeInfo *decInfo)
{
    unsigned char image_buffer[8];
    char ch;
    for (int i = 0; i < decInfo->size_secret_file; i++)
    {
        if (fread(image_buffer, 1, 8, decInfo->fptr_stego_image) != 8)
        {
            return e_failure;
        }
        decode_byte_from_lsb(&ch, image_buffer);
        fputc(ch, decInfo->fptr_output);
    }
    return e_success;
}

 //do_decoding

Status do_decoding(DecodeInfo *decInfo)
{
    if (open_decode_files(decInfo) == e_failure)
    {
        printf("ERROR:Unable to open files\n");
        return e_failure;
    }

    if (decode_magic_string(MAGIC_STRING, decInfo) == e_failure)
    {
        printf("ERROR:Unable to decode magic string\n");
        return e_failure;
    }
    if (decode_secret_file_extn_size(decInfo) == e_failure)
    {
        printf("ERROR:Unable to decode secret file extension size\n");
        return e_failure;
    }
    if (decode_secret_file_extn(decInfo) == e_failure)
    {
        printf("ERROR:Unable to decode secret file extension\n");
        return e_failure;
    }
    if (decode_secret_file_size(decInfo) == e_failure)
    {
        printf("ERROR:Unable to decode secret file size\n");
        return e_failure;
    }

    if (decode_secret_file_data(decInfo) == e_failure)
    {
        printf("ERROR:Unable to decode secret file data\n");
        return e_failure;
    }

    printf("INFO: Decoding successful! Data written to %s\n", decInfo->output_fname);
    return e_success;
    fclose(decInfo->fptr_output);
    fclose(decInfo->fptr_stego_image);
}
