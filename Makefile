include ../common.make
# TODO: Work with spaces in BUILDDIR

sources  := \
  rx/text.cc \
  rx/netreach.cc \

lib_headers     := $(wildcard rx/*.h) $(wildcard rx/*.hh)

objdir     	    := .obj$(BUILD_SUFFIX)
objects   	    := $(call src_to_obj,$(sources))
object_dirs     := $(call find_uniqdirs,$(objects))

lib_headers_dir := $(BUILDDIR)/include/rx
lib_headers     := $(patsubst rx/%,$(lib_headers_dir)/%,$(lib_headers))
lib_header_dirs := $(call find_uniqdirs,$(lib_headers))

librx           := $(BUILDDIR)/lib/librx.a

CXXFLAGS += -fno-rtti

$(librx): pre $(lib_headers) $(objects)
	$(XAR) $@ $(objects)

pre:
	@mkdir -p "$(object_dirs)" "$(lib_headers_dir)" "$(dir $(librx))"

ifeq ($(PLATFORM),ios)
$(objdir)/%.o: %.cc
	$(IOSXC) $< $@ $(CXXFLAGS)
$(objdir)/%.o: %.c
	$(IOSXC) $< $@ $(CFLAGS)
else
$(objdir)/%.o: %.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@
$(objdir)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
endif

$(lib_headers_dir)/%.h: rx/%.h
	cp $^ $@
$(lib_headers_dir)/%.hh: rx/%.hh
	cp $^ $@

clean:
	rm -rf .obj* "$(SRCROOT)"/build{,-g}/lib/librx.a

-include ${objects:.o=.d}
.PHONY: all clean
