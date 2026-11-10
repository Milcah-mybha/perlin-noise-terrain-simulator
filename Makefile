CC = gcc
CXX = g++
CFLAGS = -Wall -O2 -Iinclude -Iinclude/imgui
CXXFLAGS = -std=c++17 -Wall -O2 -Iinclude -Iinclude/imgui
LDFLAGS = -lglfw -lGL -ldl -lpthread

SRCS_C = src/glad.c

SRCS_CXX = src/PerlinNoise.cpp \
           src/Shader.cpp \
           src/Camera.cpp \
           src/Terrain.cpp \
           src/Water.cpp \
           src/imgui/imgui.cpp \
           src/imgui/imgui_draw.cpp \
           src/imgui/imgui_tables.cpp \
           src/imgui/imgui_widgets.cpp \
           src/imgui/imgui_impl_glfw.cpp \
           src/imgui/imgui_impl_opengl3.cpp \
           src/main.cpp

OBJS = $(patsubst src/%.c, obj/%.o, $(SRCS_C)) $(patsubst src/%.cpp, obj/%.o, $(SRCS_CXX))
TARGET = terrain_simulator

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) $(LDFLAGS)

obj/%.o: src/%.c | obj
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

obj/%.o: src/%.cpp | obj
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

obj:
	mkdir -p obj obj/imgui

clean:
	rm -rf obj $(TARGET)

run: all
	./$(TARGET)

.PHONY: all clean run
