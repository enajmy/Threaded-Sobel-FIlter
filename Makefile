CC=g++
CFLAGS=-O0 -I. -I/usr/include/opencv4 `pkg-config opencv4 --cflags --libs`
DEPS = sobel.h
OBJ = main.o sobel.o
LIBS = -lpthread

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) $(LIBS)

sobelFilter: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)
