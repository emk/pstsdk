#
# pstvalues 
# 

#
# compiler options
#
CC=cl

ifeq ($(CC),cl) #MSVC
OPTIONS=/Ox /EHsc /Zi /Za /nologo /W4 /wd4250 /I "..\..\.."
OUTPUTSWITCH=/Fe
NOLINK=/c
DELCMD=del
else #GCC
OPTIONS=-Wall -I "..\..\.."
OUTPUTSWITCH=-o 
NOLINK=-c
DELCMD=rm
endif

# source file list
SRCS=\
	 main.cpp			\

ifeq ($(CC),cl) #MSVC
OBJS=$(SRCS:.cpp=.obj)
else #GCC
OBJS=$(SRCS:.cpp=.o)
endif

pstvalues.exe: $(OBJS) values.h 
	$(CC) $(OPTIONS) $(OUTPUTSWITCH)pstvalues.exe $(OBJS)

%.obj: %.cpp
	$(CC) $(NOLINK) $(OPTIONS) $?

%.o: %.cpp
	$(CC) $(NOLINK) $(OPTIONS) $?

main.cpp: values.h

values.h: ..\..\util\primatives.h ..\..\disk\disk.h
	perl values.pl ..\..\util\primatives.h ..\..\disk\disk.h > values.h

clean:
	$(DELCMD) values.h *.obj *.o pstvalues.exe *.pdb *.ilk
