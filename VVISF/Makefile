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
	CPPFLAGS += -I./include/ -DVVGL_SDK_RPI
	#CPPFLAGS += -O3
	CPPFLAGS += -O0
	CPPFLAGS += -I$(SDKSTAGE)/opt/vc/include/
	CPPFLAGS += -I$(SDKSTAGE)/opt/vc/include/interface/vcos/pthreads
	CPPFLAGS += -I$(SDKSTAGE)/opt/vc/include/interface/vmcs_host/linux
	CPPFLAGS += -Wno-unknown-pragmas
	
	LDFLAGS := -lstdc++ -shared -fPIC
	LDFLAGS += -L$(SDKSTAGE)/opt/vc/lib/
	LDFLAGS += -lGLESv2 -lEGL
	LDFLAGS += -lbcm_host -lpthread
else ifeq ($(shell uname),Linux)

else ifeq ($(PLATFORM),Darwin)
	# make sure we have an ARCH value even if one wasn't provided by the environment!
	ARCH ?= x86_64
	
	CXX = clang
	CPPFLAGS := -Wall -g -arch $(ARCH) -O3 -std=c++11 -stdlib=libc++ -fPIC 
	CPPFLAGS += -isysroot /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk
	CPPFLAGS += -I./include/ -DVVGL_SDK_MAC
    CPPFLAGS += -DGL_SILENCE_DEPRECATION
	OBJCPPFLAGS := $(CPPFLAGS)
	
	CPPFLAGS += -x c++
	OBJCPPFLAGS += -x objective-c++
	
	LDFLAGS := -arch $(ARCH) -lstdc++ -shared -fPIC -lobjc
	LDFLAGS += -framework Foundation -framework ImageIO -framework OpenGL -framework IOSurface -framework CoreGraphics -framework CoreVideo
endif


#	the GL_DEBUG flag enables a call to glGetError() after every call to GL
#CPPFLAGS += -DGL_DEBUG
#OBJCPPFLAGS += -DGL_DEBUG


#CPPFLAGS += -I./include/exprtk/
#CPPFLAGS += -I./include/nlohmann_json/


#	un-comment the following line when compiling on embedded hardware or wherever resources will be limited (compiling exprtk takes a comparatively huge amount of resources)
#.NOTPARALLEL : 


# link against and include the VVGL library
LDFLAGS += -L./../VVGL/bin -lVVGL
CPPFLAGS += -I./../VVGL/include
OBJCPPFLAGS += -I./../VVGL/include


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
OUTPUT = ./bin/libVVISF.a
#	recipe to create the dependencies, output file and the directory it goes in
$(OUTPUT) : $(OBJS)
	$(MAKE) -C ./../VVGL
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
	$(MAKE) -C ./../VVGL clean_all

install : all
	




