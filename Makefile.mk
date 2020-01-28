.PHONY: add-dependencies build build-debug clean run-api test

BUILD_DEBUG?=true

.EXPORT_ALL_VARIABLES:
ELECTIONGUARD_DIR="$(realpath .)/build/ElectionGuard"

# Detect operating system
ifeq ($(OS),Windows_NT)
    OPERATING_SYSTEM := Windows
else
    OPERATING_SYSTEM := $(shell uname 2>/dev/null || echo Unknown)
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
	if [ ! -d "build" ]; then mkdir build; fi
ifeq ($(OPERATING_SYSTEM),Windows)
	cmake -S . -B build -G "MSYS Makefiles" -DCMAKE_BUILD_TYPE=Debug
else
	cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
endif
	cmake --build build

build-release: clean
	if [ ! -d "build" ]; then mkdir build; fi
ifeq ($(OPERATING_SYSTEM),Windows)
	cmake -S . -B build -G "MSYS Makefiles" -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON
else
	cmake -S . -B build -DBUILD_SHARED_LIBS=ON
endif
	cmake --build build

clean:
	rm -rf ./build/* ./api_build/*

run-api: build
	if [ ! -d "api_build" ]; then mkdir api_build; fi
ifeq ($(OPERATING_SYSTEM),Windows)
	CMAKE_PREFIX_PATH="./build/ElectionGuard" cmake -S examples/api -B api_build -G "MSYS Makefiles"
	cmake --build api_build --target api
	PATH=$(PWD)/build:$$PATH; ./api_build/api
else
	ElectionGuard_DIR=$(ELECTIONGUARD_DIR) cmake -S examples/api -B api_build
	cmake --build api_build --target api
	./api_build/api
endif

test: build
	cmake --build build --target test
 