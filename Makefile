NAME		:= rfx2wav
DESCRIPTION	:= Convert RFX format files created by rfxgen to WAV
COMPANY		:= DrPetter, Raylib Technologies, Deltabeard
COPYRIGHT	:= Copyright (c) 2007 Tomas Pettersson, 2014-2019 Ramon Santamaria, 2020 Mahyar Koshkouei, et al.
LICENSE_SPDX	:= Zlib

# Default compiler options for Unix-like operating systems
CC	:= cc
OBJEXT	:= o
RM	:= rm -f
EXEOUT	:= -o
CFLAGS	:= -std=c99 -pedantic -Wall -Wextra -O2 -g3
EXE	:= $(NAME)
LICENSE := $(COPYRIGHT); Released under the $(LICENSE_SPDX) License.
GIT_VER := $(shell git describe --dirty --always --tags --long)

# Default configurable build options
EXAMPLE_OPTION := 0

define help_txt
$(NAME): $(DESCRIPTION)
$(COPYRIGHT)
Released under the $(LICENSE_SPDX) license.

This application has no configurable build options.
endef

SRCS := $(wildcard src/*.c)
OBJS := $(SRCS:.c=.$(OBJEXT))

# File extension ".exe" is automatically appended on MinGW and MSVC builds, even
# if we don't ask for it.
ifeq ($(OS),Windows_NT)
	EXE := $(NAME).exe
endif

ifeq ($(GIT_VER),)
	GIT_VER := LOCAL
endif

override CFLAGS += -Iinc

all: $(NAME)
$(NAME): $(OBJS) $(RES)
	$(CC) $(CFLAGS) $(EXEOUT)$@ $^ $(LDFLAGS)

%.obj: %.c
	$(CC) $(CFLAGS) /Fo$@ /c /TC $^

%.res: %.rc
	rc /nologo /DCOMPANY="$(COMPANY)" /DDESCRIPTION="$(DESCRIPTION)" \
		/DLICENSE="$(LICENSE)" /DGIT_VER="$(GIT_VER)" \
		/DNAME="$(NAME)" /DICON_FILE="$(ICON_FILE)" $^

clean:
	$(RM) *.$(OBJEXT) $(EXE) $(RES)

help:
	@cd
	$(info $(help_txt))
