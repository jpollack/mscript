MATLABROOT=	/opt/matlab-2013a-native
CXXFLAGS=	-Wall -O3 -Werror -g -I $(MATLABROOT)/extern/include
LDFLAGS=	-lm -L$(MATLABROOT)/bin/glnxa64 -lmx -lmex -leng -Wl,-rpath,$(MATLABROOT)/bin/glnxa64
OBJS=		mscript.o
PREFIX=		/usr/local

all:		mscript

mscript:	$(OBJS)

clean:
	rm -f mscript $(OBJS)
