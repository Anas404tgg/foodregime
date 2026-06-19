BUILD_DIR ?= build

.PHONY: all configure build test clean

all: build

configure:
	cmake -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Release

build: configure
	cmake --build $(BUILD_DIR)

test: build
	ctest --test-dir $(BUILD_DIR) --output-on-failure --verbose

clean:
	cmake --build $(BUILD_DIR) --target clean || true
