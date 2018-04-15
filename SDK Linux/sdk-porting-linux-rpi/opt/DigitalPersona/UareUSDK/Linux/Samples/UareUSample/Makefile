EXE_NAME = UareUSample

SDK_INCLUDES = ../../../Include
OUT_DIR ?= .
LIB_OUT_DIR ?= /usr/lib

CCFLAGS = -g -Wall -I$(SDK_INCLUDES) $(CFLAGS)

# $(call getlink, linkname)
getlink = $(shell cygpath -wm `readlink -f $(1)`)

ifeq ($(findstring arm, $(CFLAGS))$(findstring CYGWIN, $(shell uname)),armCYGWIN)
	#Code Sourcery toolchain under Cygwin cannot dereference symbolic links, need to specify the actual library for linking
	LDFLAGS = -lm -lc $(CFLAGS) $(call getlink, $(LIB_OUT_DIR)/libdpfpdd.so) $(call getlink, $(LIB_OUT_DIR)/libdpfj.so)
else
	LDFLAGS = -lm -lc $(CFLAGS) -L $(LIB_OUT_DIR) -ldpfpdd -ldpfj
endif

OBJS = sample.o menu.o helpers.o selection.o verification.o identification.o enrollment.o

all: $(OBJS)
	mkdir -p $(OUT_DIR)
	$(CC) $(OBJS) $(LDFLAGS) -o $(OUT_DIR)/$(EXE_NAME)

clean:
	rm -f $(OUT_DIR)/$(EXE_NAME) *.o *~

%.o: %.c
	$(CC) $(CCFLAGS) -c $< -o $@

.PHONY: install
