PROGS     = mqtt_sub

INSTDIR   = $(prefix)/usr/bin
INSTMODE  = 0755
INSTOWNER = root
INSTGROUP = root

#/home/lq/mw-a87/staging_dir/toolchain-mips_34kc_gcc-4.8-linaro_uClibc-0.9.33.2/lib

OBJS = mqtt_sub.o ramrt.o listmanager.o downthread.o common.o
LDLIBS +=  -ldl -lmosquitto -lpthread

all: $(PROGS)
$(PROGS): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ $(LDLIBS) -o $@
	$(STRIP) $@

%.o: %.c
	$(CC) -c $(CFLAGS) $^ -o $@

install: $(PROGS)
	$(INSTALL) -d $(INSTDIR)
	$(INSTALL) -m $(INSTMODE) -o $(INSTOWNER) -g $(INSTGROUP) $(PROGS) $(INSTDIR)

clean:
	rm -f $(PROGS) *.o core
