# ----------------------------
# Makefile Options
# ----------------------------

NAME = Pinecone
ICON = icon.png
DESCRIPTION = "A Computer Algebra System"
COMPRESSED = NO
ARCHIVED = YES

SRC = $(wildcard *.c catalog/*.c)
CFLAGS = -Wall -Wextra -Oz
CXXFLAGS = -Wall -Wextra -Oz

# ----------------------------

include $(shell cedev-config --makefile)
