	# the compiler: gcc for C program, define as g++ for C++
  	CC=gcc
	DEBUG = -g
	
	
	RM=/bin/rm -f
	MV=/bin/mv -f
  # LOCATION of all Files
	HEADERS = $(shell ls hdr/*.h)
	SOURCES = $(shell ls src/*.c)
	BUILD_LOCATION = build/
  	SOURCE_LOCATION = src/
  # compiler flags:
  #  -g    adds debugging information to the executable file
  #  -Wall turns on most, but not all, compiler warnings
  	CFLAGS = -Wall -c $(DEBUG) -DPATTERN
	LFLAGS = -Wall $(DEBUG) -lcrypto -lcunit
	INC = -I$(HEADERS)
	INCLUDES = -I./hdr/	
  # the build target executable:
    TARGET = $(BUILD_LOCATION)file
  	OBJS = $(SOURCES:.c=.o)
	BIN_DIR = build/
  
#SPLINT
	

  all: $(TARGET)



	$(RM) $(SOURCE_LOCATION)*.o

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LFLAGS) $(LIBS) $(INC)

#$(BIN_DIR)src/%.o: src/%.c
#	$(CC) -o $@ -c $< $(CFLAGS)
	
	$(MV) $(SOURCE_LOCATION)*.o $(BUILD_LOCATION)	

	
  clean: 
	$(RM) $(BUILD_LOCATION)*.*
	$(RM) $(BUILD_LOCATION)*

  splint:
	splint $(SOURCES) +posixlib

  flawfinder:	
	flawfinder $(SOURCES)
