CXXFLAGS+=-I../ -L../ -MMD -g

TARGET=libpuyocpu.a
SRCS=cpu_interface.cc
OBJS=$(subst .cc,.o,$(SRCS))

all:
	$(MAKE) $(TARGET)
	$(MAKE) -C hamaji
	$(MAKE) -C hidehiko
	$(MAKE) -C hiroshimizuno
	$(MAKE) -C ichikawa
	$(MAKE) -C pascal
	$(MAKE) -C peria
	$(MAKE) -C shinyak
	$(MAKE) -C sample

$(TARGET): $(OBJS)
	ar rcs $@ $(OBJS)

clean:
	rm -f *.o *.a *~
	$(MAKE) -C hamaji clean
	$(MAKE) -C hidehiko clean
	$(MAKE) -C hiroshimizuno clean
	$(MAKE) -C ichikawa clean
	$(MAKE) -C pascal clean
	$(MAKE) -C peria clean
	$(MAKE) -C shinyak clean
	$(MAKE) -C sample clean
