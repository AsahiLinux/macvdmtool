CXXFLAGS := -std=c++14
LDFLAGS := -framework CoreFoundation -framework IOKit

OBJS := main.o

PREFIX ?= /usr/local
DESTDIR ?=

INSTALL := /usr/bin/install

all: macvdmtool

macvdmtool: $(OBJS)
	$(CXX) -o $@ $(OBJS) $(LDFLAGS)

.PHONY: install
install: macvdmtool
	$(INSTALL) -d "$(DESTDIR)$(PREFIX)/bin"
	$(INSTALL) -m 755 "$(<)" "$(DESTDIR)$(PREFIX)/bin"

.PHONY: clean
clean:
	rm -f $(OBJS) macvdmtool
