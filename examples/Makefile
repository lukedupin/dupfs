CC = g++

EXEC_NAME = dupfs

LIBS = -lfuse

DEFINE = -D_FILE_OFFSET_BITS=64

OBJECT_LIST = 

$(EXEC_NAME): $(OBJECT_LIST) main.cpp
	$(CC) main.cpp $(OBJECT_LIST) $(LIBS) $(DEFINE) -Wall -o $(EXEC_NAME)

clean:
	rm -f *.o
	rm -f $(EXEC_NAME)

