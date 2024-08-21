#ifndef COMMON_H
#define COMMON_H

/* Magic string to identify whether stegged or not */
#define MAGIC_STRING "#*"  //2 bytes of data

#endif

// To read the image hex value
// Format-Hex -Path .\stego.bmp | Select-Object -First 10 
// Format-Hex -Path .\beautiful.bmp | Select-Object -First 10 