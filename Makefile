CC=gcc
CXX=g++
RM=rm -f
CPPFLAGS=
LDFLAGS=-Llib
LDLIBS=-lglfw -lXrandr -lGL
INCLUDES=-I. -Icommon -Imath -Iinclude

SRCS_CPP= \
    common/platform_linux.cpp \
    datamanager/data.cpp \
    datamanager/dataserializer.cpp \
	game/character.cpp \
	game/game.cpp \
	game/handle.cpp \
	game/main.cpp \
	math/collision.cpp \
	math/color.cpp \
	math/euler.cpp \
	math/frustum.cpp \
	math/matrix.cpp \
	math/quaternion.cpp \
	math/vector.cpp \
	math/graph.cpp \
	renderer/application.cpp \
	renderer/image_read.cpp \
	renderer/renderer.cpp \
	renderer/renderingcontext.cpp \
	renderer/shaders.cpp \

SRCS_C= \
	renderer/gl3w.c \

SRCS= $(SRCS_CPP) $(SRCS_C)

OBJS_CPP=$(subst .cpp,.o,$(SRCS_CPP))
OBJS_C=$(subst .c,.o,$(SRCS_C))

OBJS=$(OBJS_CPP) $(OBJS_C)

all: mfgd

mfgd: $(OBJS)
	g++ $(LDFLAGS) -o mfgd $(OBJS) $(LDLIBS) 

%.o:%.c
	$(CXX) $(CPPFLAGS) $(INCLUDES) -c $< -o $@

%.o:%.cpp
	$(CXX) $(CPPFLAGS) -std=c++0x $(INCLUDES) -c $< -o $@

depend: .depend

.depend: $(SRCS)
	rm -f ./.depend
	$(CXX) $(CPPFLAGS) $(INCLUDES) -MM $(SRCS) $^>>./.depend;

clean:
	$(RM) $(OBJS) .depend

dist-clean: clean
	$(RM) *~ .dependtool

include .depend
