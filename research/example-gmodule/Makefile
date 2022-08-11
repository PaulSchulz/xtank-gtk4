app = gmodule-example
lib = libmodule.so

CFLAGS = -Wall -ansi -pedantic
LDFLAGS = -ldl

all: $(app) $(lib)

# Order of libraries is important (before primary source)
$(app): main.c
	$(CC) -o $@ $< `pkg-config --cflags --libs gmodule-2.0`

lib%.so: %.c
	$(CC) -shared -fPIC $(CFLAGS) -o $@ $<

clean:
	$(RM) $(app) $(lib)

run: all
	./$(app)

$(lib): module.h
