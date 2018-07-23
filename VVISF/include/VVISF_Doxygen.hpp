#ifndef VVISF_DOXYGEN_H
#define VVISF_DOXYGEN_H


#include "VVISF.hpp"





/*!
\defgroup VVISF_BASIC VVISF- Basic Classes

\brief These are the classes that comprise VVISF.

\detail
	- VVISF::ISFDoc
	- VVISF::ISFScene
	- VVISF::ISFErr and VVISF::ISFErrType
	- VVISF::ISFVal
	- VVISF::ISFAttr
	- VVISF::ISFPassTarget
*/








/*!
\defgroup VVGL_SAMPLE_1 Sample Code I- Make a context

<b>Creating a GLContext</b><BR>
If you want to work with OpenGL then you need an "OpenGL context", because a context is the primary interface for accessing GPU hardware using OpenGL.  Every platform has its own native SDK for working with OpenGL, and they all have subtle differences- VVGL::GLContext/#GLContextRef is an attempt to create a cross-platform class that presents the same interface across all platforms, but the exact call to create a GLContext is going to change slightly depending on which platform you're compiling VVGL against.  The same holds true if you're compiling VVGL to use with another cross-platform GL solution, like GLFW or Qt.<BR>

<b>Creating a new GLContext from an existing GLContext (all SDKs)</b>
\code{.cpp}
GLContextRef		origCtx;	//	this is assumed to be non-nil in the real world...
GLContextRef		newCtx = origCtx->newContextSharingMe();
\endcode

<b>Creating a new GLContext- simplest approach, but least control over the kind of context that gets created (all SDKs)</b>
\code{.cpp}
GLContextRef		newCtx = CreateNewGLContextRef();
\endcode

<b>Creating a GLContext with the mac SDK:</b>
\code{.cpp}
NSOpenGLContext		*origMacCtx;	//	this is assumed to be non-nil in the real world...
CGLContextObj		tmpMacCtx = [origMacCtx CGLContextObj];
CGLPixelFormatObj	tmpMacPxlFmt = [[origMacCtx pixelFormat] CGLPixelFormatObj];

//	this makes a GLContext that wraps (and retains) an existing 
//	mac context (doesn't create a new OpenGL context)
GLContextRef		vvglCtx = CreateGLContextRefUsing(tmpMacCtx, tmpMacCtx, tmpMacPxlFmt);

//	this makes a GLContext that creates a new OpenGL context.  this new 
//	context shares the passed context (they can share resources)
GLContextRef		vvglCtx = CreateNewGLContextRef(tmpMacCtx, tmpMacPxlFmt);

//	if you don't have an existing mac context- if you're creating the first context, for example...

//	this makes a GLContext (and new OpenGL context) using 
//	the compatibility version of GL (GL 2.1 on os x)
GLContextRef		vvglCtx = CreateNewGLContextRef(NULL, CreateCompatibilityGLPixelFormat());

//	this makes a GLContext (and new OpenGL context) using GL4
GLContextRef		vvglCtx = CreateNewGLContextRef(NULL, CreateGL4PixelFormat());
\endcode

<b>Creating a GLContext with the iOS SDK:</b>
\code{.cpp}
EAGLContext			*tmpCtx;	//	this is assumed to be non-nil in the real world...

//	this makes a GLContext that wraps (and retains) an existing 
//	iOS context (doesn't create a new OpenGL context)
GLContextRef		vvglCtx = CreateGLContextRefUsing(tmpCtx);
\endcode

<b>Creating a GLContext with the GLFW SDK:</b>
\code{.cpp}
GLFWwindow *	window;	//	this is assumed to be non-nil in the real world...

//	this makes a GLContext that wraps the window's GL context 
//	(doesn't create a new OpenGL context)
GLContextRef	ctxRef = CreateGLContextRefUsing(window);
\endcode

<b>Creating a GLContext with the Qt SDK:</b>
\code{.cpp}
QSurface *			origSfc;	//	this is assumed to be non-nil in the real world...
QOpenGLContext *	origCtx;	//	this is assumed to be non-nil in the real world...
QSurfaceFormat		origSfcFmt;	//	this is assumed to be non-nil in the real world...

//	this makes a GLContext that wraps and establishes a strong ref to 
//	the passed vars (doesn't create a new OpenGL context)
GLContextRef		vvglCtx = CreateGLContextRefUsing(origSfc, origCtx, origSfcFmt);

//	this makes a GLContext that creates a new OpenGL context.  this new 
//	context shares the passed context (they can share resources)
GLContextRef		vvglCtx = CreateNewGLContextRef(origSfc, origCtx, origSfcFmt);

//	if you don't have an existing Qt context- if you're creating the first context, for example...

//	this makes a GLContext (and new OpenGL context) using the default version of GL...
GLContextRef		vvglCtx = CreateNewGLContextRef(nullptr, nullptr, CreateDefaultSurfaceFormat());

//	this makes a GLContext (and new OpenGL context) using GL 4...
GLContextRef		vvglCtx = CreateNewGLContextRef(nullptr, nullptr, CreateGL4SurfaceFormat());
\endcode
*/








/*!
\defgroup VVGL_SAMPLE_2 Sample Code II- GLBuffer & GLBufferPool

<b>GLBuffer</b><BR>
VVGL::GLBuffer represents a buffer of some kind- it could be a buffer on the GPU (VRAM), or it could be a buffer on the CPU (RAM).  Most of the time- almost all the time- a GLBuffer will represent an image.  

The preferred interface for working with GLBuffer instances is #GLBufferRef, which wraps a GLBuffer up in a std::shared_ptr<GLBuffer>.  GLBuffer instances should be freed as soon as they are no longer in use- freeing a GLBuffer will, if appropriate, automatically return its resources to the buffer pool that created it.

Generally speaking, you probably won't be modifying GLBuffer instances- a very common workflow is "create a buffer, render into it, display the texture or use the texture as an input for rendering something else, delete the buffer".  This isn't a "rule"- there's nothing explicitly preventing you from modifying a GLBuffer instance- it's just not a pattern employed by these libs or anything I've written with them.  Destroying a buffer just returns it to the pool where it's available for re-use, so this workflow doesn't generally result in lots of textures being created/destroyed.

There are several different kinds of GLBuffers- here are the more common ones you'll encounter:
- Texture-type GLBuffers are probably the most common type you'll work with- they store an image in a GL texture.
- CPU-type GLBuffers store data in RAM- they do not have any associated GPU resources.  These libs focus on GPU-related resources, but CPU-based images are still encountered when uploading data to textures (loading images, playing back video) or downloading data from textures (saving images or video).  CPU-type GLBuffers can be created with memory allocated by this lib- or from blocks of memory that were allocated by something else (you can provide a release callback so the GLBuffer can retain objects from other classes for the duration of its lifetime).
- PBO-type GLBuffers store data in OpenGL pixel buffer objects (PBOs).  PBOs are a simple, cross-platform way of efficiently uploading data to and downloading data from the texture, and you may receive these if you're using GLTexToCPUCopier to download texture data to system memory and haven't explicitly provided a CPU-type GLBuffer to copy into.  The image-related properties of the PBO-type GLBuffer are populated with the values of the texture-type GLBuffer.
- VBO-type GLBuffers store data in OpenGL vertex buffer objects (VBOs).  Typically these are used to provide geometry data to OpenGL- because of this, it generally doesn't make sense to treat these as if they were images.

Because GLBuffers are generally treated as content-immutable, many of its more useful values are available by directly accessing the instance variables.
- 'desc' is a VVGL::GLBuffer::Descriptor that describes the basic hardware properties of the GPU or CPU resources associated with a GLBuffer instance.  This contains things like the internal format, pixel format, pixel type, backing type, target, etc.
- 'name' is the name of the GL resource
- 'size' is a VVGL::Size struct describing the size of the GPU/CPU resource.  Note that the image represented by the GLBuffer may not occupy the full size of the GPU/CPU resource.
- 'srcRect' is a VVGL::Rect that describes the region of the GPU/CPU resource that contains the image represented by this GLBuffer.
- 'flipped' is a bool that describes whether or not the content in 'srcRect' is flipped vertically.  Many SDKs vend image data that is "upside down", and by tracking this on a resource-by-resource basis it can be compensated for naturally without having to perform any unnecessary copies.
- 'contentTimestamp' is a VVGL::Timestamp struct that describes a time using the common 64-bit/32-bit value pair found in many SDKs.  VVGL::GLBufferPool automatically timestamps any buffers it creates, or you can use this struct to provide your own timestamps from another API.
- 'pboMapped' is a bool that describes whether or not the PBO has been mapped into system memory.  This is only valid for PBO-type GLBuffers, but if it's true the PBO's data can be accessed at the GLBuffer's 'cpuBackingPtr'.
- 'cpuBackingPtr' is a pointer to any resources associated with the GLBuffer in system memory.  If the receiver is a CPU-type GLBuffer, or a PBO-type GLBuffer that is mapped, its data can be accessed for both reading and writing at this pointer.

<b>GLBufferPool</b><BR>
GLBufferPool is the class responsible for creating, pool, and destroying GL assets like textures, PBOs, etc.  This lib defines a global singleton that you should populate during setup by passing it a GLContext that it can use to create/destroy resources.

There are a variety of buffer creation functions listed in (\ref VVGL_BUFFERCREATE)<BR>


<b>Creating the global buffer pool, then a couple GL resources:</b>
\code{.cpp}
//	first create a shared context using one of the above methods (this is just a quick example)
GLContextRef		sharedContext = CreateNewGLContextRef();

//	make the global buffer pool- this line creates the global buffer pool 
//	using the shared context (the buffer pool will use the shared context's 
//	OpenGL context to create or destroy any GL resources).
CreateGlobalBufferPool(sharedContext);

//	make the global buffer pool- this is the same function call, but it 
//	creates a new context (a new OpenGL context) for the buffer pool.
CreateGlobalBufferPool(sharedContext->newContextSharingMe());

//	makes a 1920x1080 GL texture (32 bits per pixel).  the texture 
//	will be created by the global buffer pool's GL context
GLBufferRef			tmpTex = CreateRGBATex(VVGL::Size(1920,1080));

//	makes a 1920x1080 GL texture (32 bits per pixel).  the texture 
//	will be created by whatever GL context is current in the executing thread.
//	If no GL context is current, this won't work!
GLBufferRef			tmpTex = CreateRGBATex(VVGL::Size(1920,1080), true);

//	makes a 1920x1080 GL texture (128 bits per pixel).
GLBufferRef			tmpTex = CreateRGBAFloatTex(VVGL::Size(1920,1080));

//	makes a CPU-based buffer- this GLBuffer doesn't have any GL resources,
//	it's entirely RAM-based.  the memory that contains the image data is 
//	allocated by this lib.
GLBufferRef			tmpCPUImg = CreateRGBACPUBuffer(VVGL::Size(1920,1080));

//	makes a CPU-based buffer- this GLBuffer doesn't have any GL resources 
//	either, but the memory that contains the image data has been allocated 
//	by another library and will be freed when the GLBuffer is deleted and its 
//	backing release callback (the block below) is executed.
VVGL::Size			cpuImageSize(1920,1080);
VVGL::Size			cpuBackingSize(1924,1080);	//	compensates for padding
void				*cpuBitmapData = XXX;
AnotherAPIsImageObject		*someImageRef = XXX;
GLBufferRef			tmpCPUImg = CreateRGBACPUBufferUsing(
	cpuImageSize,	//	image size
	cpuBitmapData,	//	bitmap data
	cpuBackingSize,	//	bitmap size
	someImageRef,	//	release callback context
	[](GLBuffer & inBuffer, void * inReleaseCallbackContext)	{
		AnotherAPIsImageObject		*tmpObj = (AnotherAPIsImageObject *)inReleaseCallbackContext;
		//	...free 'tmpObj' here...
	}
	);
\endcode
*/








/*!
\defgroup VVGL_SAMPLE_3 Sample Code III- Texture copy/upload/download

An instance of GLBuffer represents an OpenGL resource, or it contains a pointer to system memory.  In either case, one thing is certain: if you want "a copy of a GLBuffer instance", you cannot simply make another copy of the struct- bad things will happen as the underlying image data either won't be duplicated.

It is strongly recommended that you use #GLBufferRef instead of GLBuffer if you just want to share a given GLBuffer instance with another object (displaying the same texture in two different views, for example).  As long as you use #GLBufferRef and ensure that you set any GLBufferRefs you maintain to nullptr as soon as you no longer use them, all the underlying GPU/CPU resources will be freed/pooled and you'll have plenty of VRAM available.  Remember, GLBuffers come from and are returned to pools: obtaining a new buffer and setting a buffer to nil are typically very fast and very cheap (within reason).

\code{.cpp}
//	allocate an RGBA OpenGL texture
GLBufferRef			origTex = CreateRGBATex(VVGL::Size(1920,1080));
//	print the name and source rect of the RGBA texture
cout << "origTex is " << orgTex->name << ", srcRect is " << origTex->srcRect << endl;

//	make another var- both vars refer to the same underlying GLBuffer instance
GLBufferRef			sameGLBufferNewVar = origTex;
//	modify the srcRect of the new texture var
sameGLBufferNewVar->srcRect = VVGL::Rect(10,10,1900,1060);

//	print the name and source rect of the new var
cout << "sameGLBufferNewVar is " << sameGLBufferNewVar->name;
cout << ", srcRect is " << sameGLBufferNewVar->srcRect << endl;

//	print the source rect of the original- observe that it has changed!
cout << "origTex srcRect is now " << origTex->srcRect << endl;
\endcode

#GLBufferRef is great, but if you're working with it and you modify a property- like 'srcRect', for example- then every other #GLBufferRef for that same GLBuffer will immediately inherit that change.  This is not always desirable- for example, you may want to crop a #GLBufferRef for use in one particular scene, or use a larger texture-based GLBuffer as the source of many smaller texture-based GLBuffers that all share the same GL resource (texture atlas).  The function GLBufferCopy() accepts a #GLBufferRef, makes a new GLBuffer instance that has a strong reference to the GLBuffer instance you passed in, and returns a #GLBufferRef for the new instance.  Because it's an entirely different GLBuffer, you can modify the superficial properties without affecting the original GLBuffer or any of its refs- but because it's the same underlying GL resource, nothing was actually copied and the resource will automatically be retained as long as necessary.  Copying buffers in this manner is quick and cheap because the actual image data isn't being duplicated.

\code{.cpp}
//	allocate an RGBA OpenGL texture
GLBufferRef			origTex = CreateRGBATex(VVGL::Size(1920,1080));
//	print the name and source rect of the RGBA texture
cout << "origTex is " << orgTex->name << ", srcRect is " << origTex->srcRect << endl;

//	make a copy of the GLBufferRef- this makes another 
//	instance of GLBuffer that retains the passed GLBuffer.
GLBufferRef			sameTexNewBufferNewVar = GLBufferCopy(origTex);
//	modify the srcRect of the new texture var
sameTexNewBufferNewVar->srcRect = VVGL::Rect(10,10,1900,1060);

//	print the name and source rect of the new var
cout << "sameTexNewBufferNewVar is " << sameTexNewBufferNewVar->name;
cout << ", srcRect is " << sameTexNewBufferNewVar->srcRect << endl;

//	print the source rect of the original- observe that it has changed!
cout << "origTex srcRect is " << origTex->srcRect << endl;	
\endcode

Sometimes you actually want to copy the contents of a GLBuffer to another GLBuffer.  For this, VVGL offers a couple different classes that specialize in the more commonly encountered types of copies so far:
	- GLTexToTexCopier copies texture-type GLBuffers to other texture-type GLBuffers.  Texture-to-texture copies are relatively fast, and resizing them is relatively easy.
\code{.cpp}
//	Same creation semantics as GLScene: if you don't explicitly provide an OpenGL 
//	context, it will create its own context in the global buffer pool's sharegroup.
GLTexToTexCopierRef		texToTexCopier = CreateGLTexToTexCopierRef();

//	make a source texture, then copy it into a new texture
GLBufferRef			tmpSrcTex = CreateRGBATex(VVGL::Size(1920,1080));
GLBufferRef			tmpDstTex = texToTexCopier->copyToNewBuffer(tmpSrcTex);
\endcode
	- GLTexToCPUCopier copies texture-type GLBuffers to CPU-type GLBuffers.  It has two basic access points, one which is suitable for performing immediate one-shot texture downloads, and another asynchronous mode which has been optimized to be substantially more efficient.
\code{.cpp}
//	Same creation semantics as GLScene: if you don't explicitly provide an OpenGL 
//	context, it will create its own context in the global buffer pool's sharegroup.
GLTexToCPUCopierRef		texToCPUCopier = CreateGLTexToCPUCopierRef();

//	make a source texture- we're going to download this
GLBufferRef			tmpTexBuffer = CreateRGBATex(VVGL::Size(1920,1080));

//	download the source texture to system memory immediately- good for quick one-shot downloads
GLBufferRef			tmpBuffer = texToCPUCopier->downloadTexToCPU(tmpTexBuffer);
//	because we didn't provide a CPU-type buffer, 'tmpBuffer' is a PBO-type buffer
//	that has been mapped into system memory, and can be accessed like this:
if (tmpBuffer!=nullptr && tmpBuffer->pboMapped)	{
	void				*tmpBufferData = tmpBuffer->cpuBackingPtr;
	uint32_t			bytesPerRow = tmpBuffer->calculateBackingBytesPerRow();
	uint32_t			totalLength = tmpBuffer->calculateBackingLength();
}

//	if you're receiving a stream of textures to download, there's a more efficient way:
GLBufferRef			newFrameTexture = XXX;	//	provided by something else in the real world
//	GLTexToCPUCopier uses a PBO to do async/n-buffered downloads, so the 
//	first frame or two won't get returned until you call it again later.
GLBufferRef			downloadedFrame = texToCPUCopier->streamTexToCPU(newFrameTexture);
if (downloadedFrame != nullptr)	{
	void		*downloadedData = NULL;
	uint32_t	bytesPerRow = 0;
	uint32_t	totalLength = 0;
	if ((downloadedFrame->desc.type == Type_PBO && downloadedFrame->pboMapped)	||
	(downloadedFrame->desc.type == Type_CPU))
	{
		downloadedData = downloadedFrame->cpuBackingPtr;
		bytesPerRow = downloadedFrame->calculateBackingBytesPerRow();
		totalLength = downloadedFrame->calculateBackingLength();
	}
	//	..now do something with the downloaded data- encode to video, export to image, etc.
}
\endcode
	- GLCPUToTexCopier copies CPU-type GLBuffers to texture-type GLBuffers.  It also has two basic access modes, one which is optimized for immediate upload and another, more efficient/optimized mode for streaming data to textures.
\code{.cpp}
//	Same creation semantics as GLScene: if you don't explicitly provide an OpenGL 
//	context, it will create its own context in the global buffer pool's sharegroup.
GLCPUToTexCopierRef		cpuToTexCopier = CreateGLCPUToTexCopierRef();

GLBufferRef			tmpCPUTypeBuffer = XXX;	//	non-null in the real world

//	upload the cpu buffer to a GL texture immediately- good for quick one-shot uploads
GLBufferRef			tmpBuffer = cpuToTexCopier->uploadCPUToTex(tmpCPUTypeBuffer);

//	if you're receiving a stream of images to upload, there's a more efficient pipeline:
GLBufferRef			newFrameCPUBuffer = XXX;	//	non-null in the real world
//	GLCPUToTexCopier uses a PBO to do async/n-buffered uploads, so the
//	first frame or two won't get returned until you call it again later.
GLBufferRef			uploadedFrame = cpuToTexCopier->streamCPUToTex(newFrameCPUBuffer);
if (uploadedFrame != nullptr)	{
	//	do something with the uploaded texture
}
\endcode
*/








/*!
\defgroup VVGL_SAMPLE_4 Sample Code IV- Examing ISF files: ISFDoc

<b>Making and working with ISFDoc</b><BR>
VVISF::ISFDoc is how you interface with ISF files- you can create a VVISF::ISFDoc from a path to a file on disk, or from the frag/vertex shader strings.  When the doc is created, the JSON blob in the frag shader string is prased, and the VVISF::ISFDoc instance is populated with a variety of #ISFAttrRef and #ISFPassTargetRef instances that describe the contents and value of the ISF file.  This allows the properties of the ISF document to be examined programmatically.

As with other classes in these libs, #ISFDocRef is the preferred means of interacting with ISFDoc.  Because it contains attributes and passes and lots of things that have value, performing a deep copy isn't something you want to do unnecesarily.  #ISFDocRef allows an instance of ISFDoc to be accessed simultaneously from number of different places (ISFDoc is threadsafe).

VVISF::ISFDoc performs a variety of important functions beyond introspection: it generates source code for GL shaders with all the appropriate variable declarations for the ISF variables, and it maintains the state of the inputs and render passes and is used by ISFScene when doing rendering, etc.  Much of VVISF::ISFScene's logic revolves around getting values from and sending values to the VVISF::ISFDoc it's using to store and track the state/value of all the shader inputs used to render frames.

\code{.cpp}
//	ISFDoc can throw a variety of exceptions, so we'll define it in a try block...
ISFDocRef		myDoc = nullptr;
try	{
	//	You can create an ISFRef from a given file on disk...
	myDoc = CreateISFDocRef(string("/some/path/some_ISF.fs"));
	
	//	...or you can create an ISFRef from frag/vert shader strings...
	myDoc = CreateISFDocRef(someFragStringVar, someVertStringVar);

}
catch (ISFErr & exc)	{
	cout << "ERR: caught exception: " << exc.genTypeString() << ": " << exc.general << "- " << exc.specific << endl;
}

//	print some basic information about the doc
if (myDoc != nullptr)	{
	cout << "successfully loaded ISF doc!\n";
	cout << "description: " << myDoc->getDescription() << endl;
	cout << "file type: " << ISFFileTypeString(myDoc->getType()) << endl;
	
	auto			attrs = myDoc->getInputs();
	cout << "file\'s doc has " << attrs.size() << " attributes\n";
	ISFAttrRef		firstAttr = (attrs.size()<1) ? nullptr : attrs.front();
	if (firstAttr == nullptr)
		cout << "first attr is null\n";
	else
		cout << "first attr is " << firstAttr->getAttrDescription() << endl;
	
	auto			passes = myDoc->getRenderPasses();
	cout << "the ISF doc has " << passes.size() << " render passes\n";
}
else
	cout << "err: problem loading the ISFDoc!\n";
\endcode
*/








/*!
\defgroup VVGL_SAMPLE_5 Sample Code V- ISFScene renders ISF files

<b>Creating an ISFScene</b><BR>

VVISF::ISFScene is a subclass of VVGL::GLScene, and as such shares its facilities for rendering to texture, simple resizing, etc.  Likewise, creating VVISF::ISFScene has the same semantics as creating a VVGL::GLScene- if you don't explicitly provide a VVGL::GLContext for it to use on creation, it will create its own context in the global buffer pool's sharegroup.

\code{.cpp}
//	put together a shared context and create the global buffer pool
GLContextRef		sharedCtx = CreateGLContextRefUsing()/CreateNewGLContextRef()/etc;
CreateGlobalBufferPool(sharedCtx);

//	create an ISFScene- this line creates an ISFScene that makes a new 
//	GL context which shares the global buffer pool's context
ISFSceneRef			tmpScene = CreateISFSceneRef();

//	another way to create an ISFScene- this line creates an ISFScene that 
//	uses the shared context directly (it doesn't create a new GL context)
ISFSceneRef			tmpScene = CreateISFSceneRefUsing(sharedCtx);

//	...and here's a third way to create a scene- this line creates a new 
//	GL context and uses it (functionally identical to the first example)
ISFSceneRef			tmpScene = CreateISFSceneRefUsing(sharedCtx->newContextSharingMe());
\endcode


<b>Loading an ISF file and rendering it to a GL texture</b><BR>

Once you have an ISFScene, you can render it to a texture using any of the functions inherited from VVGL::GLScene.  ISFScene::createAndRenderABuffer() is perhaps the simplest function- if you have more specific needs, consult the documentation for ISFScene.

\code{.cpp}
//	put together a shared context and create the global buffer pool
GLContextRef		sharedCtx = CreateGLContextRefUsing()/CreateNewGLContextRef()/etc;
CreateGlobalBufferPool(sharedCtx);

//	create an ISFScene- this line creates an ISFScene that makes a new 
//	GL context which shares the global buffer pool's context
ISFSceneRef			tmpScene = CreateISFSceneRef();
//	configure the ISFScene to throw exceptions during file loading/shader compiling
tmpScene->setThrowExceptions(true);

try	{
	//	tell the ISFScene to load a file
	tmpScene->useFile(string("/path/to/ISF_file.fs"));
	
	//	tell the ISFScene to render a frame
	GLBufferRef			tmpFrame = tmpScene->createAndRenderABuffer(VVGL::Size(1920,1080));
}
catch (ISFErr & exc)	{
	cout << "ERR: caught exception: " << exc.genTypeString() << ": " << exc.general << "- " << exc.specific << endl;
}
\endcode
*/








/*!
\defgroup VVGL_SAMPLE_6 Sample Code VI- Interacting with ISFScene

<b>Interacting with the ISFScene</b><BR>

Internally, VVISF::ISFScene uses VVISF::ISFDoc to represent the value and state of an ISF file during rendering.  This means that you can get an ISFScene's currently-used ISFDoc, and adjust rendering parameters by getting the appropriate attribute from the doc and changing its value.

\code{.cpp}
ISFSceneRef			tmpScene = XXX;	//	this is assumed to be non-nil in the real world...

//	get the scene's doc- interacting with the doc affects rendering
ISFDocRef			tmpDoc = tmpScene->getDoc();
if (tmpDoc == nullptr)
	return;

//	get the attribute from the doc for the fake float input, make sure it's a float
ISFAttrRef			floatAttr = tmpDoc->getInput(string("floatInputName"));
if (floatAttr == nullptr || floatAttr->getType() != ISFValType_Float)
	return;

//	get the float attribute's current, min, and max vals
ISFVal				currentVal = floatAttr->getCurrentVal();
ISFVal				minVal = floatAttr->getMinVal();
ISFVal				maxVal = floatAttr->getMaxVal();
double				tmpVal = currentVal.getDoubleVal();

//	add .01 to the current val, looping the value around the min/max
tmpVal += 0.01;
if (!maxVal.isNullVal() && tmpVal > maxVal.getDoubleVal())	{
	if (!minVal.isNullVal())
		tmpVal = minVal.getDoubleValue();
	else
		tmpVal = 0.;
}
currentVal = ISFFloatVal(tmpVal);

//	apply the new value we calculated to the attribute
floatAttr->setCurrentVal(currentVal);
\endcode
*/








/*!
\defgroup VVGL_SAMPLE_7 Sample Code VII- GLScene

VVGL::GLScene is a frontend for a performing custom render commands in an OpenGL context.  It's a relatively simple class- it lets you provide a block that executes drawing commands and performs a series of rendering steps that can be customized, with built-in support for orthogonal rendering, rendering to textures/#GLBufferRef, and support for a variety of different versions of OpenGL.

Following are some examples that demonstrate creating and configuring a GLScene, and then rendering it into a texture.

<BR>
<b>Creating and GLScene and rendering it to a texture for older versions of GL:</b>
\code{.cpp}
//	make the shared context, set up the global buffer pool to use it
GLContextRef		sharedContext = CreateNewGLContextRef();
CreateGlobalBufferPool(sharedContext);
//	you can create a scene around an existing context or- as shown here- make a new context for the scene
GLSceneRef			glScene = CreateGLSceneRef();
//	configure the scene's render callback
glScene->setRenderCallback([](const GLScene & inScene)	{
	//	populate a tex quad with the geometry, tex, and color vals- this struct is organized in a manner compatible with GL such that it's both easy to work with and can be uploaded directly.
	Quad<VertXYZRGBA>		texQuad;
	VVGL::Size				sceneSize = inScene.getOrthoSize();
	texQuad.populateGeo(VVGL::Rect(0,0,sceneSize.width,sceneSize.height));
	texQuad.bl.color = GLColor(1., 1., 1., 1.);
	texQuad.tl.color = GLColor(1., 0., 0., 1.);
	texQuad.tr.color = GLColor(0., 1., 0., 1.);
	texQuad.br.color = GLColor(0., 0., 1., 1.);
	
	//	configure GL to expect vertex and color coords when it draws.
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	
	//	set the pointers to the vertex and color coords
	glVertexPointer(texQuad.bl.geo.numComponents(), GL_FLOAT, texQuad.stride(), &texQuad.bl.geo[0]);
	glColorPointer(texQuad.bl.color.numComponents(), GL_FLOAT, texQuad.stride(), &texQuad.bl.color[0]);
	
	//	draw
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
});
//	create a GLBuffer for an OpenGL texture, render the scene to it
GLBufferRef			renderedTexture = CreateRGBATex(VVGL::Size(1920,1080));
glScene->renderToBuffer(renderedTexture);
\endcode


<BR>
<b>Creating a GLScene and rendering it to a texture for desktop GL 3+:</b>
\code{.cpp}
//	make the shared context, set up the global buffer pool to use it
GLContextRef		sharedContext = CreateNewGLContextRef();
CreateGlobalBufferPool(sharedContext);
//	you can create a scene around an existing context or- as shown here- make a new context for the scene
GLSceneRef			glScene = CreateGLSceneRef();
//	create the vertex & fragment shaders, tell the scene to use them
string			vsString("\r\
#version 330 core\r\
in vec3		inXYZ;\r\
in vec4		inRGBA;\r\
uniform mat4	vvglOrthoProj;\r\
out vec4		programRGBA;\r\
void main()	{\r\
	gl_Position = vec4(inXYZ.x, inXYZ.y, inXYZ.z, 1.0) * vvglOrthoProj;\r\
	programRGBA = inRGBA;\r\
}\r\
");
	string			fsString("\r\
#version 330 core\r\
in vec4		programRGBA;\r\
out vec4		FragColor;\r\
void main()	{\r\
FragColor = programRGBA;\r\
}\r\
");
glScene->setVertexShaderString(vsString);
glScene->setFragmentShaderString(fsString);

//	we're going to create a VAO, we also need attribs for the XYZ and RGBA inputs, and quad to store the last-uploaded data
static GLBufferRef				vao = nullptr;
static GLCachedAttribRef		xyzAttr = make_shared<GLCachedAttrib>("inXYZ");
static GLCachedAttribRef		rgbaAttr = make_shared<GLCachedAttrib>("inRGBA");
static Quad<VertXYZRGBA>		vboData = Quad<VertXYZRGBA>();

//	configure the scene's render prep callback to cache the location of the vertex attributes and uniforms
glScene->setRenderPrepCallback([](const GLScene & inScene, const bool & inReshaped, const bool & inPgmChanged)	{
	if (inPgmChanged)	{
		//	cache all the locations for the vertex attributes & uniform locations
		GLint				myProgram = inScene.getProgram();
		xyzAttr->cacheTheLoc(myProgram);
		rgbaAttr->cacheTheLoc(myProgram);
		
		//	make a new VAO
		vao = CreateVAO(true);
	}
});

//	configure the scene's render callback to pass the data to the GL program if it's changed (the params to targetQuad were animated, but i took it out because it wasn't x-platform)
glScene->setRenderCallback([](const GLScene & inScene)	{
	VVGL::Size			orthoSize = inScene.getOrthoSize();
	VVGL::Rect			boundsRect(0, 0, orthoSize.width, orthoSize.height);
	Quad<VertXYZRGBA>		targetQuad;
	targetQuad.populateGeo(boundsRect);
	targetQuad.bl.color = GLColor(1., 1., 1., 1.);
	targetQuad.tl.color = GLColor(1., 0., 0., 1.);
	targetQuad.tr.color = GLColor(0., 1., 0., 1.);
	targetQuad.br.color = GLColor(0., 0., 1., 1.);
	
	//	bind the VAO
	glBindVertexArray(vao->name);
	
	uint32_t			vbo = 0;
	//	if the target quad differs from the vbo data, there's been a change/animation and we need to push new data to the GL program
	if (vboData != targetQuad)	{
		vboData = targetQuad;
		//	make a new VBO to contain vertex + color data
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(targetQuad), (void*)(&targetQuad), GL_STATIC_DRAW);
		//	configure the attribute pointers to use the VBO
		if (xyzAttr->loc >= 0)	{
			glVertexAttribPointer(xyzAttr->loc, targetQuad.bl.geo.numComponents(), GL_FLOAT, GL_FALSE, targetQuad.stride(), BUFFER_OFFSET(targetQuad.geoOffset()));
			xyzAttr->enable();
		}
		if (rgbaAttr->loc >= 0)	{
			glVertexAttribPointer(rgbaAttr->loc, targetQuad.bl.color.numComponents(), GL_FLOAT, GL_FALSE, targetQuad.stride(), BUFFER_OFFSET(targetQuad.colorOffset()));
			rgbaAttr->enable();
		}
	}
	
	//	draw
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	//	un-bind the VAO
	glBindVertexArray(0);
	
	//	if we created a vbo earlier, delete it now (the vao will retain it)
	if (vbo != 0)	{
		glDeleteBuffers(1, &vbo);
	}
});
//	create a GLBuffer for an OpenGL texture, render the scene to it
GLBufferRef			renderedTexture = CreateRGBATex(VVGL::Size(1920,1080));
glScene->renderToBuffer(renderedTexture);
\endcode
*/








#endif // VVISF_DOXYGEN_H
