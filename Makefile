SRC := Process.cpp process_test.cpp
OBJ := ${SRC:.cpp=.o}
LIBS := 
CC := g++
CFLAGS := -Wall -g -std=c++0x

all: ${OBJ}
	${CC} ${CFLAGS} -o process_test ${LIBS} ${OBJ}

#generate dependancy files at the same time as object compilation
%.o : %.cpp
	${CC} ${CXX_FLAGS} -MD -o $@ -c $< 
	@cp $*.d $*.P; \
	  sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
	 	  -e '/^$$/ d' -e 's/$$/ :/' < $*.P >> $*.d; \
	  rm -f $*.P

clean:
	-rm *.o *.d *~

