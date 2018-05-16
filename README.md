# Orientation

This repository is home to VVISF, VVGL, and a variety of sample applications that demonstrate their use.

### VVISF

ISF stands for "Interactive Shader Format", and is a file format that describes a GLSL fragment shader, as well as how to execute and interact with it ([the official specification can be found here](https://github.com/mrRay/ISF_Spec)).  VVISF is a cross-platform c++ library that implements the ISF protocol with support for a number of common SDKs (OS X, iOS, GLFW, and Qt) and GL environments (most versions of GL- both ES and desktop- are supported).

### VVGL

VVGL is a small library that performs rudimentary GL rendering, focusing mainly on texture generation/pooling and render-to-texture operations.  Its primary purpose is to provide a simple consistent interface for performing basic GL operations while obfuscating any platform-specific or environment-specific GL implementation details.  VVISF is built on top of VVGL, but you don't need to be intimately familiar with VVGL to use VVISF.

### Dependencies

All of these dependencies should be included for most use-cases:
* VVGL uses GLEW when compiling against the Qt or GLFW SDKs- precompiled binaries of GLEW are provided for os x (x86_64), as well as win32 and win64 (win binaries only compatible with VC compiler, if you're using mingw then you'll have to compile your own GLEW libraries).
* VVISF uses nlohmann's JSON for Modern C++, a header-only library for JSON parsing and output.  The source code for this is included in ./VVISF/include/nlohmann_json.
* VVISF uses exprtk, a header-only library for high-performance evaluation of strings as values.  The source code for this is included in ./VVISF/include/exprtk.

### Licensing/Copyright

This is all BSD licensed, you can do whatever you want with it and feel good about yourself!  If you have any ideas for improvements (or you find any bugs or problems), please open an issue and let me know.

# Building and Using VVGL & VVISF

VVGL and VVISF are intended to be built and used as shared libraries.  Precompiled binaries are not distributed because the expectation is that the binaries you compile of VVGL and VVISF are going to be specific to your use-case (you'll likely be compiling them against a specific SDK/operating system, and even within the same OS/SDK binaries may not be compatible from compiler to compiler).

### Building for Mac OS X/iOS

This repository includes an xcode project (examples/apple/ISFSandbox.xcodeproj) that has already been configured to compile VVGL and VVISF as frameworks that can be embedded in and deployed with your software.  The same xcode project also includes a number of sample applications for OS X and iOS demonstrating how to include and use the frameworks.  If you want to build your own library or include the source directly you'll need to define the compiler flag ISF_SDK_MAC during compilation.

### Building for Qt

This repository includes a Qt project (examples/Qt/Qt.pro) that compiles VVGL and VVISF as shared libraries- the same project also includes two sample apps demonstrating the basic use of these shared libraries and VVGL/VVISF in Qt.  If you want to build your own library or include the source directly you'll need to define the compiler flag ISF_SDK_QT during compilation.

### Building for GLFW

### Building for the Raspberry Pi

# Getting Started with VVGL/VVISF

### VVGL Basics

### VVISF Basics

### Learning More

This documentation only covers the biggest, most commonly used objects in these libraries- the information that you'd need to know as a developer to quickly add and start using VVISF in your project.  The source code itself is heavily documented, and all of this stuff is simple and straightforward enough that you should feel comfortable digging around if you want to do something that isn't covered here.

### Adding more platforms/SDKs to VVGL/VVISF

* Create a new "VVGL_SDK_****" define for the platform/SDK you want to add.  Open VVGL_Defines.hpp, and add your new SDK define to the if/elif/endif statement, being sure to define which GL environments you want your SDK to be compiled against.  This just defines which GL environments (like GLES/GL4/etc) VVGL/VVISF will be compiled against- you should make sure that all GL environments that will be available at runtime are specified here (VVGL and VVISF support multiple versions of GL at runtime, but when you compile these libs you need to ensure that they include code for the relevant versions of OpenGL you'll be working with).
* Locate GLContext.hpp and GLContext.cpp.  GLContext is a platform-agnostic representation of a GL context- you need to use the VVGL_SDK you created to populate the instance variables and methods of GLContext with the SDK-specific variables it needs to interact with OpenGL in your target SDK.  Typically, this means that GLContext needs a strong reference to your SDK-specific GL context object.  GLContext.hpp and GLContext.cpp are populated with source code for at least four different SDKs, all of which are very similar to one another and can be used as an exemplar.
* The texture types available differ from platform to platform- for convenience, you'll want to create a header file that lists the enums for the OpenGL texture targets/internal formats/pixel formats/pixel types available in your SDK.  You should pattern this header file after one of the existing header files (GLBuffer_XXXX_Enums.h), and ensure that it's included in GLBuffer.hpp using the VVGL_SDK define you created.
* GLBufferPool.hpp declares a variety of functions that create GL textures, returned as GLBufferRef instances.  These functions are defined in GLBufferPool.cpp, and you should check their implementations briefly to ensure that the resources they're creating are available on your platform (it's likely that you'll get a compiler error if they aren't, but it can't hurt to check).

