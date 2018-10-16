# O/S Dependencies and Common Rules for Makefiles
#
OS = $(shell uname)

ifneq (,$(findstring CYGWIN, $(OS)))

############################
# CYGWIN
############################
GLUT_DIR = /usr/src/freeglut-2.6.0
CC     = gcc
LD     = /bin/sh /usr/src/freeglut-2.6.0/libtool --tag=CC --mode=link gcc -all-static ${GLUT_DIR}/src/libglut.la
CFLAGS = -Wall -Werror -pedantic -O2 -g -DEMULATE_BUFFERS -I../base -I${GLUT_DIR}/include
LFLAGS = -g -lm -lz -lstdc++ -lglu32
DRUN   = /bin/sh /usr/src/freeglut-2.6.0/libtool --mode=execute gdb --init-command=.gdbinit $(PROG) 

else
ifeq ($(OS), Darwin) 

############################
# MAC OS X
############################
CC     = clang
LD     = clang
CFLAGS = -Wall -Werror -Wno-deprecated-declarations -pedantic -O2 -g -DEMULATE_BUFFERS -DGLUT_ONLY -I../base -fno-color-diagnostics -Wno-c++11-long-long
LFLAGS = -g -lm -lz -lstdc++ -framework OpenGl -framework GLUT -framework CoreFoundation -framework IOKit -framework Carbon -framework CoreGraphics -framework Cocoa
DRUN   = lldb -f $(PROG) -s .gdbinit

else
ifeq ($(OS), Linux)

############################
# LINUX
############################
GLUT_DIR = /home/utils/freeglut-2.8.1
CC     = gcc
LD     = gcc
#CFLAGS = -Wall -Werror -pedantic -O2 -g -DGL_GLEXT_PROTOTYPES -I../base -I${GLUT_DIR}/include
CFLAGS = -Wall -Werror -pedantic -Wno-long-long -Wno-deprecated -O2 -g -DEMULATE_BUFFERS -I../base -I${GLUT_DIR}/include
LFLAGS = -g -lm -lz -lstdc++ -lGL -lglut -lGLU -L${GLUT_DIR}/lib
DRUN   = gdb $(PROG)

else
$(error Unknown O/S: $(OS))
endif
endif
endif

############################
# COMMON RULES
############################

all: $(OBJS) $(PROG)

%.o: %.cpp $(DEPS)
	$(CC) $(CFLAGS) -c $<

%.exe: %.o $(DEPS) $(OBJS) 
	$(LD) $(OBJS) $(LIBS) $(LFLAGS) -o $@ 

$(WRAPPER).js : $(DEPS) $(WRAPPER).i
	swig -w401 -w451 -javascript -node -c++ -I../base $(WRAPPER).i

drun: $(PROG)
	$(DRUN)

run: all
	./$(PROG)

clean:
	rm -fr *.o *.exe .libs *.gz *.out ._* .libs $(PROG) *.stackdump *.mp4 $(WRAPPER).pm *.cxx
