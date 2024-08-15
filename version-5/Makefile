IDIR =./include
CC=gcc
CMAKE=cmake
MAKE=make
CFLAGS=-I$(IDIR) -I./$(SAFEEXECDIR) -g

ODIR=obj
SRCDIR=src
SAFEEXECDIR=safeexec
BDIR=bin
LDIR =./lib

SERVER_LIBS=-lsqlite3 -L ./$(BDIR)/ $(BDIR)/$(SAFEEXEC_LIB)
LIBS=-lpthread

_DEPS = utils.h constants.h sqldb.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

SAFEEXEC_LIB = libsafeexec.a

_COMMON_OBJ = utils.o
_CLIENT_OBJ = client.o $(_COMMON_OBJ)
_SERVER_OBJ = server.o server_grader_helper.o thread_pool.o server_helper.o sqldb.o $(_COMMON_OBJ)
SERVER_OBJ = $(patsubst %,$(ODIR)/%,$(_SERVER_OBJ)) 
CLIENT_OBJ = $(patsubst %,$(ODIR)/%,$(_CLIENT_OBJ))


$(ODIR)/%.o: $(SRCDIR)/%.c $(DEPS) | $(ODIR)
	$(CC) -c -o $@ $< $(CFLAGS)

all: $(BDIR)/client $(BDIR)/server

install: $(SAFEEXECDIR)
	cd $(SAFEEXECDIR); $(CMAKE) .; $(MAKE)
	cp $(SAFEEXECDIR)/$(SAFEEXEC_LIB) $(BDIR)

$(BDIR) $(ODIR):
	mkdir -p $@

$(BDIR)/server: $(SERVER_OBJ) | $(BDIR) 
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS) $(SERVER_LIBS)

$(BDIR)/client: $(CLIENT_OBJ) | $(BDIR)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean all install

clean:
	rm -rf $(ODIR)/*.o *~ core $(INCDIR)/*~  $(ODIR) $(BDIR) 
