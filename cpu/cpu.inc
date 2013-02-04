ifneq ($(HAS_GLOG), 1)
  CXXFLAGS+=-I../../third_party -I../..
  LDFLAGS+=-L../../third_party ../../third_party/gflags/gflags.o ../../third_party/glog/logging.o
else
  # typical directory for gflags and glog
  CXXFLAGS+=-I../..
  LDFLAGS+=-L/usr/local/lib
  LDFLAGS+=-lgflags -lglog
endif
