CXXFLAGS=-shared
TARGET_GLOG=libglog.a
TARGET_GFLAGS=libgflags.a

all: $(TARGET_GLOG) $(TARGET_GFLAGS)

$(TARGET_GLOG):
	$(MAKE) -C glog
	ar rcs $@ glog/logging.o

$(TARGET_GFLAGS):
	$(MAKE) -C gflags
	ar rcs $@ gflags/gflags.o

clean:
	rm -f *.a *.so *.o *~
	$(MAKE) -C glog clean
	$(MAKE) -C gflags clean
