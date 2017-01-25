PLATFORM = $(shell uname)
ifneq ("$(wildcard /etc/os-release)","")
PLATFORM = $(shell . /etc/os-release && echo $$NAME)
endif

#override platform selection on RPi:
ifneq ("$(wildcard /opt/vc/include/bcm_host.h)","")
    PLATFORM = Raspbian GNU/Linux
endif

#$(info Platform is $(PLATFORM)) 




ifeq ($(PLATFORM),Raspbian GNU/Linux)
	CXX = g++
	
	CPPFLAGS := -Wall -g -std=c++11 -fPIC -I./include/ -DISF_TARGET_RPI
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
	CXX = clang
	CPPFLAGS := -Wall -g -arch x86_64 -O3 -std=c++11 -stdlib=libc++ -fPIC -I./include/ -DISF_TARGET_MAC
	OBJCPPFLAGS := $(CPPFLAGS)
	
	CPPFLAGS += -x c++
	OBJCPPFLAGS += -x objective-c++
	
	LDFLAGS := -arch x86_64 -lstdc++ -shared -fPIC -lobjc
	LDFLAGS += -framework Foundation -framework ImageIO -framework OpenGL -framework IOSurface -framework CoreGraphics -framework CoreVideo
endif

#CPPFLAGS += -I./include/exprtk/
#CPPFLAGS += -I./include/nlohmann_json/

# we want to link against the VVGL static library
LDFLAGS += -L./../VVGL/bin -lVVGL
CPPFLAGS += -I./../VVGL/include
OBJCPPFLAGS += -I./../VVGL/include

#CPPFLAGS += -Dexprtk_disable_enhanced_features
#OBJCPPFLAGS += -Dexprtk_disable_enhanced_features
#CPPFLAGS += -DGL_DEBUG
#OBJCPPFLAGS += -DGL_DEBUG


OUTPUT = ./bin/libVVISF.a

OBJS := $(patsubst ./src/%.cpp,./obj/%.o,$(wildcard ./src/*.cpp))
ifeq ($(PLATFORM),Darwin)
OBJS += $(patsubst ./src/%.mm,./obj/%.mm.o,$(wildcard ./src/*.mm))
endif

#$(info OBJS are $(OBJS))
#$(info CPPFLAGS are $(CPPFLAGS))
#$(info LDFLAGS are $(LDFLAGS))


.NOTPARALLEL : 
.PHONY : all clean


all : $(OUTPUT)

clean:
	rm -f ./obj/*.o
	rm -f ./bin/*.a
	$(MAKE) -C ./../VVGL clean


$(OUTPUT) : $(OBJS)
	$(MAKE) -C ./../VVGL
	$(shell mkdir -p ./bin)
	ar crv $(OUTPUT) $(OBJS)


ifeq ($(PLATFORM),Darwin)
./obj/%.mm.o : ./src/%.mm
	$(shell mkdir -p ./obj)
	$(CXX) $(OBJCPPFLAGS) -c $< -o $@
endif
./obj/%.o : ./src/%.cpp
	$(shell mkdir -p ./obj)
	$(CXX) $(CPPFLAGS) -c $< -o $@


.PHONY : clean






