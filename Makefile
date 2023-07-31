CXXFLAGS := -std=c++14
LDFLAGS := -framework CoreFoundation -framework IOKit -lc++

OBJS := main.o

all: macvdmtool

macvdmtool: $(OBJS)
	cc -o $@ $(OBJS) $(LDFLAGS)

ifeq ($(PREFIX),)
    PREFIX := /usr/local
endif

INSTALL := /usr/bin/install

.PHONY: install
install: macvdmtool
	@sudo $(INSTALL) -d "$(PREFIX)/bin/"
	@sudo $(INSTALL) -m 755 -o root -g wheel "$(<)" "$(PREFIX)/bin/"
