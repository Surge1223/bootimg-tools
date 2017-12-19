/* tools/mkbootimg/bootimg.h
**
** Copyright 2007, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License"); 
** you may not use this file except in compliance with the License. 
** You may obtain a copy of the License at 
**
**     http://www.apache.org/licenses/LICENSE-2.0 
**
** Unless required by applicable law or agreed to in writing, software 
** distributed under the License is distributed on an "AS IS" BASIS, 
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
** See the License for the specific language governing permissions and 
** limitations under the License.
*/

#ifndef _BOOT_IMAGE_H_
#define _BOOT_IMAGE_H_

typedef struct boot_img_hdr boot_img_hdr;
typedef struct loki_hdr loki_hdr;
typedef struct sbl_hdr sbl_hdr;
typedef struct dtb_hdr dtb_hdr;

#define BOOT_MAGIC "ANDROID!"
#define BOOT_MAGIC_SIZE 8
#define BOOT_NAME_SIZE 16
#define BOOT_ARGS_SIZE 512
#define BOOT_EXTRA_ARGS_SIZE 1024
#define QCDT_MAGIC     "QCDT"  /* Master DTB magic */
#define QCDT_VERSION   3       /* QCDT version */
#define QCDT_DT_TAG    "qcom,msm-id        /* <"
#define QCDT_BOARD_TAG "qcom,board-id        /* <"
#define QCDT_PMIC_TAG  "qcom,pmic-id        /* <"
#define PAGE_SIZE_DEF  2048
#define PAGE_SIZE_MAX  (1024*1024)
#define log_err(x...)  printf(x)
#define log_info(x...) printf(x)
#define log_dbg(x...)  { if (verbose) printf(x); }
#define COPY_BLK       1024    /* File copy block size */
#define RC_SUCCESS     0
#define RC_ERROR       -1

#define SPARSE_HEADER_MAGIC	0xed26ff3a

#define CHUNK_TYPE_RAW		0xCAC1
#define CHUNK_TYPE_FILL		0xCAC2
#define CHUNK_TYPE_DONT_CARE	0xCAC3
#define CHUNK_TYPE_CRC		0xCAC4

struct chipInfo_t {
  uint32_t chipset;
  uint32_t platform;
  uint32_t subtype;
  uint32_t revNum;
  uint32_t pmic_model[4];
  uint32_t dtb_size;
  char     *dtb_file;
  struct chipInfo_t *prev;
  struct chipInfo_t *next;
  struct chipInfo_t *master;
  int      wroteDtb;
  uint32_t master_offset;
  struct chipInfo_t *t_next;
};

struct chipInfo_t *chip_list;

struct chipId_t {
  uint32_t chipset;
  uint32_t revNum;
  struct chipId_t *next;
  struct chipId_t *t_next;
};

struct chipSt_t {
  uint32_t platform;
  uint32_t subtype;
  struct chipSt_t *next;
  struct chipSt_t *t_next;
};

struct chipPt_t {
  uint32_t pmic0;
  uint32_t pmic1;
  uint32_t pmic2;
  uint32_t pmic3;
  struct chipPt_t *next;
  struct chipPt_t *t_next;
};
struct boot_img_hdr
{
    unsigned char magic[BOOT_MAGIC_SIZE];

    unsigned kernel_size;  /* size in bytes */
    unsigned kernel_addr;  /* physical load addr */

    unsigned ramdisk_size; /* size in bytes */
    unsigned ramdisk_addr; /* physical load addr */

    unsigned second_size;  /* size in bytes */
    unsigned second_addr;  /* physical load addr */

    unsigned tags_addr;    /* physical addr for kernel tags */
    unsigned page_size;    /* flash page size we assume */
    unsigned dt_size;      /* device tree in bytes */
    unsigned unused;       /* future expansion: should be 0 */
    unsigned char name[BOOT_NAME_SIZE]; /* asciiz product name */

    unsigned char cmdline[BOOT_ARGS_SIZE];

    unsigned id[8]; /* timestamp / checksum / sha1 / etc */
};


struct loki_hdr {
    unsigned char magic[4];     /* 0x494b4f4c */
    unsigned int recovery;      
    char build[128];   /* Build number */
    unsigned int orig_kernel_size;
    unsigned int orig_ramdisk_size;
    unsigned int ramdisk_addr;
};

struct sbl_hdr {
    unsigned sbl;              /* Sbl_Hdr  */
    unsigned sbl_codeword;       /* FLASH_CODE_WORD */
    unsigned sbl_magic;       /* MAGIC_NUM*/
    unsigned sbl_image_id;     
    unsigned sbl_image_src;       
    unsigned sbl_image_dest_ptr;       /* image_dest*/
    unsigned sbl_image_size;       /* image_size*/
    unsigned sbl_code_size;       /* code_size*/
    unsigned sbl_sig_ptr;       /* image_dest + code_size*/
    unsigned sbl_sig_size;       /* signature_size*/
    unsigned sbl_cert_chain_ptr;       /* image_dest + code_size + signature_size*/
    unsigned sbl_cert_chain_size;       /* cert_chain_size */
    unsigned sbl_oem_root_cert_sel;       
    unsigned sbl_oem_num_root_certs;      
};
    

struct dtb_hdr {
    unsigned char magic[4];     /* 0x494b4f4c */
    unsigned int version; 
    char build[128];   /* Build number */
    unsigned int orig_kernel_size;
    unsigned int orig_ramdisk_size;
    unsigned int dtbddr;
};
  
/*
** +-----------------+ 
** | boot header     | 1 page
** +-----------------+
** | kernel          | n pages  
** +-----------------+
** | ramdisk         | m pages  
** +-----------------+
** | second stage    | o pages
** +-----------------+
**
** n        /* (kernel_size + page_size - 1) / page_size
** m        /* (ramdisk_size + page_size - 1) / page_size
** o        /* (second_size + page_size - 1) / page_size
**
** 0. all entities are page_size aligned in flash
** 1. kernel and ramdisk are required (size != 0)
** 2. second is optional (second_size        /*= 0 -> no second)
** 3. load each element (kernel, ramdisk, second) at
**    the specified physical address (kernel_addr, etc)
** 4. prepare tags at tag_addr.  kernel_args[] is
**    appended to the kernel commandline in the tags.
** 5. r0        /* 0, r1        /* MACHINE_TYPE, r2        /* tags_addr
** 6. if second_size != 0: jump to second_addr
**    else: jump to kernel_addr
*/

#if 0
typedef struct ptentry ptentry;

struct ptentry {
    char name[16];      /* asciiz partition name    */
    unsigned start;     /* starting block number    */
    unsigned length;    /* length in blocks         */
    unsigned flags;     /* set to zero              */
};

/* MSM Partition Table ATAG
**
** length: 2 + 7 * n
** atag:   0x4d534d70
**         <ptentry> x n
*/
                                 size
   x      +------------------+
   |      | MAGIC ("QCDT")   |   4B
   |      +------------------+
 header   | VERSION          |   uint32 (version 3)
   |      +------------------+
   |      | num of DTBs      |   uint32 (number of DTB entries)
   x      +------------------+
   |      | platform id #1   |   uint32 (e.g. ID for MSM8974)
   |      +------------------+
   |      | variant id #1    |   uint32 (e.g. ID for CDP, MTP)
   |      +------------------+
   |      | subtype id #1    |   uint32 (e.g. ID for subtype) (QCDT v2)
 device   +------------------+
  #1      | soc rev #1       |   uint32 (e.g. MSM8974 v2)
 entry    +------------------+
   |      | pmic0 #1         |   uint32 (pmic0-> first smallest SID of existing pmic)
   |      +------------------+
   |      | pmic1 #1         |   uint32 (pmic1-> secondary smallest SID of existing pmic)
   |      +------------------+
   |      | pmic2 #1         |   uint32 (pmic2-> third smallest SID of existing pmic)
   |      +------------------+
   |      | pmic3 #1         |   uint32 (pmic3-> fourth smallest SID of existing pmic)
   |      +------------------+
   |      | offset #1        |   uint32 (byte offset from start/before MAGIC
   |      +------------------+           to DTB entry)
   |      | size #1          |   uint32 (size in bytes of DTB blob)
   x      +------------------+
   .              .
   .              .  (repeat)
   .              .

   x      +------------------+
   |      | platform id #Z   |   uint32 (e.g. ID for MSM8974)
   |      +------------------+
  device  | variant id #Z    |   uint32 (e.g. ID for CDP, MTP)
  #Z      +------------------+
  entry   | subtype id #Z    |   uint32 (e.g. ID for subtype) (QCDT v2)
  (last)  +------------------+
   |      | soc rev #Z       |   uint32 (e.g. MSM8974 v2)
   |      +------------------+
   |      | pmic0 #1         |   uint32 (pmic0-> first smallest SID of existing pmic)
   |      +------------------+
   |      | pmic1 #1         |   uint32 (pmic1-> secondary smallest SID of existing pmic)
   |      +------------------+
   |      | pmic2 #1         |   uint32 (pmic2-> third smallest SID of existing pmic)
   |      +------------------+
   |      | pmic3 #1         |   uint32 (pmic3-> fourth smallest SID of existing pmic)
   |      +------------------+
   |      | offset #Z        |   uint32 (byte offset from start/before MAGIC
   x      +------------------+           to DTB entry)
          | 0 ("zero")       |   uint32 (end of list delimiter)
          +------------------+           to DTB entry)
          | padding          |   variable length for next DTB to start on
          +------------------+           page boundary
          | DTB #1           |   variable (start is page aligned)
          |                  |
          |                  |
          +------------------+
          | padding          |   variable length for next DTB to start on
          +------------------+           page boundary
                  .
                  .
                  .

          +------------------+
          | DTB #Z (last)    |   variable (start is page aligned)
          |                  |
          |                  |
          +------------------+


           Below is notes provided for dex2oat header

           Notes on the interleaving of creating the image and oat file to
           ensure the references between the two are correct.
          
           Currently we have a memory layout that looks something like this:
          
           +--------------+
           | image        |
           +--------------+
           | boot oat     |
           +--------------+
           | alloc spaces |
           +--------------+
          
           There are several constraints on the loading of the image and boot.oat.
          
           1. The image is expected to be loaded at an absolute address and
           contains Objects with absolute pointers within the image.
          
           2. There are absolute pointers from Methods in the image to their
           code in the oat.
          
           3. There are absolute pointers from the code in the oat to Methods
           in the image.
          
           4. There are absolute pointers from code in the oat to other code
           in the oat.
          
           To get this all correct, we go through several steps.
          
           1. We prepare offsets for all data in the oat file and calculate
           the oat data size and code size. During this stage, we also set
           oat code offsets in methods for use by the image writer.
          
           2. We prepare offsets for the objects in the image and calculate
           the image size.
          
           3. We create the oat file. Originally this was just our own proprietary
           file but now it is contained within an ELF dynamic object (aka an .so
           file). Since we know the image size and oat data size and code size we
           can prepare the ELF headers and we then know the ELF memory segment
           layout and we can now resolve all references. The compiler provides
           LinkerPatch information in each CompiledMethod and we resolve these,
           using the layout information and image object locations provided by
           image writer, as were writing the method code.
          
           4. We create the image file. It needs to know where the oat file
           will be loaded after itself. Originally when oat file was simply
           memory mapped so we could predict where its contents were based
           on the file size. Now that it is an ELF file, we need to inspect
           the ELF file to understand the in memory segment layout including
           where the oat header is located within.
           TODO: We could just remember this information from step 3.
          
           5. We fixup the ELF program headers so that dlopen will try to
           load the .so at the desired location at runtime by offsetting the
           Elf32_Phdr.p_vaddr values by the desired base address.
           TODO: Do this in step 3. We already know the layout there.
          
           Steps 1.-3. are done by the CreateOatFile() above, steps 4.-5.
           are done by the CreateImageFile() below.
           
                     
#endif
          
#endif
