#
# Copyright (c) 2013-2014 ELL-i co-operative
#
# This is part of ELL-i software
#
# ELL-i software is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# ELL-i software is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with ELL-i software.  If not, see <http://www.gnu.org/licenses/>.
#

PLATFORM ?= arduino
# PLATFORM = emulator

all:
	make PLATFORM=$(PLATFORM) -f ../make/app.mk all

clean:
	make PLATFORM=$(PLATFORM) -f ../make/app.mk clean

sketch.cpp: Makefile
	echo "#include <Arduino.h>" > sketch.cpp
	echo "void setup() {}"      >> sketch.cpp 
	echo "void loop() {}"       >> sketch.cpp
