CXXFLAGS := -std=c++14
LDFLAGS := -framework CoreFoundation -framework IOKit

OBJS := main.o

all: macvdmtool

macvdmtool: $(OBJS)
	$(CXX) -o $@ $(OBJS) $(LDFLAGS)

.PHONY: clean
clean:
	rm -f $(OBJS) macvdmtool

ifeq ($(PREFIX),)
    PREFIX := /usr/local
endif

INSTALL := /usr/bin/install

.PHONY: install
install: macvdmtool
	@sudo $(INSTALL) -d "$(PREFIX)/bin/"
	@sudo $(INSTALL) -m 755 -o root -g wheel "$(<)" "$(PREFIX)/bin/"
