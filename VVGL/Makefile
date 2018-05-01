PLATFORM = $(shell uname)
ifneq ("$(wildcard /etc/os-release)","")
PLATFORM = $(shell . /etc/os-release && echo $$NAME)
endif

#override platform selection on RPi:
ifneq ("$(wildcard /opt/vc/include/bcm_host.h)","")
	PLATFORM = Raspbian GNU/Linux
endif


#	set the basic per-platform compile options
ifeq ($(PLATFORM),Raspbian GNU/Linux)
	CXX = g++
	
	CPPFLAGS := -Wall -g -std=c++11 -fPIC 
	CPPFLAGS += -I./include/ -DISF_SDK_RPI
	#CPPFLAGS += -O3
	CPPFLAGS += -O0
	CPPFLAGS += -I$(SDKSTAGE)/opt/vc/include/
	CPPFLAGS += -I$(SDKSTAGE)/opt/vc/include/interface/vcos/pthreads
	CPPFLAGS += -I$(SDKSTAGE)/opt/vc/include/interface/vmcs_host/linux
	CPPFLAGS += -Wno-unknown-pragmas
	CPPFLAGS += -Wno-multichar
	
	LDFLAGS := -lstdc++ -shared -fPIC
	LDFLAGS += -L$(SDKSTAGE)/opt/vc/lib/
	LDFLAGS += -lGLESv2 -lEGL
	LDFLAGS += -lbcm_host -lpthread
else ifeq ($(shell uname),Linux)

else ifeq ($(PLATFORM),Darwin)
	CXX = clang
	CPPFLAGS := -Wall -g -arch x86_64 -O3 -std=c++11 -stdlib=libc++ -fPIC 
	CPPFLAGS += -I./include/ -DISF_SDK_MAC
	OBJCPPFLAGS := $(CPPFLAGS)
	
	CPPFLAGS += -x c++
	OBJCPPFLAGS += -x objective-c++
	
	LDFLAGS := -arch x86_64 -lstdc++ -shared -fPIC -lobjc
	LDFLAGS += -framework Foundation -framework ImageIO -framework OpenGL -framework IOSurface -framework CoreGraphics -framework CoreVideo
endif


#	the GL_DEBUG flag enables a call to glGetError() after every call to GL
CPPFLAGS += -DGL_DEBUG
OBJCPPFLAGS += -DGL_DEBUG


#	rules that define object files for source files
OBJS := $(patsubst ./src/%.cpp,./obj/%.o,$(wildcard ./src/*.cpp))
ifeq ($(PLATFORM),Darwin)
OBJS += $(patsubst ./src/%.mm,./obj/%.mm.o,$(wildcard ./src/*.mm))
endif


#	targets are object files, recipe compiles them
ifeq ($(PLATFORM),Darwin)
./obj/%.mm.o : ./src/%.mm
	$(shell mkdir -p ./obj)
	$(CXX) $(OBJCPPFLAGS) -c $< -o $@
endif
./obj/%.o : ./src/%.cpp
	$(shell mkdir -p ./obj)
	$(CXX) $(CPPFLAGS) -c $< -o $@


#$(info Platform is $(PLATFORM))
#$(info OBJS are $(OBJS))
#$(info CPPFLAGS are $(CPPFLAGS))
#$(info LDFLAGS are $(LDFLAGS))


#	the output file is a static lib
OUTPUT = ./bin/libVVGL.a
#	recipe to create the output file and the directory it goes in
$(OUTPUT) : $(OBJS)
	$(shell mkdir -p ./bin)
	ar crv $(OUTPUT) $(OBJS)
#	ar rcs $(OUTPUT) $(OBJS)
#	libtool static $(OUTPUT) -o $(OBJS)


#administrative targets
.PHONY : all clean clean_all

all : $(OUTPUT)

clean:
	rm -rf ./obj
	rm -rf ./bin

clean_all : clean






