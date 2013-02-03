ifdef NO_GLOG
  CXXFLAGS+=-I../../third_party
  LDFLAGS+=-L../../third_party ../../third_party/gflags/gflags.o ../../third_party/glog/logging.o
else
  # typical directory for gflags and glog
  LDFLAGS+=-L/usr/local/lib
  LDFLAGS+=-lgflags -lglog
endif
