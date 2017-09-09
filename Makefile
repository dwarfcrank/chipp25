TARGET = chipp25

OUT_DIR = build
SRC_DIR = src

SDL2_LIBS = $(shell sdl2-config --libs)
SDL2_CFLAGS = $(shell sdl2-config --cflags)

CXXFLAGS += -g ${SDL2_CFLAGS} --std=c++17
LDFLAGS += -lstdc++ -lstdc++fs $(SDL2_LIBS)

.PHONY: all clean default

default: $(TARGET)
all: default

SRCS = $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS = $(patsubst $(SRC_DIR)/%.cpp, $(OUT_DIR)/%.o, $(SRCS))
HEADERS = $(wildcard $(SRC_DIR)/*.h)

PCH = precompiled.h
PCH_OUT = $(OUT_DIR)/$(PCH).gch
PCH_INCLUDE = -include $(OUT_DIR)/$(PCH)

$(OUT_DIR)/stamp:
	mkdir -p $(OUT_DIR)
	touch $@

$(PCH_OUT): $(OUT_DIR)/stamp
	$(CXX) $(CXXFLAGS) -c -o $@ $(SRC_DIR)/$(PCH)

$(OUT_DIR)/%.o: $(SRC_DIR)/%.cpp $(HEADERS) $(PCH_OUT) $(OUT_DIR)/stamp
	$(CXX) $(CXXFLAGS) $(PCH_INCLUDE) -c -o $@ $<

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -Wall $(LDFLAGS) -o $(OUT_DIR)/$@

clean:
	-rm -rf $(OUT_DIR)

