.PHONY: add-dependencies build build-debug clean run-api test

BUILD_DEBUG?=false

.EXPORT_ALL_VARIABLES:
ELECTIONGUARD_DIR="$(realpath .)/build/ElectionGuard"

# Detect operating system
ifeq '$(findstring ;,$(PATH))' ';'
    OPERATING_SYSTEM := Windows
else
    OPERATING_SYSTEM := $(shell uname 2>/dev/null || echo Unknown)
    OPERATING_SYSTEM := $(patsubst CYGWIN%,Cygwin,$(OPERATING_SYSTEM))
    OPERATING_SYSTEM := $(patsubst MSYS%,MSYS,$(OPERATING_SYSTEM))
    OPERATING_SYSTEM := $(patsubst MINGW%,MSYS,$(OPERATING_SYSTEM))
endif

add-dependencies:
ifeq ($(OPERATING_SYSTEM),Darwin)
    
endif
ifeq ($(OPERATING_SYSTEM),Linux)
    
endif
ifeq ($(OPERATING_SYSTEM),Windows)
    
endif

ifeq ($(BUILD_DEBUG),true)
build: build-debug
else
build: build-release
endif

build-debug: clean
ifeq ($(OPERATING_SYSTEM),Windows)
    cmake -S . -B build -G "MSYS Makefiles" -DCMAKE_BUILD_TYPE=Debug
else
	if [ ! -d "build" ]; then mkdir build; fi
	cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
	cmake --build build
endif

build-release: clean
ifeq ($(OPERATING_SYSTEM),Windows)
    cmake -S . -B build -G "MSYS Makefiles" -DBUILD_SHARED_LIBS=ON
else
	if [ ! -d "build" ]; then mkdir build; fi
	cmake -S . -B build -DBUILD_SHARED_LIBS=ON
	cmake --build build
endif

clean:
ifeq ($(OPERATING_SYSTEM),Windows)
    
else
	rm -rf ./build/* ./api_build/*
endif

run-api: build
ifeq ($(OPERATING_SYSTEM),Windows)
    
else
	if [ ! -d "api_build" ]; then mkdir api_build; fi
	ElectionGuard_DIR=$(ELECTIONGUARD_DIR) cmake -S examples/api -B api_build
	cmake --build api_build --target api
	./api_build/api
endif

test: build
ifeq ($(OPERATING_SYSTEM),Windows)
    
else
	cmake --build build --target test
endif
 