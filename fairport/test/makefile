#
# fairport unit test
# 

#
# compiler options
#
CC=cl

ifeq ($(CC),cl) #MSVC
OPTIONS=/DEBUG /Od /MDd /EHsc /Zi /nologo /W4 /I "..\.."
OUTPUTSWITCH=/Fe
NOLINK=/c
DELCMD=del
else #GCC
OPTIONS=-Wall -std=c++0x -I "../.."
OUTPUTSWITCH=-o 
NOLINK=-c
DELCMD=rm
endif

# source file list
SRCS=\
	 btreetest.cpp 		\
	 databasetest.cpp 	\
	 main.cpp			\
	 disktest.cpp		\
	 highlevel.cpp		\

ifeq ($(CC),cl) #MSVC
OBJS=$(SRCS:.cpp=.obj)
else #GCC
OBJS=$(SRCS:.cpp=.o)
endif

fairport.exe: $(OBJS) 
	$(CC) $(OPTIONS) $(OUTPUTSWITCH)fairport.exe $(OBJS)

%.obj: %.cpp
	$(CC) $(NOLINK) $(OPTIONS) $?

%.o: %.cpp
	$(CC) $(NOLINK) $(OPTIONS) $?

clean:
	$(DELCMD) *.obj *.o fairport.exe *.pdb *.ilk
