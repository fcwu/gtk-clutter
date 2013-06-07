CFLAGS=$(shell pkg-config --cflags gthread-2.0 gtk+-2.0)
LDFLAGS=$(shell pkg-config --libs gthread-2.0 gtk+-2.0)
CLUTTER_CFLAGS=-Wall $(shell pkg-config --cflags glib-2.0 gtk+-2.0 clutter-gtk-1.0)
CLUTTER_LDFLAGS=$(shell pkg-config --libs clutter-gtk-1.0 gtk+-2.0 glib-2.0)
TARGET := hello testnotebook welcome animation

all: $(TARGET)

hello: hello.o
	gcc $^ -o $@ $(LDFLAGS)

testnotebook: testnotebook.o
	gcc $^ -o $@ $(CLUTTER_LDFLAGS)

testnotebook.o: testnotebook.c
	gcc -c $(CLUTTER_CFLAGS) $^ -o $@

welcome: welcome.o
	gcc $^ -o $@ $(CLUTTER_LDFLAGS)

welcome.o: welcome.c
	gcc -c $(CLUTTER_CFLAGS) $^ -o $@

animation: animation.o
	gcc $^ -o $@ $(LDFLAGS)

%.o: %.c
	gcc -c $(CFLAGS) $^ -o $@

clean:
	-rm -f *.o
	-rm -f $(TARGET)
