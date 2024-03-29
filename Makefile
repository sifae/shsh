CC=gcc
SRCDIR=./src
OBJDIR=./obj
CFLAGS=-I$(SRCDIR) -Wall -g -o
HEADERS=shell.h list.h
SOURCES=shell.c list.c main.c
TARGET=shsh
OBJ:=$(SOURCES:.c=.o) $(HEADERS:.h=.o)
OBJ:=$(addprefix $(OBJDIR)/,$(OBJ))

vpath %.c $(SRCDIR)
vpath %.h $(SRCDIR)

$(TARGET) : $(OBJ) 
	$(CC) $(CFLAGS) $(TARGET) $^

$(OBJDIR)/%.o: %.c $(HEADERS)
	$(CC) -c $(CFLAGS) $@ $<

clean:  
	rm -f $(OBJ) $(TARGET)
