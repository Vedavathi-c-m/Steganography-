#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "types.h"
#include "common.h"

/* Function Definitions */

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */
/*Encoding steps
1.open the source file,secret file and stego file
  check for errors
  check for valid file formats
  check for NULL pointers
  check for file existence
  check for read/write permissions
2.check capacity of source image
  check file has secret data can be hidden in source image
  check if source image can hold secret data
  return success/failure status
3.copy bmp header to stego image
  read 54 bytes from source image
  write 54 bytes to stego image
  return success/failure status
4.encode magic string to stego image
  read 8 bytes at a time from source image
  encode each byte of magic string to 8 bytes of image data
  write modified 8 bytes to stego image
  return success/failure status
5.encode secret file extn size
  read 4 bytes from secret file
  encode each byte of secret file extn size to 8 bytes of image data
  write modified 8 bytes to stego image
  return success/failure status
6.encode secret file extn
  read 8 bytes at a time from source image
  encode each byte of magic string to 8 bytes of image data
  write modified 8 bytes to stego image
  return success/failure status
7.encode secret file size
  read 4 bytes at a time from source image
  encode each byte of magic string to 8 bytes of image data
  write modified 8 bytes to stego image
  return success/failure status
8.encode secret file data
    read 8 bytes at a time from source image
  encode each byte of magic string to 8 bytes of image data
  write modified 8 bytes to stego image
  return success/failure status
9.copy remaining image data
  read till end of source image
  write to stego image
  return success/failure status
10.close all files
    close source image file,secret file,stego image file
11.return success/failure status*/
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    printf("width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    printf("height = %u\n", height);

    // Return image capacity
    return width * height * 3;
}


/* Get file size */
uint get_file_size(FILE *fptr)
{
    fseek(fptr, 0, SEEK_END); // Find the size of secret file data
    return ftell(fptr);
}


/* Validate and store file names */
Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    if (argv[2] == NULL || strstr(argv[2], ".bmp") == NULL)
        return e_failure;
    encInfo->src_image_fname = argv[2];

    if (argv[3] == NULL)
        return e_failure;
    if (strstr(argv[3], ".txt") == NULL &&
        strstr(argv[3], ".c") == NULL &&
        strstr(argv[3], ".h") == NULL &&
        strstr(argv[3], ".sh") == NULL)
        return e_failure;
    encInfo->secret_fname = argv[3];

    // Extract and store extension

    if (argv[4] != NULL)
    {
        if (strstr(argv[4], ".bmp") == NULL)
            return e_failure;
        encInfo->stego_image_fname = argv[4];
    }
    else
        encInfo->stego_image_fname = "stego.bmp";

    return e_success;
}

/* Open all required files */
Status open_files(EncodeInfo *encInfo)
{
    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");
    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);

        return e_failure;
    }

    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
    // Do Error handling
    if (encInfo->fptr_secret == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);

        return e_failure;
    }

    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");
    // Do Error handling
    if (encInfo->fptr_stego_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);

        return e_failure;
    }
    return e_success;
}


/* Check if source image has enough capacity */
Status check_capacity(EncodeInfo *encInfo)
{
    encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);
    encInfo->size_secret_file = get_file_size(encInfo->fptr_secret);
    char *extn = strstr(encInfo->secret_fname, ".");
    int extn_size = strlen(extn);

    int total_bytes = 54 + (strlen(MAGIC_STRING) * 8) + 32 +(extn_size * 8) + 32 +(encInfo->size_secret_file * 8);

    if (encInfo->image_capacity > total_bytes)
    {
        return e_success;
    }
    else
    {
        return e_failure;
    }
}

/* Copy BMP header (54 bytes) */
/*Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    char buffer[54];
    fseek(fptr_src_image,0,SEEK_SET);
    fread(buffer, 54, 1, fptr_src_image);
    fwrite(buffer, 54, 1, fptr_dest_image);
    return e_success;
}*/
Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    char buffer[54];
    rewind(fptr_src_image);

    fread(buffer, 54, 1, fptr_src_image);

    fwrite(buffer, 54, 1, fptr_dest_image);

    if (ftell(fptr_src_image) == ftell(fptr_dest_image))
    {
        return e_success;
    }
    else
    {
        return e_failure;
    }
}


/* Encode a single byte into 8 pixels’ LSBs */
Status encode_byte_to_lsb(char data, char *image_buffer)
{
    for (int i = 0; i < 8; i++)
    {
        image_buffer[i] = (image_buffer[i] & (~1)) | ((data >> i) & 1); //set lsb to data bit
    }
    return e_success;
}

/* Encode integer size into 32 pixels’ LSBs */
Status encode_size_to_lsb(int size, char *imageBuffer)
{
    for (int i = 0; i < 32; i++)
    {
        imageBuffer[i] = (imageBuffer[i] & (~1)) | ((size >> i) & 1); //set lsb to size bit
    }
    return e_success;
}

/* Encode magic string */
Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    // Move past BMP header
    fseek(encInfo->fptr_src_image, 54, SEEK_SET);

    unsigned char buffer[8];

    for (int i = 0; i < strlen(magic_string); i++)
    {
        // Read 8 bytes from source image
        if (fread(buffer, 1, 8, encInfo->fptr_src_image) != 8)
            return e_failure;

        // Encode 1 byte (magic_string[i]) into the 8 image bytes
        for (int bit = 7; bit >= 0; bit--)
        {
            buffer[7 - bit] = (buffer[7 - bit] & 0xFE) | ((magic_string[i] >> bit) & 1);
        }

        // Write modified bytes into stego image
        fwrite(buffer, 1, 8, encInfo->fptr_stego_image);
    }

    return e_success;
}

/*Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    char buffer[8];
    for (int i = 0; i < strlen(magic_string); i++)
    {
        fread(buffer, 8, 1, encInfo->fptr_src_image);
        encode_byte_to_lsb(magic_string[i], buffer);
        fwrite(buffer, 8, 1, encInfo->fptr_stego_image);
    }
    return e_success;
} */


/* Encode file extension size */
Status encode_secret_file_extn_size(int size, EncodeInfo *encInfo)
{
    char buffer[32];
    fread(buffer, 32, 1, encInfo->fptr_src_image);
    encode_size_to_lsb(size, buffer);
    fwrite(buffer, 32, 1, encInfo->fptr_stego_image);
    return e_success;
}

/* Encode file extension */
Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    char buffer[8];
    for (int i = 0; i < strlen(file_extn); i++)
    {
        fread(buffer, 8, 1, encInfo->fptr_src_image);
        encode_byte_to_lsb(file_extn[i], buffer);
        fwrite(buffer, 8, 1, encInfo->fptr_stego_image);
    }
    return e_success;
}

/* Encode secret file size */
Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{
    char buffer[32];
    fread(buffer, 32, 1, encInfo->fptr_src_image);
    encode_size_to_lsb(file_size, buffer);
    fwrite(buffer, 32, 1, encInfo->fptr_stego_image);
    return e_success;
}

/* Encode secret file data */
Status encode_secret_file_data(EncodeInfo *encInfo)
{
    rewind(encInfo->fptr_secret);
    fread(encInfo->secret_data, encInfo->size_secret_file, 1, encInfo->fptr_secret); //read secret file data to buffer 

    char buffer[8];
    for (long i = 0; i < encInfo->size_secret_file; i++)
    {
        fread(buffer, 8, 1, encInfo->fptr_src_image); 
        encode_byte_to_lsb(encInfo->secret_data[i], buffer); //encode secret data byte to lsb
        fwrite(buffer, 8, 1, encInfo->fptr_stego_image);
    }
    return e_success;
}

/* Copy the remaining image data */
Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    char ch;
    while (fread(&ch, 1, 1, fptr_src)) //read byte by byte till EOF
    {
        fwrite(&ch, 1, 1, fptr_dest);//write to stego image
    }
    return e_success;
}

/* Main encoding driver */
Status do_encoding(EncodeInfo *encInfo)
{
    if (open_files(encInfo) == e_failure)
    {
        printf("ERROR:Unable to open files\n");
        return e_failure;
    }

    if (check_capacity(encInfo) == e_failure)
    {
        printf("ERROR:Unable to check capacity\n");
        return e_failure;
    }
    if (copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_failure)
    {
        printf("ERROR:Unable to copy BMP header\n");
        return e_failure;
    }
    if (encode_magic_string(MAGIC_STRING, encInfo) == e_failure)
    {
        printf("ERROR:Unable to encode magic string\n");
        return e_failure;
    }

    int extn_size = strlen(encInfo->extn_secret_file);

    if (encode_secret_file_extn_size(extn_size, encInfo) == e_failure)
    {
        printf("ERROR:Unable to encode secret file extension size\n");
        return e_failure;
    }

    if (encode_secret_file_extn(encInfo->extn_secret_file, encInfo) == e_failure)
    {
        printf("ERROR:Unable to encode secret file extension\n");
        return e_failure;
    }

    if (encode_secret_file_size(encInfo->size_secret_file, encInfo) == e_failure)
    {
        printf("ERROR:Unable to encode secret file size\n");
        return e_failure;
    }

    if (encode_secret_file_data(encInfo) == e_failure)
    {
        printf("ERROR:Unable to encode secret file data\n");
        return e_failure;
    }

    if (copy_remaining_img_data(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_failure)
    {
        printf("ERROR:Unable to copy remaining image data\n");
        return e_failure;
    }

    return e_success;
fclose(encInfo->fptr_src_image);
fclose(encInfo->fptr_secret);
fclose(encInfo->fptr_stego_image);

}