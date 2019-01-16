#include(../../spectrum.pri)

static: error(This library cannot be built for static linkage)

TEMPLATE = lib
TARGET   = fftreal

# FFTReal
HEADERS  += Array.h \
            Array.hpp \
            DynArray.h \
            DynArray.hpp \
            FFTRealFixLen.h \
            FFTRealFixLen.hpp \
            FFTRealFixLenParam.h \
            FFTRealPassDirect.h \
            FFTRealPassDirect.hpp \
            FFTRealPassInverse.h \
            FFTRealPassInverse.hpp \
            FFTRealSelect.h \
            FFTRealSelect.hpp \
            FFTRealUseTrigo.h \
            FFTRealUseTrigo.hpp \
            OscSinCos.h \
            OscSinCos.hpp \
            def.h
	    
# Wrapper used to export the required instantiation of the FFTRealFixLen template
HEADERS  += fftreal_wrapper.h
SOURCES  += fftreal_wrapper.cpp

DEFINES  += FFTREAL_LIBRARY




win32 {
    # spectrum_build_dir is defined with a leading slash so that it can
    # be used in contexts such as
    #     ..$${spectrum_build_dir}
    # without the result having a trailing slash where spectrum_build_dir
    # is undefined.
	build_pass {
		CONFIG(release, release|debug): spectrum_build_dir = /release
		CONFIG(debug, release|debug): spectrum_build_dir = /debug
	}
}




CONFIG(debug, debug|release)	{
	#	intentionally blank
}
else	{
	win32	{
		QMAKE_LFLAGS_RELEASE += /DEBUG
		QMAKE_CXXFLAGS_RELEASE += /Zi
		QMAKE_LFLAGS_RELEASE += /OPT:REF
	}
}




# the readme says to specify this flag when compiling in release mode...
CONFIG(release, release|debug): DEFINES += NDEBUG




macx {
    CONFIG += lib_bundle
	# This adds the @rpath prefix to the lib install name (the lib is expected to be bundled with the app package)
	QMAKE_SONAME_PREFIX = @rpath
} else {
	#DESTDIR = ../..$${spectrum_build_dir}
}

EXAMPLE_FILES = bwins/fftreal.def eabi/fftreal.def readme.txt license.txt

#target.path = $$[QT_INSTALL_EXAMPLES]/multimedia/spectrum
#INSTALLS += target

#CONFIG += install_ok  # Do not cargo-cult this!
