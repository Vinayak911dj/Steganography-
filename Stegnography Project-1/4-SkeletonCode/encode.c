#include <stdio.h>
#include "encode.h"
#include "types.h"
#include<string.h>
#include "common.h"

/* Function Definitions */

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */
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

/* 
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */
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

    // No failure return e_success
    return e_success;
}

Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    if(strcmp(strstr(argv[2],"."),".bmp")==0)
    {
        encInfo-> src_image_fname=argv[2];
    }
    else
    {
        return e_failure;
    }
    if(strcmp(strstr(argv[3],"."),".txt") == 0)
    {
        encInfo-> secret_fname = argv[3];
    }
    else
    {
        return e_failure;
    }
    if(argv[4] !=NULL)
    {
        encInfo->stego_image_fname = argv[4];
    }
    else
    {
        encInfo->stego_image_fname = "stego.bmp";
    }
    return e_success;
}

uint get_file_size(FILE *fptr)
{
    fseek(fptr,0,SEEK_SET);
    fseek(fptr,0,SEEK_END);
    return ftell(fptr);
}

Status check_capacity(EncodeInfo *encInfo)
{
    encInfo-> image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image ); 
    encInfo->size_secret_file=get_file_size(encInfo->fptr_secret );

    fseek(encInfo->fptr_secret,0,SEEK_SET);

    if(encInfo->image_capacity >= 54+16+32+32+32+(8 * encInfo->size_secret_file))
    {
        return e_success ;
    }
    else
    {
        return e_failure;
    }
}

Status copy_bmp_header(FILE *fptr_src_image,FILE *fptr_dest_image)
{
    char str[54];
    fseek(fptr_src_image,0,SEEK_SET);
    fread(str,sizeof(char),54,fptr_src_image);
    fwrite(str,sizeof(char),54,fptr_dest_image);
    return e_success;
}

Status encode_magic_string(const char *magic_string, EncodeInfo *encinfo)
{
    encode_data_to_image(magic_string, strlen(magic_string),encinfo-> fptr_src_image, encinfo->fptr_stego_image);
    return e_success;
}

Status encode_data_to_image(const char *data, int size, FILE *fptr_src_image, FILE *fptr_stego_image)
{
    char str[8];
    for(int i=0;i<size;i++)
    {
        fread(str,sizeof(char),8,fptr_src_image);
        encode_byte_to_lsb(data[i],str);
        fwrite(str,sizeof(char),8,fptr_stego_image);
    }
}


Status encode_byte_to_lsb(char data,char *image_buffer)
{
     for (int i=0;i<8;i++)
     {
        image_buffer[i] = (image_buffer[i] & 0xFE) | ((data >> (7-i)) & 0x01);
     }
    //  return e_success;
    // unsigned int mask = 1 <<7;
    // int i;
    // for(i=0;i<=7;i++)
    // {
    //     image_buffer[i]=(image_buffer[i] & 0xFE) | ((data & mask) >> (7-i)) ;
    //     mask=mask>>1;
    // }
    return e_success;
}

Status encode_secret_file_extn_size(int file_extn_size, FILE *fptr_src_image, FILE *fptr_stego_image)
{
    char arr[32];
    fread(arr,32,1,fptr_src_image);
    encode_size_to_lsb(file_extn_size,arr);
    fwrite(arr,1,32,fptr_stego_image);  
    return e_success;
}

Status encode_size_to_lsb(int data, char *image_buffer)
{
    unsigned int mask = 1 <<31;
    int i;
    for(i=0;i<=31;i++)
    {
        image_buffer[i]=(image_buffer[i] & 0xFE) | ((data & mask) >> (31-i)) ;
        mask=mask>>1;
    }
    return e_success;
}

Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    encode_data_to_image(file_extn, strlen(file_extn), encInfo -> fptr_src_image, encInfo -> fptr_stego_image);
    return e_success;
}

Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{
    char arr[32];
    fread(arr,32,1,encInfo->fptr_src_image);
    encode_size_to_lsb(file_size,arr);
    fwrite(arr,32,1,encInfo->fptr_stego_image);
    return e_success;
}

Status encode_secret_file_data(EncodeInfo *encInfo)
{
    char ch;
    int i;
    for(i=0;i<encInfo->size_secret_file;i++)
    {
        fread(encInfo->image_data,8,1,encInfo->fptr_src_image);
        fread(&ch,1,1,encInfo->fptr_secret);
        encode_byte_to_lsb(ch,encInfo->image_data);
        fwrite(encInfo->image_data,8,1,encInfo->fptr_stego_image);
    }
    return e_success;
}

Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    char ch;
    while(fread(&ch, 1,1,fptr_src) >0)
    {
        fwrite(&ch,1,1,fptr_dest);
    }
    return e_success;
}

Status do_encoding(EncodeInfo *encInfo)
{
    if(open_files(encInfo) == e_success)
    {
        printf("Success: Open File function\n");
        if(check_capacity(encInfo) == e_success)
        {
            printf("Success: Check_capacity\n");
            if(copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_success)
            {
                printf("Success: Copy .bmp header\n");
                if(encode_magic_string(MAGIC_STRING, encInfo) == e_success)
                {
                    printf("Success: Encode magic String\n");
                    strcpy(encInfo -> extn_secret_file,(strstr(encInfo -> secret_fname,".")));
                    if(encode_secret_file_extn_size(strlen(encInfo -> extn_secret_file),encInfo -> fptr_src_image,encInfo -> fptr_stego_image) == e_success)
                    {
                        printf("Success: Encoded secret file extension\n");
                        if(encode_secret_file_extn(encInfo->extn_secret_file, encInfo) == e_success)
                        {
                            printf("Success: Encoding secret file Extension\n");
                            if(encode_secret_file_size(encInfo->size_secret_file,encInfo) == e_success)
                            {
                                printf("Success: Encoding Secret file size\n");
                                if(encode_secret_file_data(encInfo) == e_success)
                                {
                                    printf("Success: Encoding secret file data\n");
                                    if(copy_remaining_img_data(encInfo->fptr_src_image,encInfo->fptr_stego_image)==e_success)
                                    {
                                        printf("Success: Copied Remaiming Data\n");
                                        printf("Encoding Done Successfuly!\n");
                                    }
                                    else
                                    {
                                        printf("Failure: Copied Remaiming Data\n");
                                        return -1;
                                    }
                                }
                                else
                                {
                                    printf("Failure: Encoding secret file data\n");
                                    return -1;
                                }
                            }
                            else
                            {
                                printf("Failure: Encoding Secret file size\n");
                                return -1;
                            }
                        }
                        else
                        {
                            printf("Failure: Encoding secret file Extension\n");
                            return -1;
                        }
                    }
                    else
                    {
                        printf("Failure: Encoding secret file extension\n");
                        return -1;
                    }
                }
                else
                {
                    printf("Failure: Encode magic String\n");
                    return -1;
                }
            }
            else
            {
                printf("Failure: Copy .bmp header\n");
                return -1;
            }
        }
        else
        {
            printf("Failure: Check Capacity\n");
            return -1;
        }
    }
    else
    {
       printf("Failure: Open Files function\n");
       return -1; 
    }
}