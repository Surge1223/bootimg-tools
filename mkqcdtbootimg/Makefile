include libfdt/Makefile.libfdt

TOOLCHAIN :=
CC=$(TOOLCHAIN)gcc

CFLAGS  += -Wall -Wextra -Wno-unused-parameter -pedantic -pipe
CFLAGS  += -std=c99 -D_GNU_SOURCE  -Ilibfdt
CFLAGS  += -Iinclude -fPIC -static
LDFLAGS := -lcrypto


	
LIBFDT_VERSION = version.lds
LIBFDT_OBJS = $(LIBFDT_SRCS:%.c=%.o)
LIBFDT_soname = libfdt.$(SHAREDLIB_EXT).1

BOOTIMG_SRC_FILES := \
	mkbootimg/mkbootimg.c \
	mkbootimg/unmkbootimg.c \
	mkbootimg/unpackbootimg.c \
	mkbootimg/mkqcdtbootimg.c \
	mkbootimg/pkbootimg.c \
	mkbootimg/spbootimg.c 
	

LIBFDT_SRCS := \
	libfdt/fdt.c \
	libfdt/fdt_ro.c \
	libfdt/fdt_wip.c \
	libfdt/fdt_sw.c \
	libfdt/fdt_rw.c \
	libfdt/fdt_strerror.c \
	libfdt/fdt_empty_tree.c 
	

LIBFDT_INCLUDES := \
		libfdt/fdt.h \
		libfdt/libfdt.h \
		libfdt/libfdt_env.h

OBJ_FILES := $(BOOTIMG_SRC_FILES:%.c=%.o)

all: mkbootimg/mkbootimg mkbootimg/unmkbootimg mkbootimg/unpackbootimg \
	mkbootimg/pkbootimg mkbootimg/spbootimg mkbootimg/mkqcdtbootimg

$(MODULE): $(OBJ_FILES)
	$(CC) -o $@ $^ $(LDFLAGS) $(CFLAGS)

clean:
	$(RM) $(OBJ_FILES) $(MODULE)

.PHONY: clean
