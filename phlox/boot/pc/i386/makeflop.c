/*
* Copyright 2007-2013, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BOOTCODESIZE 1024
#define BUFFERSIZE (100*1024)
#define LOBYTE(x)  (x & 0x00ff)
#define HIBYTE(x) ((x & 0xff00) >> 8)

void usage(char const *progname)
{
    printf("usage: %s [-b <block size for align>] [-v (enable vesa)] [-w xres] [-h yres] [-d bit_depth] [-t (disable vesa)] [-r <retry count on disk read error>] bootblock payload outfile\n", progname);
}

int main(int argc, char *argv[])
{
   FILE *btbl, *pld, *outf;
   int use_vesa;
   int vesa_width;
   int vesa_height;
   int vesa_depth;
   int bsize;
   int retry;
   unsigned char bootcode[BOOTCODESIZE];
   unsigned char *buffer;
   signed char opt;
   size_t payld_size;
   size_t total_bytes;
   size_t align;
   size_t bytes_read;
   unsigned int blocks;
   char const *progname;

   /* set defaults */
   progname    = argv[0];
   use_vesa    = -1;
   vesa_width  = 640;
   vesa_height = 480;
   vesa_depth  = 16;
   bsize       = -1;
   retry       = -1;

   /* parse command line */
   while( (opt = getopt(argc, argv, "b:vtw:h:d:r:")) != -1)
       switch(opt) {
          case 'b':
            bsize = atoi(optarg);
            if(bsize<=0) {
              fprintf(stderr, "error: invalid block size '%s'\n", optarg);
              return -1;
            }
            break;
          case 'v':
             use_vesa = 1;
            break;
          case 't':
             use_vesa = 0;
            break;
          case 'w':
            vesa_width = atoi(optarg);
            if(vesa_width<=0) {
              fprintf(stderr, "error: invalid xres '%s'\n", optarg);
              return -1;
            }
            break;
          case 'h':
            vesa_height = atoi(optarg);
            if(vesa_height<=0) {
              fprintf(stderr, "error: invalid yres '%s'\n", optarg);
              return -1;
            }
            break;
          case 'd':
            vesa_depth = atoi(optarg);
            if(vesa_depth<=0) {
              fprintf(stderr, "error: invalid bit depth '%s'\n", optarg);
              return -1;
            }
            break;
          case 'r':
            retry = atoi(optarg);
            if(retry<0) {
              fprintf(stderr, "error: invalid retry count '%s'\n", optarg);
              return -1;
            }
            break;
          case '?':
          default:
             usage(progname);
             return -1;
       }
   argc -= optind - 1;
   argv += optind -1;

   if(argc<4) {
      printf("insufficient args\n");
      usage(progname);
      return -1;
   }

   /* open boot block file */
   if( (btbl=fopen(argv[1], "rb")) == NULL ) {
      fprintf(stderr, "error: cannot open bootblock file '%s'\n", argv[1]);
      return -1;
   }

   /* check size */
   fseek(btbl, 0, SEEK_END);
   if(ftell(btbl) != BOOTCODESIZE) {
      fprintf(stderr, "error: bootblock damaged or incorrect\n");
      fclose(btbl);
      return -1;
   }
   fseek(btbl, 0, SEEK_SET);
   fread(bootcode, 1, BOOTCODESIZE, btbl); /* read boot block file */
   fclose(btbl);                           /* and close it */

   /* open payload file */
   if( (pld=fopen(argv[2], "rb")) == NULL ) {
      fprintf(stderr, "error: cannot open input file '%s'\n", argv[2]);
      return -1;
   }
   /* get payload size and total bytes to write */
   fseek(pld, 0, SEEK_END);
   payld_size = ftell(pld);
   fseek(pld, 0, SEEK_SET);
   total_bytes = BOOTCODESIZE + payld_size;

   /* open output file */
   if( (outf=fopen(argv[3], "wb")) == NULL ) {
      fprintf(stderr, "error: cannot open output file '%s'\n", argv[3]);
      fclose(pld);
      return -1;
   }

   /* patch boot block */
   blocks = payld_size / 512;
   if(payld_size % 512) blocks++;
   /* first sector */
   bootcode[2] = LOBYTE(blocks); /* byte 3 */
   bootcode[3] = HIBYTE(blocks); /* byte 4 */
   printf("payload: size %ld, blocks %u (size %u)\n", payld_size, blocks, blocks*512);

   /* patch second sector */
   if(use_vesa >= 0)
     if(use_vesa) {
         bootcode[512] = 1;                   /* byte 1 */
         bootcode[513] = LOBYTE(vesa_width);  /* byte 2 */
         bootcode[514] = HIBYTE(vesa_width);  /* byte 3 */
         bootcode[515] = LOBYTE(vesa_height); /* byte 4 */
         bootcode[516] = HIBYTE(vesa_height); /* byte 5 */
         bootcode[517] = LOBYTE(vesa_depth);  /* byte 6 */
         printf("VESA enabled, mode: %dx%dx%d\n", vesa_width, vesa_height, vesa_depth);
     } else {
         bootcode[512] = 0;
         printf("VESA disabled\n");
     }

   if(retry > 0) {
      bootcode[518] = LOBYTE(retry); /* byte 7 of second sector */
      printf("Retries count on read error: %d\n", retry);
   }


   buffer = (unsigned char *)malloc(BUFFERSIZE);
   if(!buffer) {
     fprintf(stderr, "error: insufficient memory\n");
     fclose(pld);
     fclose(outf);
     return -1;
   }

   /* write output */
   fwrite(bootcode, 1, BOOTCODESIZE, outf);
   while( bytes_read=fread(buffer, 1, BUFFERSIZE, pld) )
       fwrite(buffer, 1, bytes_read, outf);
   fclose(pld);
   free(buffer);

   /* align size if requested */
   if(bsize>0)
      if(total_bytes%bsize) {
         align = bsize - total_bytes % bsize;
         buffer = (unsigned char *)malloc(align);
         memset(buffer, 'U', align);
         total_bytes += align;
         fwrite(buffer, 1, align, outf);
         free(buffer);
         printf("%ld bytes added to output file for align\n", align);
      }

   fclose(outf);

   printf("Done. %ld bytes written to %s.\n", total_bytes, argv[3]);

   return 0;
}
