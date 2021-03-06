PREFIX = ~/.local
MANDIR = ${PREFIX}/share/man

LIBS_PREFIX = /usr/local
CFLAGS += -I${LIBS_PREFIX}/include
LDFLAGS += -L${LIBS_PREFIX}/lib

CFLAGS += -Wall -Wextra -Wpedantic -Wno-gnu-case-range
LDLIBS.dtch = -lutil
LDLIBS.fbclock = -lz
LDLIBS.glitch = -lz
LDLIBS.hnel = -lutil
LDLIBS.modem = -lutil
LDLIBS.pngo = -lz
LDLIBS.ptee = -lutil
LDLIBS.relay = -ltls
LDLIBS.scheme = -lm
LDLIBS.title = -lcurl

-include config.mk

BINS += beef
BINS += bibsort
BINS += bit
BINS += c
BINS += dtch
BINS += glitch
BINS += hilex
BINS += hnel
BINS += htagml
BINS += modem
BINS += mtags
BINS += nudge
BINS += order
BINS += pbd
BINS += pngo
BINS += psf2png
BINS += ptee
BINS += scheme
BINS += shotty
BINS += title
BINS += up
BINS += when
BINS += xx

BINS_BSD += ever
BINS_LINUX += bri
BINS_LINUX += fbatt
BINS_LINUX += fbclock
BINS_LINUX += psfed
BINS_TLS += relay

BINS_ALL = ${BINS} ${BINS_BSD} ${BINS_LINUX} ${BINS_TLS}
MANS_ALL = ${BINS_ALL:%=man1/%.1}

any: meta ${BINS}

bsd: meta ${BINS_BSD}

linux: meta ${BINS_LINUX}

tls: meta ${BINS_TLS}

meta: .gitignore tags

.SUFFIXES: .pl

.c:
	${CC} ${CFLAGS} ${LDFLAGS} $< ${LDLIBS.$@} -o $@

.o:
	${CC} ${LDFLAGS} $< ${LDLIBS.$@} -o $@

.pl:
	cp -f $< $@
	chmod a+x $@

OBJS.hilex += c11.o
OBJS.hilex += hilex.o
OBJS.hilex += make.o
OBJS.hilex += mdoc.o
OBJS.hilex += sh.o

hilex: ${OBJS.hilex}
	${CC} ${LDFLAGS} ${OBJS.$@} ${LDLIBS.$@} -o $@

${OBJS.hilex}: hilex.h

fbatt.o fbclock.o: scheme.h

psf2png.o scheme.o: png.h

scheme.h: scheme
	./scheme -c > scheme.h

tags: *.[chly]
	ctags -w *.[chly]

IGNORE = *.o *.html ${BINS_ALL} scheme.h tags htmltags

.gitignore: Makefile
	echo config.mk '${IGNORE}' | tr ' ' '\n' | sort > .gitignore

clean:
	rm -f ${IGNORE}

setuid: bri
	chown root bri
	chmod u+s bri

link:
	install -d ${PREFIX}/bin ${MANDIR}/man1
	ln -fs ${BINS_ALL:%=${PWD}/%} ${PREFIX}/bin
	ln -fs ${MANS_ALL:%=${PWD}/%} ${MANDIR}/man1

unlink:
	rm -f ${BINS_ALL:%=${PREFIX}/bin/%}
	rm -f ${MANS_ALL:%=${MANDIR}/%}

HTMLS = index.html ${BINS_ALL:=.html} png.html
WEBROOT = /usr/local/www/causal.agency

html: ${HTMLS}
	@true

${HTMLS}: html.sh scheme hilex htagml htmltags

htmltags: *.[chly] mtags Makefile *.sh
	rm -f $@
	for f in *.[chly]; do ctags -aw -f $@ $$f; done
	./mtags -a -f $@ Makefile *.sh

.SUFFIXES: .html

.c.html:
	sh html.sh man1/${<:.c=.1} $< > $@

.h.html:
	sh html.sh man3/${<:.h=.3} $< > $@

.y.html:
	sh html.sh man1/${<:.y=.1} $< > $@

.sh.html:
	sh html.sh man1/${<:.sh=.1} $< > $@

.pl.html:
	sh html.sh man1/${<:.pl=.1} $< > $@

index.html: README.7 Makefile html.sh
	sh html.sh README.7 Makefile html.sh > $@

install-html: ${HTMLS}
	install -d ${WEBROOT}/bin
	install -C -m 644 ${HTMLS} ${WEBROOT}/bin
