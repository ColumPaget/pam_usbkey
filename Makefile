CC=gcc
VERSION=0.0.1
CFLAGS=-g -O2
LIBS=
INSTALL=/bin/install -c
prefix=/
exec_prefix=${prefix}
bindir=${exec_prefix}/bin
libdir=${exec_prefix}/lib
sysconfdir=${prefix}/etc
FLAGS=$(CFLAGS) -DPACKAGE_NAME=\"\" -DPACKAGE_TARNAME=\"\" -DPACKAGE_VERSION=\"\" -DPACKAGE_STRING=\"\" -DPACKAGE_BUGREPORT=\"\" -DPACKAGE_URL=\"\" -DSTDC_HEADERS=1

all: pam_usbkey.so

pam_usbkey.so: common.h usbkey.o utility.o pam_module.c 
	gcc -fPIC -fno-stack-protector -c pam_module.c
	ld -x --shared -lpam -opam_usbkey.so pam_module.o usbkey.o utility.o 
	strip pam_usbkey.so

usbkey.o: usbkey.h usbkey.c
	gcc -c usbkey.c

utility.o: utility.h utility.c
	gcc -c utility.c

install: pam_usbkey.so listusbkeys.sh
	$(INSTALL) -d $(DESTDIR)$(bindir)
	$(INSTALL) -d $(DESTDIR)$(libdir)/security
	$(INSTALL) listusbkeys.sh $(DESTDIR)$(bindir)
	$(INSTALL) pam_usbkey.so $(DESTDIR)$(libdir)/security

clean:
	-rm -f *.o *.so
	-rm -f config.log config.status */config.log */config.status
	-rm -fr autom4te.cache */autom4te.cache

distclean:
	-rm -f *.o *.so
	-rm -f config.log config.status */config.log */config.status Makefile */Makefile
	-rm -fr autom4te.cache */autom4te.cache

