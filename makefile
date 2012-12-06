# Copyright 2012: Eugen Sawin <esawin@me73.com>
SRCDIR:=src
TSTDIR:=src/test
BINDIR:=bin
OBJDIR:=bin/obj
GTESTLIBS:=-lgtest -lgtest_main
GFLAGSDIR:=deps/gflags-2.0
GLOGDIR:=deps/glog-0.3.2
CXX:=g++ -std=c++0x -I$(GLOGDIR)/src
# CXX:=g++ -std=c++0x -I$(GFLAGSDIR)/src -I$(GLOGDIR)/src
CFLAGS:=-Wall -O3
LIBS:=$(GLOGDIR)/.libs/libglog.a -lgflags -lpthread -lrt -lboost_system-mt
# LIBS:=$(GFLAGSDIR)/.libs/libgflags.a $(GLOGDIR)/.libs/libglog.a -lpthread -lrt
TSTFLAGS:=
TSTLIBS:=$(GTESTLIBS) -lpthread -lrt
BINS:=pythia

TSTBINS:=$(notdir $(basename $(wildcard $(TSTDIR)/*.cc)))
TSTOBJS:=$(addsuffix .o, $(notdir $(basename $(wildcard $(TSTDIR)/*.cc))))
OBJS:=$(notdir $(basename $(wildcard $(SRCDIR)/*.cc)))
OBJS:=$(addsuffix .o, $(filter-out $(BINS), $(OBJS)))
OBJS:=$(addprefix $(OBJDIR)/, $(OBJS))
BINS:=$(addprefix $(BINDIR)/, $(BINS))
TSTBINS:=$(addprefix $(BINDIR)/, $(TSTBINS))

compile: makedirs $(BINS)
	@echo "compiled all"

profile: CFLAGS=-Wall -O3 -DPROFILE
profile: LIBS+=-lprofiler
profile: clean compile

opt: CFLAGS=-Ofast -flto -mtune=native -DNDEBUG
opt: clean compile

debug: CFLAGS=-O0 -g
debug: compile

depend: gflags glog cpplint

makedirs:
	@mkdir -p bin/obj

gflags:
	@tar xf deps/gflags-2.0.tar.gz -C deps/;
	@cd deps/gflags-2.0/; ./configure; make;
	@echo "compiled gflags"

glog:
	@tar xf deps/glog-0.3.2.tar.gz -C deps/;
	@cd deps/glog-0.3.2/; ./configure; make;
	@echo "compiled glog"

cpplint: 
	@if [ -f tools/cpplint/cpplint.py ];\
	then\
		echo "updating cpplint";\
		cd tools/cpplint; git pull; cd ../..;\
	else\
		echo "cloning cpplint";\
		mkdir tools && cd tools;\
		git clone git@github.com:eamsen/cpplint.git; cd ..;\
	fi

check: makedirs $(TSTBINS)
	@for t in $(TSTBINS); do ./$$t; done
	@echo "completed tests"

checkstyle:
	@python tools/cpplint/cpplint.py \
		--filter=-readability/streams,-readability/multiline_string\
		$(SRCDIR)/*.h $(SRCDIR)/*.cc

clean:
	@rm -f $(OBJDIR)/*.o
	@rm -f $(BINS)
	@rm -f $(TSTBINS)
	@echo "cleaned"

.PRECIOUS: $(OBJS) $(TSTOBJS)
.PHONY: compile profile opt depend makedirs gflags glog check cpplint\
	checkstyle clean

$(BINDIR)/%: $(OBJS) $(SRCDIR)/%.cc
	@$(CXX) $(CFLAGS) -o $(OBJDIR)/$(@F).o -c $(SRCDIR)/$(@F).cc
	@$(CXX) $(CFLAGS) -o $(BINDIR)/$(@F) $(OBJDIR)/$(@F).o $(OBJS) $(LIBS)
	@echo "compiled $(BINDIR)/$(@F)"

$(BINDIR)/%-test: $(OBJDIR)/%-test.o $(OBJS)
	@$(CXX) $(TSTFLAGS) -o $(BINDIR)/$(@F) $(OBJS) $< $(TSTLIBS)
	@echo "compiled $(BINDIR)/$(@F)"

$(OBJDIR)/%-test.o: $(TSTDIR)/%-test.cc
	@$(CXX) $(TSTFLAGS) -o $(OBJDIR)/$(@F) -c $<

$(OBJDIR)/%.o: $(SRCDIR)/%.cc $(SRCDIR)/%.h
	@$(CXX) $(CFLAGS) -o $(OBJDIR)/$(@F) -c $<
