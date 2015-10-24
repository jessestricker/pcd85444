MAJOR	= 1
MINOR	= 0
VERSION	= $(MAJOR).$(MINOR)
NAME	= pcd8544
LIBFILE	= lib$(NAME).so.$(VERSION)
DEST	= /usr/lib/

# Compiler
CXXFLAGS += -fPIC
CXXFLAGS += -Wall -Werror
CXXFLAGS += -O3
CXXFLAGS += -std=c++11

# Linker
STTCLIBS += -lbcm2835
LDLIBS += -lgpio

.PHONY: install test clean

$(LIBFILE): $(NAME).o
	$(LINK.cc) $(LDLIBS) $^ -shared -o $@ -Wl,-whole-archive $(STTCLIBS) -Wl,-no-whole-archive

install: $(LIBFILE)
	cp -f $(NAME).h /usr/include
	cp -f $(LIBFILE) $(DEST)
	ln -sf $(DEST)/$(LIBFILE) $(DEST)/lib$(NAME).so
	ldconfig -n $(DEST)

test: $(NAME)-test.o
	$(LINK.cc) $(LDLIBS) -l$(NAME) $^ -o $(NAME)-test

clean:
	$(RM) *.o *.so* $(NAME)-test