TARGET=libpuyocore.a
SRCS=ctrl.cc field.cc decision.cc plan.cc field_deprecated.cc
OBJS=$(subst .cc,.o,$(SRCS))
CXXFLAGS+=-I..

# Comment out if you have a real glib library.
ifneq ($(HAS_GLOG), 1)
  CXXFLAGS+=-I../third_party -L../third_party
endif

$(TARGET): $(OBJS)
	ar rcs $@ $(OBJS)

################################################################################
# Test section.
################################################################################
HAS_GTEST=$(shell $(CC) -L../third_party -xc /dev/null -nostdlib -lgtest && \
	echo "Success")
TEST_BINARIES=ctrl_test field_test performance_test
CXXFLAGS+=-O2 -I../third_party -L../third_party -MMD -g
LDFLAGS+=-lgtest -lgtest_main -lglog -lpthread

tests:
	$(if $(HAS_GTEST), make tests_main)

tests_main: $(TEST_BINARIES)
	./ctrl_test
	./field_test
	./performance_test

field_test: $(OBJS) ../third_party/*.a field_test.o
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

performance_test: $(OBJS) performance_test.o ../util/tsc.o
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

ctrl_test: $(OBJS) ../third_party/*.a ctrl_test.o
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

clean:
	rm -f *~ *.o *.d $(TARGET) $(TEST_BINARIES)

-include *.d
