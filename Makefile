
TOOLCHAIN :=
CC=$(TOOLCHAIN)cc
AR=$(TOOLCHAIN)ar

CFLAGS  += -Wall -Wextra -Wno-unused-parameter -pipe
CFLAGS  += -std=c99 -D_GNU_SOURCE  -Ilibfdt -DANDROID
CFLAGS  += -Iinclude -fPIC -Ilibz -I./ -Ilibufdt
LDFLAGS := -Llibfdt/ -Llibs -lcrypto -lufdt_sysdeps
FDTLDFLAGS += -Llibz -Llibs -lcrypto
UFDTFLAGS += -lfdt -lufdt_sysdeps

BINDIR := bin
LIBDIR := libs

TARGETS := \
	mkbootimg/mkbootimg \
	mkbootimg/unmkbootimg \
	mkbootimg/unpackbootimg \
	mkbootimg/pkbootimg \
	mkbootimg/spbootimg \
	mkbootimg/mkqcdtbootimg \
	untargz/untgz \
	mkbootimg/dtbtool \
	mkbootimg/fdtextract \
	libufdt/utils/src/mkdtimg \
	libufdt/tests/src/extract_dtb


MKDTIMG_SRCS := \
	libufdt/utils/src/mkdtimg.c \
	libufdt/utils/src/mkdtimg_cfg_create.c \
	libufdt/utils/src/mkdtimg_core.c \
	libufdt/utils/src/mkdtimg_create.c \
	libufdt/utils/src/mkdtimg_dump.c \
	libufdt/utils/src/dt_table.c

STATIC_LIBS := \
       $(LIBDIR)/libcrypt.a \
       $(LIBDIR)/libfdt.a \
       $(LIBDIR)/libz.a \
       $(LIBDIR)/libufdt_sysdeps.a \
       $(LIBDIR)/libufdt.a

LIBUFDT_SYSDEPS_SRCS := libufdt/sysdeps/libufdt_sysdeps_posix.c

LIBUFDT_SRCS := \
       libufdt/ufdt_overlay.c \
       libufdt/ufdt_convert.c \
       libufdt/ufdt_node.c \
       libufdt/ufdt_node_pool.c \
       libufdt/ufdt_prop_dict.c \

LIBFDT_VERSION = version.lds

LIBFDT_soname = libfdt.$(SHAREDLIB_EXT).1

BOOTIMG_SRC_FILES := \
	mkbootimg/mkbootimg.c \
	mkbootimg/unmkbootimg.c \
	mkbootimg/unpackbootimg.c \
	mkbootimg/mkqcdtbootimg.c \
	mkbootimg/pkbootimg.c \
	mkbootimg/spbootimg.c \

UNTARGZ_SRC := \
        untargz/untgz.c

LIBZ_SRCS := \
	libz/adler32.c \
	libz/compress.c \
	libz/crc32.c \
	libz/deflate.c \
	libz/gzclose.c \
	libz/gzlib.c \
	libz/gzread.c \
	libz/gzwrite.c \
	libz/infback.c \
	libz/inflate.c \
	libz/inftrees.c \
	libz/inffast.c \
	libz/trees.c \
	libz/uncompr.c \
	libz/zutil.c

LIBFDT_SRCS := \
	libfdt/fdt.c \
	libfdt/fdt_ro.c \
	libfdt/fdt_wip.c \
	libfdt/fdt_sw.c \
	libfdt/fdt_rw.c \
	libfdt/fdt_strerror.c \
	libfdt/fdt_empty_tree.c

LCRYPYO_SRCS := \
        libmincrypt/sha.c \
	libmincrypt/rsa.c \
	libmincrypt/sha256.c

LIBFDT_INCLUDES := \
        libfdt/fdt.h \
	libfdt/libfdt.h \
	libfdt/libfdt_env.h

DTC_SRCS := \
	dtc/checks.c \
	dtc/data.c \
	dtc/dtc.c \
	dtc/flattree.c \
	dtc/fstree.c \
	dtc/livetree.c \
	dtc/srcpos.c \
	dtc/treesource.c \
	dtc/util.c

DTC_GEN_SRCS := dtc/dtc-lexer.lex.c dtc/dtc-parser.tab.c

EXTRACT_DTB_SRCS := \
        libufdt/tests/src/extract_dtb.c \
	libufdt/tests/src/util.c

EXTRACT_DTB_OBJS := $(EXTRACT_DTB_SRCS:%.c=%.o)
DTC_OBJS := $(DTC_SRCS:%.c=%.o) $(DTC_GEN_SRCS:%.c=%.o)
OBJ_FILES := $(BOOTIMG_SRC_FILES:%.c=%.o)
MKDTIMG_OBJS := $(MKDTIMG_SRCS:%.c=%.o)
CRYPTOBJ_FILES := $(LCRYPYO_SRCS:%.c=%.o)
LIBFDT_OBJS := $(LIBFDT_SRCS:%.c=%.o)
LIBZ_OBJS := $(LIBZ_SRCS:%.c=%.o)
LIBUFDT_SYSDEPS_OBJS := $(LIBUFDT_SYSDEPS_SRCS:%.c=%.o)
LIBUFDT_OBJS := $(LIBUFDT_SRCS:%.c=%.o)

all: libcrypt libfdt libz libufdt_sysdeps libufdt $(libs) mkbootimg unmkbootimg unpackbootimg pkbootimg spbootimg \
	mkqcdtbootimg untgz  dtbtool fdtextract mkdtimg extract_dtb $(install)

libcrypt: $(CRYPTOBJ_FILES)
	$(AR) rs $(LIBDIR)/libcrypt.a $^

libz: $(LIBZ_OBJS)
	$(AR) rs $(LIBDIR)/libz.a $^

libfdt: $(LIBFDT_OBJS)
	$(AR) rs $(LIBDIR)/libfdt.a $^

libufdt_sysdeps: $(LIBUFDT_SYSDEPS_OBJS)
	$(CC) $(CFLAGS) -c $(LIBUFDT_SYSDEPS_SRCS)
	$(AR) rs $(LIBDIR)/libufdt_sysdeps.a $^

libufdt: $(LIBFDT_OBJS) $(LIBUFDT_SYSDEPS_OBJS) $(LIBUFDT_OBJS)
	$(CC) -c $(CFLAGS) $(LIBUFDT_SRCS) $(FDTLDFLAGS) $(UFDTFLAGS)
	$(AR) rs $(LIBDIR)/libufdt.a $^

mkbootimg:  $(CRYPTOBJ_FILES) $(LIBFDT_OBJS) mkbootimg/mkbootimg.o
	$(CC) -o $(BINDIR)/$@ $^ $(LDFLAGS) $(CFLAGS)

untgz:  $(LIBZ_OBJS) untargz/untgz.o
	$(CC) -o $(BINDIR)/$@ $^ $(LDFLAGS) $(CFLAGS)

unmkbootimg: mkbootimg/unmkbootimg.o
	$(CC) -o $(BINDIR)/$@ $^ $(LDFLAGS) $(CFLAGS)

unpackbootimg: mkbootimg/unpackbootimg.o
	$(CC) -o $(BINDIR)/$@ $^ $(LDFLAGS) $(CFLAGS)

pkbootimg: $(CRYPTOBJ_FILES) mkbootimg/pkbootimg.o
	$(CC) -o $(BINDIR)/$@ $^ $(LDFLAGS) $(CFLAGS)

spbootimg: mkbootimg/spbootimg.o
	$(CC) -o $(BINDIR)/$@ $^ $(LDFLAGS) $(CFLAGS)

mkqcdtbootimg: $(LIBFDT_OBJS) $(CRYPTOBJ_FILES) mkbootimg/mkqcdtbootimg.o
	$(CC) -o $(BINDIR)/$@ $^ $(FDTLDFLAGS) $(CFLAGS)

dtbtool: $(LIBFDT_OBJS) $(CRYPTOBJ_FILES) mkbootimg/dtbtool.o
	$(CC) -o $(BINDIR)/$@ $^ $(FDTLDFLAGS) $(CFLAGS)

fdtextract: $(LIBFDT_OBJS) $(CRYPTOBJ_FILES) mkbootimg/fdtextract.o
	$(CC) -o $(BINDIR)/$@ $^ $(FDTLDFLAGS) $(CFLAGS)

mkdtimg: $(LIBFDT_OBJS) $(MKDTIMG_OBJS)  libufdt/utils/src/mkdtimg.o
	$(CC) -o $(BINDIR)/$@ $^ $(FDTLDFLAGS) $(UFDTFLAGS)  $(CFLAGS)

extract_dtb: $(LIBFDT_OBJS) $(EXTRACT_DTB_OBJS) libufdt/tests/src/extract_dtb.o
	$(CC) -o $(BINDIR)/$@ $^ $(FDTLDFLAGS) $(UFDTFLAGS)  $(CFLAGS)

#mkbootfs:  mkbootimg/mkbootfs.o
#	$(CC) -o $(BINDIR)/$@ $^ $(LDFLAGS) $(CFLAGS)

libs: $(STATIC_LIBS)
	for file in $(STATIC_LIBS) ; do \
			mv $$file $(LIBDIR)/; done

testbin := $(shell test -d $(BINDIR) || mkdir -p $(BINDIR))

testlib := $(shell test -d $(LIBDIR) || mkdir -p $(LIBDIR))

install: $(testbin) $(testlib) $(TARGETS)
	for file in `find ./ -name "$(TARGETS)" `; do \
			mv $$file $(BINDIR)/; done

clean:
	rm -rf $(LIBDIR)/* $(BINDIR)/* **/*.o **/*.a *.o bin libs
	rm -rf `find $(PWD) -name "*.o"`

.PHONY: clean
