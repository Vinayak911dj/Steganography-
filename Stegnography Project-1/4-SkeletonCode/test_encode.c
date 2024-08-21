#include <stdio.h>
#include "encode.h"
#include "types.h"
#include<string.h>
int main(int argc, char **argv)
{
    if (check_operation_type(argv) == e_encode)
    {
        printf("Selected Encoding\n");
        EncodeInfo encInfo;

        if(read_and_validate_encode_args(argv, &encInfo) == e_success)
        {
            printf("Success: Read and validate Function\n");

            if(do_encoding(&encInfo) == e_success)
            {
                printf("Encoding Finished\n");
            }
            else
            {
                printf("Encoding Failed\n");
                return -1;
            }
        }
        else
        {
            printf("Failure: Read and validate function\n ");
            return -1;
        }
    }
    else if(check_operation_type(argv) == e_decode)
    {
        printf("Selcted Decoding\n");
    }
    else
    {
        printf("Invalid Opration\n");
        printf("*******************************************\n");
        printf("try this Encoding: ./a.out -e beautiful.bmp secret.bmp stego.bmp\n");
        printf("or try this Decoding: ./a.out -d stego.bmp output.bmp\n");
    }  
    return 0;
}

OperationType check_operation_type(char **argv)
{
    if(strcmp(argv[1],"-e")==0)
        return e_encode;
    
    else if (strcmp(argv[1],"-d")==0)
        return e_decode;
    else
        return e_unsupported;
}