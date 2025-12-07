#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "decode.h"
#include "types.h"
#include "common.h"

// Function declaration
OperationType check_operation_type(char *symbol);

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("Usage:\n");
        printf("  For Encoding: ./steg -e <source_image.bmp> <secret.txt> [output_stego.bmp]\n"); 
        printf("  For Decoding: ./steg -d <stego_image.bmp> [output.txt]\n");
        return 1;
    }

    // Identify operation type: encode or decode
    OperationType op_type = check_operation_type(argv[1]);

    if (op_type == e_encode)
    {
        printf("INFO: Selected Encoding...\n");

        EncodeInfo encInfo;

        if (read_and_validate_encode_args(argv, &encInfo) == e_success) //validate args
        {
            if (do_encoding(&encInfo) == e_success)
            {
        
                printf("INFO: Encoding completed successfully!\n");
            }
            else
            {
                printf("ERROR: Encoding failed.\n");
            }
        }
        else
        {
            printf("ERROR: Invalid encoding arguments.\n");
            return e_failure;
        }
    }
    else if (op_type == e_decode) //decode the stego image
    {
        printf("INFO: Selected Decoding...\n");

        DecodeInfo decInfo;

        if (read_and_validate_decode_args(argv, &decInfo) == e_success)
        {
            if (do_decoding(&decInfo) == e_success)
            {
                printf("INFO: Decoding completed successfully!\n");
            }
            else
            {
                printf("ERROR: Decoding failed.\n");
            }
        }
        else
        {
            printf("ERROR: Invalid decoding arguments.\n");
            return e_failure;
        }
    }
    else
    {
        printf("ERROR: Unsupported operation. Use -e for encode or -d for decode.\n");
        return e_failure;
    }

    return e_success;
}

/* Function: check_operation_type
 * Purpose : Identify whether operation is encode or decode
 */
OperationType check_operation_type(char *symbol)
{
    if (strcmp(symbol, "-e") == 0)
    {
        return e_encode;
    }
    else if (strcmp(symbol, "-d") == 0)
    {
        return e_decode;
    }
    else
    {
        return e_unsupported;
    }
}
