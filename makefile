SRCS = main.cpp
TARGET = arp_spoofing
OBJS = $(SRCS:.cpp=.o)
HEADERS = arp.hpp

all: $(TARGET)

$(TARGET) : $(OBJS)
	g++ -o $@ $^
%.o: %.cpp $(HEDAERS)
	g++ -c $<

clean:
	rm -rf $(OBJS) $(TARGET)
