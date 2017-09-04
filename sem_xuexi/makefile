TARGET = thesemLianxi

CC = g++ -g -Wall -Werror -fPIC -m64 -D_DEBUG
# -w 忽略警告 -Wall 开启警告
CC := $(CC) -w

SRCS = $(wildcard *.cpp)
$(warning, SRCS=$(SRCS))
OBJS = $(patsubst %.cpp,%.o,$(SRCS))
$(warning, OBJS=$(OBJS))

$(TARGET):$(OBJS)
	$(CC) -o $@ $(OBJS)
	
%.o:%.cpp
	$(CC) -c $?
	
clean:
	rm -f *.o $(TARGET)
	rm -f `find ./ -name "*.log"`
