#
# pst_attachment 
# 

#
# compiler options
#
CC=cl

ifeq ($(CC),cl) #MSVC
OPTIONS=/Ox /EHsc /Zi /Za /nologo /W4 /wd4250 /I "..\..\..\..\Boost\boost_1_42_0" /I "..\.."
OUTPUTSWITCH=/Fe
NOLINK=/c
DELCMD=del
else #GCC
OPTIONS=-Wall -std=c++0x -I "..\..\..\..\Boost\boost_1_42_0" -I "..\.."
OUTPUTSWITCH=-o 
NOLINK=-c
DELCMD=rm
endif

# source file list
SRCS=main.cpp

ifeq ($(CC),cl) #MSVC
OBJS=$(SRCS:.cpp=.obj)
else #GCC
OBJS=$(SRCS:.cpp=.o)
endif

pst_attachment.exe: $(OBJS)
	$(CC) $(OPTIONS) $(OUTPUTSWITCH)pst_attachment.exe $(OBJS)

%.obj: %.cpp
	$(CC) $(NOLINK) $(OPTIONS) $?

%.o: %.cpp
	$(CC) $(NOLINK) $(OPTIONS) $?

clean:
	$(DELCMD) *.obj *.o pst_attachment.exe *.pdb *.ilk
