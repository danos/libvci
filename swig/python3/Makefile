# Copyright (c) 2018-2019, AT&T Intellectual Property.
# All rights reserved.
#
# SPDX-License-Identifier: LGPL-2.1-only

TARGET := _vci.so

all: $(TARGET)

vci_wrap.cxx: vci.i
	swig3.0 -Wall -c++ -python -py3 -threads vci.i

$(TARGET): vci_wrap.cxx helpers.hpp ../../vci.hpp
	g++ $(CPPFLAGS) $(CXXFLAGS) $(shell pkg-config --cflags python3) -L../../ -shared -fPIC $(LDFLAGS) -Wl,-soname,libvci-python3.so.1 -o _vci.so -std=c++11 -lvci vci_wrap.cxx

clean:
	rm -f _vci.so
	rm -f vci_wrap.cxx
	rm -f vci_wrap.h
	rm -f vci.py
	rm -rf __pycache__
