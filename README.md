# ISFSandbox


what's up, fellas-

- the source in 'VVGL' builds the base VVGL lib.  it doesn't have to be a separate lib, you can probably cram all the source into a project and skip the "lib" aspect entirely, but it was designed as a lib so i don't have to include ISF if i don't need it.
	- VVGLContext is a context abstraction; differs greatly on a platform-by-platform basis, provides other stuff in these libs with an opaque way to "set current context"
	- VVGLBuffer is a wrapper for a GL buffer of some sort (pretty much always a texture, but can also be a renderbuffer, a CPU-backed texture, etc).  VVGLBufferRef is a shared_ptr to a VVGLBuffer, and is what most things in the framework vend.  you can't construct a VVGLBuffer instance by assignment or by copying to prevent ownership confusion- a VVGLBuffer instance "owns" its GL texture and needs to hand that ownership to the pool when it is deleted (this is why almost everything in these libs use VVGLBufferRef instead of VVGLBuffer).  if you want to create another VVBuffer instance that refers to the same GL resource as an existing VVGLBuffer, use the VVGLBufferCopy() method (the buffer it returns will retain the buffer it was passed- this is for doing stuff like texture atlases).  you don't need to know most of this crap, for the most part this should be opaque and i'm only explaining it because you're dan/dave.
	- VVGLBufferPool/VVGLBufferPoolRef is a buffer pool.  you need to create a global buffer pool, and you create it using a VVGLContext (you want the buffer pool to be in the same sharegroup as the other GL contexts you're using it with).  call "housekeeping()" on the buffer pool once per render pass (at the end of the pass).
	- VVGLScene wraps a GL context with some basic vars that describe an orthogonal rendering context with simple rendering methods that accept and return VVGLBufferRefs.
	- VVGLShaderScene is a subclass of VVGLScene, and adds a frag/vert shader
	- VVBufferCopier copies VVBuffers by drawing them into new textures.  this class creates new GL resources, which differs from copying a VVGLBufferRef by assignment or using VVGLBufferCopy() to create a new VVGLBuffer.
- the source in 'VVISF' builds the ISF stuff.  this lib has two dependencies (both of which are incldued)- nlohmann_json (c++11-style json support) and exprtk (expression evaluation).  this lib can take a really long time to compile, and its binary is going to be rather huge because of exprtk.
	- ISFDoc describes an ISF document.  pass it a path to a .fs file, it will parse the file and create data structures that describe it.  if you have a folder of .fs files, you would create an ISFDoc for each path, and do some introspection on the ISF docs to determine how you want to handle the ISF files in your application.  ISFDoc will also generate frag and vert shaders on command, and has all the internal data structure necessary to persistently render an ISF file.  i think you'll probably use this a good bit.
		- ISFAttr describes a single ISF attribute fully (type of output, min/max range, default, current val- the works).  ISFAttrRef is a shared_ptr to an ISFAttr.  an ISFDoc, on creation, populates itself with ISFAttrRefs.
		- ISFPassTarget describes an ISF render pass.
	- ISFVal is a wrapper for the values that you would want to pass to ISF
	- ISFScene is a subclass of VVGLShaderScene, and renders ISF files to VVGLBufferRefs.  internally, ISFScene:
		- creates an ISFDoc when told to load a file
		- tells the ISFDoc to generate frag and vert shaders
		- during each rendering pass, it runs through the ISFDoc's attributes, pushing their values to the shader (protip: ISFAttr can cache uniform locations!)
		- renders each pass to a texture, making the textures available to subsequent rendering passes



