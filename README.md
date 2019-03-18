# Orientation

This repository is home to VVISF, VVGL, and a variety of sample applications that demonstrate their use.

### VVISF

ISF stands for "Interactive Shader Format", and is a file format that describes a GLSL fragment shader, as well as how to execute and interact with it ([the official specification can be found here](https://github.com/mrRay/ISF_Spec)).  VVISF is a cross-platform c++ library that implements the ISF protocol with support for a number of common SDKs (OS X, iOS, GLFW, and Qt) and GL environments (most versions of GL- both ES and desktop- are supported).

### VVGL

VVGL is a small library that performs rudimentary GL rendering, focusing mainly on texture generation/pooling and render-to-texture operations.  Its primary purpose is to provide a simple consistent interface for performing these basic GL operations while obfuscating any platform-specific or environment-specific GL implementation details.  VVISF is built on top of VVGL, but you don't need to be intimately familiar with VVGL to use VVISF.

### Dependencies

All of these dependencies should be included for most use-cases:
* VVGL uses GLEW when compiling against the Qt or GLFW SDKs- precompiled binaries of GLEW are provided for os x (x86_64), as well as win32 and win64 (win binaries only compatible with VC compiler, if you're using mingw then you'll have to compile your own GLEW libraries).
* VVISF uses nlohmann's JSON for Modern C++, a header-only library for JSON parsing and output.  The source code for this is included in ./VVISF/include/nlohmann_json.
* VVISF uses exprtk, a header-only library for high-performance evaluation of strings as values.  The source code for this is included in ./VVISF/include/exprtk.

### Licensing/Copyright

This is all BSD licensed, you can do whatever you want with it and feel good about yourself!  If you have any ideas for improvements (or you find any bugs or problems), please open an issue and let me know.

# Documentation

This project is documented, using both Doxygen and traditional inline comments.  The compiled Doxygen web pages are available at the following URLs, broken down by platform:

<A HREF="https://www.vidvox.net/rays_oddsnends/VVGLVVISF_Docs/Mac/html/modules.html">Mac Documentation</A><BR>
<A HREF="https://www.vidvox.net/rays_oddsnends/VVGLVVISF_Docs/iOS/html/modules.html">iOS Documentation</A><BR>
<A HREF="https://www.vidvox.net/rays_oddsnends/VVGLVVISF_Docs/GLFW/html/modules.html">GLFW Documentation</A><BR>
<A HREF="https://www.vidvox.net/rays_oddsnends/VVGLVVISF_Docs/Qt/html/modules.html">Qt Documentation</A><BR>
<A HREF="https://www.vidvox.net/rays_oddsnends/VVGLVVISF_Docs/RPi/html/modules.html">Raspbian Documentation</A><BR>

...Your first stop should probably be to the "Modules" section, which has a series of walkthroughs that introduce the concepts, classes, and workflows for these libs.

# ISF Shaders

The [ISF Files](https://github.com/Vidvox/ISF-Files) repository contains over 200 different opensource video generators and effects written in ISF that can be used with these libraries.

# Building and Using VVGL & VVISF

VVGL and VVISF are intended to be built and used as shared libraries.  Precompiled binaries are not distributed because the expectation is that the binaries you compile of VVGL and VVISF are going to be specific to your use-case (you'll likely be compiling them against a specific SDK/operating system, and even within the same OS/SDK binaries may not be compatible from compiler to compiler).

Compiling VVGL is hopefully straightforward- detailed per-SDK sample projects and instructions follow, but all of these project files do one thing before compiling: they define which SDK they're compiling VVGL against.  This can be done in one of two ways: either with a compiler flag, or by modifying the file "VVGL_HardCodedDefines.hpp".  More detailed information can be found in "VVGL_Defines.hpp" if you're interested- if no SDK has been defined, the compiler will throw a human-readable error pointing this out.

### VVGL/VVISF in Mac OS X/iOS

This repository includes an xcode project (examples/apple/ISFSandbox.xcodeproj) that has already been configured to compile VVGL and VVISF as frameworks that can be embedded in and deployed with your software.  The same xcode project also includes a number of sample applications for OS X and iOS demonstrating how to include and use the frameworks.

Adding VVGL.framework and VVISF.framework to an existing xcode project:
- Add the project file ISFSandbox.xcodeproj to your workspace.
- Add VVGL.framework and VVISF.framework as dependencies for your target (if you're compiling them for iOS, add VVGL_iOS and VVISF_iOS instead).
- Navigate to the "Build Phases" UI in XCode for your target.  Locate the "Link Binary With Libraries" section, and add VVGL.framework and VVISF.framework to the list (click the "+" button and locate the frameworks in the list).
- Add a new "Copy Files" build phase.  Set its destination to "Frameworks".  Add VVGL.framework and VVISF.framework to this list (click the "+" button and then navigate to the frameworks in the "Products" folder).
- Navigate to the "Build Settings" UI in XCode for your target.  Locate the "Other C Flags" section, and define which VVGL SDK your target will be using by adding the flag "-DVVGL_SDK_MAC".  As an alternative to adding this build setting, you can define the SDK by modifying the file "VVGL_HardCodedDefines.hpp".

### VVGL/VVISF in Qt

This repository includes a Qt project (examples/Qt/Qt.pro) that compiles VVGL and VVISF as shared libraries- the same project also includes two sample apps demonstrating the basic use of these shared libraries and VVGL/VVISF in Qt, two texture upload/download benchmark apps, and a full-featured cross-platform port of the original cocoa ISF editor.

After adding the shared libraries and their accompanying header files to your project file, you'll have to define which SDK the Qt project you're adding VVGL to is using.  This is done by defining the compiler flag VVGL_SDK_QT, either in the Qt project file (DEFINES += VVGL_SDK_QT), or by un-commenting the appropriate line in "VVGL_HardCodedDefines.hpp".

### VVGL/VVISF in GLFW

This repository contains an xcode project demonstrating compiling VVGL/VVISF against GLFW on OS X.  While GLFW supports other platforms/IDEs/SDKs, the process for adding VVGL/VVISF to a GLFW project is basically the same:
- Make sure you define which SDK you're compiling VVGL and your app against by either adding a compiler flag that defines VVGL_SDK_GLFW or by un-commenting VVGL_SDK_GLFW in "VVGL_HardCodedDefines.hpp".  Compile VVGL/VVISF.
- Compile your software, linking it against the compiled VVGL/VVISF libraries.  If you didn't modify VVGL_HardCodedDefines.hpp" then you'll have to add the VVGL_SDK_GLFW compiler flag to the target, too.
- Make sure the compiled libraries are included with your target software.

### VVGL/VVISF on the Raspberry Pi

Because GL support on the Raspberry Pi isn't quite as robust as GL support on desktop or mobile platforms with more powerful dedicated hardware, the facilities for working with GL code on this platform are comparatively limited.  Your best best is to use the makefile to build the sample app in "./examples/raspbian".

### Learning More

This documentation only covers the biggest, most commonly used objects in these libraries- the information that you'd need to know as a developer to quickly add and start using VVISF in your project.  The source code itself is heavily documented, and all of this stuff is simple and straightforward enough that you should feel comfortable digging around if you want to do something that isn't covered here.

