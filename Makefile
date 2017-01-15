export DEBUG ?= 1
export USE_STATSD ?= 0
CC = gcc
# TODO: -O3, -ffast-math?
CFLAGS = -std=c99 -g -Wall -D_XOPEN_SOURCE=500 -DDEBUG=$(DEBUG) -DUSE_STATSD=$(USE_STATSD)
LDLIBS = -lfec -lm -lrtlsdr
BIN = rtlvdl2
DEPS =	acars.o \
	avlc.o \
	bitstream.o \
	clnp.o \
	crc.o \
	decode.o \
	deinterleave.o \
	output.o \
	rs.o \
	rtlvdl2.o \
	tlv.o \
	x25.o \
	xid.o \
	util.o

ifeq ($(USE_STATSD), 1)
  DEPS += statsd.o
  LDLIBS += -lstatsdclient
endif

.PHONY = all clean

all: $(BIN)

$(BIN): $(DEPS)

clnp.o: rtlvdl2.h clnp.h

decode.o: rtlvdl2.h

bitstream.o: rtlvdl2.h

deinterleave.o: rtlvdl2.h

rs.o: rtlvdl2.h

rtlvdl2.o: rtlvdl2.h

avlc.o: rtlvdl2.h avlc.h

acars.o: rtlvdl2.h acars.h

output.o: avlc.h acars.h

tlv.o: tlv.h rtlvdl2.h

xid.o: rtlvdl2.h tlv.h

x25.o: rtlvdl2.h clnp.h tlv.h x25.h

clean:
	rm -f *.o $(BIN)
