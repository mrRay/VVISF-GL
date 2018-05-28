#ifndef VVISF_DOXYGEN_H
#define VVISF_DOXYGEN_H




/*!
\defgroup VVISF_SAMPLE VVISF-Sample Code I

This repository includes a variety of sample apps that demonstrate the use of VVISF on several different platforms, but some sample code snippets for common actions in VVISF are listed here as a quick overview:

<BR>
<b>Making and working with ISFDoc</b>
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
	cout << "the ISF doc has " << myDoc->getInputs().size() << " INPUTS\n";
	cout << "the ISF doc has " << myDoc->getRenderPasses() << " render passes\n";
}
else
	cout << "err: problem loading the ISFDoc!\n";
\endcode

<b>Creating an ISFScene</b>
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

<b>Loading an ISF file and rendering it to a GL texture</b>
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

<b>Interacting with the ISFScene</b>
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


#endif // VVISF_DOXYGEN_H
