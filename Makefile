CC      ?= cc
CFLAGS  ?= -O2 -std=c11 -Wall -Wextra
LDFLAGS ?=
LDLIBS  ?= -lX11 -lGL -lm -ldl

TARGET := dotradio
SRCS   := main.c audioproc.c gfx_x11.c globemath.c world_data.c
OBJS   := $(SRCS:.c=.o)
DEPS   := $(OBJS:.o=.d)

PREFIX  ?= $(HOME)/.local
BINDIR  ?= $(PREFIX)/bin
DATADIR ?= $(PREFIX)/share
APPDIR  ?= $(DATADIR)/applications
ICONDIR ?= $(DATADIR)/icons/hicolor

.PHONY: all clean install uninstall

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

%.o: %.c
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

-include $(DEPS)

clean:
	rm -f $(TARGET) $(OBJS) $(DEPS)

install: $(TARGET)
	install -Dm755 $(TARGET) $(DESTDIR)$(BINDIR)/$(TARGET)
	install -Dm644 stations.csv $(DESTDIR)$(DATADIR)/dotradio/stations.csv
	install -Dm644 dotradio.desktop $(DESTDIR)$(APPDIR)/dotradio.desktop
	install -Dm644 icons/dotradio.svg $(DESTDIR)$(ICONDIR)/scalable/apps/dotradio.svg
	@if [ -z "$(DESTDIR)" ] && command -v update-desktop-database >/dev/null 2>&1; then \
		update-desktop-database $(APPDIR) || true; \
	fi
	@if [ -z "$(DESTDIR)" ] && command -v gtk-update-icon-cache >/dev/null 2>&1; then \
		gtk-update-icon-cache -qt $(ICONDIR) || true; \
	fi

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/$(TARGET)
	rm -f $(DESTDIR)$(DATADIR)/dotradio/stations.csv
	rm -f $(DESTDIR)$(APPDIR)/dotradio.desktop
	rm -f $(DESTDIR)$(ICONDIR)/scalable/apps/dotradio.svg
	@if [ -z "$(DESTDIR)" ] && command -v update-desktop-database >/dev/null 2>&1; then \
		update-desktop-database $(APPDIR) || true; \
	fi
