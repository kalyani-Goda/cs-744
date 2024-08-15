IDIR =./include
CC=gcc
CFLAGS=-I$(IDIR) -g

ODIR=obj
SRCDIR=src
BDIR=bin
LDIR =./lib

SERVER_LIBS=-lsqlite3
LIBS=-lpthread

_DEPS = utils.h constants.h sqldb.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_COMMON_OBJ = utils.o
_CLIENT_OBJ = client.o $(_COMMON_OBJ)
_SERVER_OBJ = server.o server_grader_helper.o thread_pool.o server_helper.o sqldb.o $(_COMMON_OBJ)
SERVER_OBJ = $(patsubst %,$(ODIR)/%,$(_SERVER_OBJ)) 
CLIENT_OBJ = $(patsubst %,$(ODIR)/%,$(_CLIENT_OBJ))


$(ODIR)/%.o: $(SRCDIR)/%.c $(DEPS) | $(ODIR)
	$(CC) -c -o $@ $< $(CFLAGS)

all: $(BDIR)/client $(BDIR)/server

$(BDIR) $(ODIR):
	mkdir -p $@

$(BDIR)/server: $(SERVER_OBJ) | $(BDIR)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS) $(SERVER_LIBS)

$(BDIR)/client: $(CLIENT_OBJ) | $(BDIR)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean all

clean:
	rm -rf $(ODIR)/*.o *~ core $(INCDIR)/*~  $(ODIR) $(BDIR)
