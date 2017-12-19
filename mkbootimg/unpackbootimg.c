#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <limits.h>
#include <libgen.h>

#include "mincrypt/sha.h"
#include "bootimg2.h"

typedef unsigned char byte;

int read_padding(FILE* f, unsigned itemsize, int pagesize)
{
    byte* buf = (byte*)malloc(sizeof(byte) * pagesize);
    unsigned pagemask = pagesize - 1;
    unsigned count;

    if((itemsize & pagemask) == 0) {
        free(buf);
        return 0;
    }

    count = pagesize - (itemsize & pagemask);

    fread(buf, count, 1, f);
    free(buf);
    return count;
}

void write_string_to_file(char* file, char* string)
{
    FILE* f = fopen(file, "w");
    fwrite(string, strlen(string), 1, f);
    fwrite("\n", 1, 1, f);
    fclose(f);
}

void append_to_file(char* file, char* string)
{
    FILE* f = fopen(file, "a");
    fwrite(string, strlen(string), 1, f);
    fwrite("\n", 1, 1, f);
    fclose(f);
}

static unsigned save_file(const char *fn, const void* data, const unsigned sz)
{
    FILE *fd;
    unsigned _sz = 0;

    fd = fopen(fn, "wb");
    if(fd == 0) return 0;

    _sz = fwrite(data, 1, sz, fd);
    fclose(fd);

    return _sz;
}


int usage() {
    printf("usage: unpackbootimg\n");
    printf("\t-i|--input boot.img\n");
    printf("\t-n|--name device name\n");
    printf("\t[ -o|--output output_directory]\n");
    printf("\t[ -p|--pagesize <size-in-hexadecimal> ]\n");
    return 0;
}

int main(int argc, char** argv)
{
    boot_img_hdr hdr;
    char tmp[PATH_MAX];
    char* directory = "./";
    char* filename = NULL;
    char* devicename = NULL;
    int pagesize = 0;
    void *fd = 0;
    unsigned fs = 0;
    int base = 0;

    char *kernel_fn = "kernel";
    char *ramdisk_fn = "ramdisk.cpio.gz";
    char *second_fn = "second_bootloader";
    char *bootimg = 0;
    unsigned offset;
    
    argc--;
    argv++;
    

    while(argc > 0){
        char *arg = argv[0];
        char *val = argv[1];
        argc -= 2;
        argv += 2;
        if(!strcmp(arg, "--input") || !strcmp(arg, "-i")) {
            filename = val;
        } else if(!strcmp(arg, "--name") || !strcmp(arg, "-n")) {
            devicename = val;
        } else if(!strcmp(arg, "--output") || !strcmp(arg, "-o")) {
            directory = val;
        } else if(!strcmp(arg, "--pagesize") || !strcmp(arg, "-p")) {
            pagesize = strtoul(val, 0, 16);
        } else {
            return usage();
        }
    }
	hdr.page_size = pagesize;


    if (filename == NULL) {
        return usage();
    }
    
    int total_read = 0;
    FILE* f = fopen(filename, "rb");

    loki_hdr lhdr;
    //printf("Reading hdr...\n");
    int i;
    for (i = 0; i <= 512; i++) {
        fseek(f, i, SEEK_SET);
        fread(tmp, BOOT_MAGIC_SIZE, 1, f);
        if (memcmp(tmp, BOOT_MAGIC, BOOT_MAGIC_SIZE) == 0)
            break;
    }
    total_read = i;
    if (i > 512) {
        printf("Android boot magic not found.\n");
        return 1;
    }
    fseek(f, i, SEEK_SET);
    printf("Android magic found at: %d\n", i);
    fread(&hdr, sizeof(hdr), 1, f);
    printf("BOARD_KERNEL_CMDLINE :%s\n", hdr.cmdline);
    printf("BOARD_KERNEL_BASE :%08x\n", hdr.kernel_addr - 0x00008000);
    printf("BOARD_KERNEL_OFFSET :%08x\n",hdr.kernel_addr);
    printf("BOARD_RAMDISK_OFFSET :%08x\n", hdr.ramdisk_addr - base);
    printf("BOARD_SECOND_OFFSET :%08x\n", hdr.second_addr - hdr.kernel_addr + 0x00008000);
    printf("BOARD_KERNEL_TAGS_OFFSET :%08x\n",hdr.tags_addr - base);
    printf("BOARD_KERNEL_TAGS_OFFSET :%08x\n",hdr.tags_addr - base);
    printf("BOARD_KERNEL_PAGESIZE :%d\n", hdr.page_size);
    printf("BOARD_PAGE_SIZE %d\n", hdr.page_size);
    printf("BOARD_SECOND_SIZE :%d\n", hdr.second_size);
    printf("BOARD_DT_SIZE :%d\n", hdr.dt_size);
    

    if (pagesize == 0) {
        pagesize = hdr.page_size;
    }

    //printf("cmdline...\n");
    if (devicename == NULL) {
        devicename == filename;
    }

    char cmdline[4000];
    sprintf(tmp, "%s/%s", directory, basename(devicename));
    strcat(tmp, cmdline);
    sprintf(cmdline, "BOARD_KERNEL_CMDLINE :%s\n", hdr.cmdline);
    write_string_to_file(tmp, cmdline);
    

    //printf("base...\n");
    sprintf(tmp, "%s/%s", directory, basename(devicename));
    sprintf(cmdline, "BOARD_KERNEL_BASE :%08x\n", hdr.kernel_addr - 0x00008000);
    append_to_file(tmp, cmdline);

    //printf("ramdisk_offset...\n");
    sprintf(tmp, "%s/%s", directory, basename(devicename));
    sprintf(cmdline, "BOARD_KERNEL_OFFSET :%08x\n", hdr.kernel_addr + 0x00008000);
    append_to_file(tmp, cmdline);


    //printf("ramdisk_offset...\n");
    sprintf(tmp, "%s/%s", directory, basename(devicename));
    sprintf(cmdline, "BOARD_RAMDISK_OFFSET :%08x\n", hdr.ramdisk_addr - base);
    append_to_file(tmp, cmdline);

    //printf("second_offset...\n");
    sprintf(tmp, "%s/%s", directory, basename(devicename));
    sprintf(cmdline, "BOARD_SECOND_OFFSET :%08x\n", hdr.second_addr - hdr.kernel_addr + 0x00008000);
    append_to_file(tmp, cmdline);

    //printf("tags_offset...\n");
    sprintf(tmp, "%s/%s", directory, basename(devicename));
    sprintf(cmdline, "BOARD_KERNEL_TAGS_OFFSET :%08x\n", hdr.tags_addr - base);
    append_to_file(tmp, cmdline);

    //printf("pagesize...\n");
    sprintf(tmp, "%s/%s", directory, basename(devicename));
    sprintf(cmdline, "BOARD_KERNEL_PAGESIZE :%d\n", hdr.page_size);
    append_to_file(tmp, cmdline);
    
    //printf("second_size...\n");
    sprintf(tmp, "%s/%s", directory, basename(devicename));
    sprintf(cmdline, "BOARD_KERNEL_SECOND_SIZE :%d\n", hdr.second_size);
    append_to_file(tmp, cmdline);

    //printf("dt_size...\n");
    sprintf(tmp, "%s/%s", directory, basename(devicename));
    sprintf(cmdline, "BOARD_KERNEL_DT_SIZE :%d\n", hdr.dt_size);
    append_to_file(tmp, cmdline);
   

    //printf("total read: %d\n", total_read);
    total_read += read_padding(f, sizeof(hdr), pagesize);

    sprintf(tmp, "%s/%s", directory, basename(kernel_fn));
    strcat(tmp, "");
    FILE *k = fopen(tmp, "wb");
    byte* kernel = (byte*)malloc(hdr.kernel_size);
    //printf("Reading kernel...\n");
    fread(kernel, hdr.kernel_size, 1, f);
    total_read += hdr.kernel_size;
    fwrite(kernel, hdr.kernel_size, 1, k);
    fclose(k);

    total_read += read_padding(f, hdr.kernel_size, pagesize);
    //printf("total read: %d\n", hdr.kernel_size);
    sprintf(tmp, "%s/%s", directory, basename(ramdisk_fn));
    strcat(tmp, "" );
    FILE *r = fopen(tmp, "wb");
    byte* ramdisk = (byte*)malloc(hdr.ramdisk_size);
    //printf("Reading ramdisk...\n");
    fread(ramdisk, hdr.ramdisk_size, 1, f);
    total_read += hdr.ramdisk_size;
    fwrite(ramdisk, hdr.ramdisk_size, 1, r);

    fclose(r);

if (hdr.second_size != 0 ) {
    total_read += read_padding(f, hdr.ramdisk_size, pagesize);
    sprintf(tmp, "%s/%s", directory, basename(filename));
    strcat(tmp, "-second");
    FILE *s = fopen(tmp, "wb");
    byte* second = (byte*)malloc(hdr.second_size);
    //printf("Reading second...\n");
    fread(second, hdr.second_size, 1, f);
    total_read += hdr.second_size;
    fwrite(second, hdr.second_size, 1, s);
    fclose(s);

} else {
printf("no second size  detected\n");
   }    
if (hdr.dt_size != 0 ) {
    total_read += read_padding(f, hdr.second_size, pagesize);
    sprintf(tmp, "%s/%s", directory, basename(filename));
    strcat(tmp, "-dt.img");
    FILE *d = fopen(tmp, "wb");
    byte* dt = (byte*)malloc(hdr.dt_size);
    //printf("Reading dt...\n");
    fread(dt, hdr.dt_size, 1, f);
    total_read += hdr.dt_size;
    fwrite(dt, hdr.dt_size, 1, d);
    fclose(d);

} else {
printf("no dt.img detected\n");
   } 
    fclose(f);
    
    //printf("Total Read: %d\n", total_read);
    return 0;
}
