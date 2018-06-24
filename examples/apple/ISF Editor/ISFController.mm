//
//  ISFController.m
//  ISF Syphon Filter Tester
//
//  Created by bagheera on 11/2/13.
//  Copyright (c) 2013 zoidberg. All rights reserved.
//

#import "ISFController.h"
#import "VVKQueueCenter.h"
#import "ISFEditorAppDelegate.h"
#import "BufferObject.h"
#import "ISFStringAdditions.h"




#define VVRELEASE(item) {if (item != nil)	{			\
	[item release];										\
	item = nil;											\
}}

#define ISFITEMHEIGHT 50
#define ISFITEMSPACE 10

#define VVFMTSTRING(f, ...) ((NSString *)[NSString stringWithFormat:f, ##__VA_ARGS__])




@implementation ISFController


- (id) init	{
	//NSLog(@"%s",__func__);
	if (self = [super init])	{
		scene = nullptr;
		targetFile = nil;
		itemArray = [[MutLockArray alloc] init];
		return self;
	}
	[self release];
	return nil;
}
- (void) awakeFromNib	{
	[self setRenderSize:NSMakeSize(640,360)];
	[self _pushRenderingResolutionToUI];
}
- (void) dealloc	{
	//NSLog(@"%s",__func__);
	//VVRELEASE(scene);
	scene = nullptr;
	VVRELEASE(targetFile);
	VVRELEASE(itemArray);
	[VVKQueueCenter removeObserver:self];
	[super dealloc];
}
- (void) setSharedGLContext:(GLContextRef)n	{
	//NSLog(@"%s",__func__);
	if (n==nullptr)
		return;
	@synchronized (self)	{
		//VVRELEASE(scene);
		scene = nullptr;
		//scene = [[ISFGLScene alloc] initWithSharedContext:n pixelFormat:[GLScene defaultPixelFormat] sized:NSMakeSize(320,240)];
		scene = CreateISFSceneRefUsing(n->newContextSharingMe());
		scene->setOrthoSize(VVGL::Size(320.,240.));
		//[scene setThrowExceptions:YES];
		scene->setThrowExceptions(true);
	}
}


- (IBAction) widthFieldUsed:(id)sender	{
	NSString	*tmpString = [widthField stringValue];
	NSNumber	*tmpNum = (tmpString==nil) ? nil : [tmpString numberByEvaluatingString];
	if (tmpNum==nil)
		return;
	NSSize		tmpSize = [self renderSize];
	tmpSize.width = [tmpNum intValue];
	[self setRenderSize:tmpSize];
	[self _pushUIToRenderingResolution];
}
- (IBAction) heightFieldUsed:(id)sender	{
	NSString	*tmpString = [heightField stringValue];
	NSNumber	*tmpNum = (tmpString==nil) ? nil : [tmpString numberByEvaluatingString];
	if (tmpNum==nil)
		return;
	NSSize		tmpSize = [self renderSize];
	tmpSize.height = [tmpNum intValue];
	[self setRenderSize:tmpSize];
	[self _pushUIToRenderingResolution];
}
- (IBAction) doubleResClicked:(id)sender	{
	NSSize		tmpSize = [self renderSize];
	tmpSize = NSMakeSize(ceil(tmpSize.width*2.0), ceil(tmpSize.height*2.0));
	[self setRenderSize:tmpSize];
	[self _pushRenderingResolutionToUI];
}
- (IBAction) halveResClicked:(id)sender	{
	NSSize		tmpSize = [self renderSize];
	tmpSize = NSMakeSize(ceil(tmpSize.width/2.0), ceil(tmpSize.height/2.0));
	[self setRenderSize:tmpSize];
	[self _pushRenderingResolutionToUI];
}
- (void) _pushUIToRenderingResolution	{
	NSString		*tmpString = nil;
	uint32_t		tmpInt = 0;
	NSSize			newSize;
	tmpString = [widthField stringValue];
	tmpInt = (tmpString==nil || [tmpString numberByEvaluatingString]==nil) ? 640 : [[tmpString numberByEvaluatingString] intValue];
	newSize.width = tmpInt;
	
	tmpString = [heightField stringValue];
	tmpInt = (tmpString==nil || [tmpString numberByEvaluatingString]==nil) ? 360 : [[tmpString numberByEvaluatingString] intValue];
	newSize.height = tmpInt;
	
	[self setRenderSize:newSize];
}
- (void) _pushRenderingResolutionToUI	{
	if (![NSThread isMainThread])	{
		dispatch_async(dispatch_get_main_queue(), ^{
			[self _pushRenderingResolutionToUI];
			return;
		});
		return;
	}
	NSSize		tmpSize = [self renderSize];
	[widthField setStringValue:VVFMTSTRING(@"%d",(int)tmpSize.width)];
	[heightField setStringValue:VVFMTSTRING(@"%d",(int)tmpSize.height)];
}
@synthesize renderSize;


- (void) loadFile:(NSString *)f	{
	//NSLog(@"%s ... %@",__func__,f);
	if (f != nil)	{
		VVRELEASE(targetFile);
		targetFile = [f retain];
	}
	
	NSFileManager		*fm = [NSFileManager defaultManager];
	//	if the passed file doesn't exist, just bail immediately
	if (![fm fileExistsAtPath:f])
		return;
	[VVKQueueCenter removeObserver:self];
	[VVKQueueCenter addObserver:self forPath:f];
	NSString		*vertShaderName = [[f stringByDeletingPathExtension] stringByAppendingPathExtension:@"vs"];
	if (![fm fileExistsAtPath:vertShaderName])	{
		vertShaderName = [[f stringByDeletingPathExtension] stringByAppendingPathExtension:@"vert"];
		if ([fm fileExistsAtPath:vertShaderName])
			[VVKQueueCenter addObserver:self forPath:vertShaderName];
	}
	
	
	@try	{
		//	i only want to fetch the shaders if the file isn't nil!
		[appDelegate setFetchShaders:(f==nil)?NO:YES];
		//if (f==nil)
		//	NSLog(@"\t\tfetchShaders is NO in %s",__func__);
		//else
		//	NSLog(@"\t\tfetchShaders is YES in %s",__func__);
		
		//[scene _renderLock];
		//sceneIsFilter = [ISFFileManager _isAFilter:f];
		//[scene useFile:f];
		try	{
			scene->useFile(string([f UTF8String]));
		}
		catch (ISFErr & exc)	{
			NSString	*generalString = [NSString stringWithFormat:@"%s", exc.getTypeString().c_str()];
			NSString	*specificString = [NSString stringWithFormat:@"%s, %s", exc.general.c_str(), exc.specific.c_str()];
			dispatch_async(dispatch_get_main_queue(), ^{
				//NSString	*tmpString = [err reason];
				VVRunAlertPanel(generalString,specificString,@"Oh snap!",nil,nil);
			});
		}
		catch (...)	{
		}
		
		
		ISFDocRef		sceneDoc = scene->getDoc();
		sceneIsFilter = (sceneDoc==nullptr || sceneDoc->getType()!=ISFFileType_Filter) ? NO : YES;
	}
	@catch (NSException *err)	{
		dispatch_async(dispatch_get_main_queue(), ^{
			NSString	*tmpString = [err reason];
			VVRunAlertPanel([err name],tmpString,@"Oh snap!",nil,nil);
		});
	}
	//[scene _renderUnlock];
	[self populateUI];
	[appDelegate _isfFileReloaded];
}
- (void) file:(NSString *)p changed:(u_int)fflag	{
	//NSLog(@"%s ... %@",__func__,p);
	
	NSString		*tmpPath = p;
	//NSLog(@"\t\ttmpPath is %@",tmpPath);
	if (tmpPath != nil)	{
		[tmpPath retain];
		//[self loadFile:p];
		dispatch_async(dispatch_get_main_queue(), ^{
			NSString	*newPath = [[tmpPath stringByDeletingPathExtension] stringByAppendingPathExtension:@"fs"];
			[self loadFile:newPath];
		});
		[tmpPath release];
		tmpPath = nil;
	}
}
- (VVGL::GLBufferRef) renderFXOnThisBuffer:(VVGL::GLBufferRef)n passDict:(NSMutableDictionary *)d	{
	//NSLog(@"%s",__func__);
	if (n==nullptr)	{
		//NSLog(@"\t\terr: bailing, passed a null buffer, %s",__func__);
		return nil;
	}
	
	//if ([scene filePath]==nil)
	//	return nil;
	if (scene == nullptr)	{
		//NSLog(@"\t\terr: bailing, scene null, %s",__func__);
		return n;
	}
	ISFDocRef	tmpDoc = scene->getDoc();
	if (tmpDoc == nullptr)
		return nil;
	string		scenePath = tmpDoc->getPath();
	if (scenePath.length() < 1)	{
		//NSLog(@"\t\terr: bailing, scene doesn't have a file, %s",__func__);
		return n;
	}
	//	apply the passed buffer to the scene as "inputImage"
	//[scene setFilterInputImageBuffer:n];
	//[scene setBuffer:n forInputImageKey:@"inputImage"];
	scene->setBufferForInputNamed(n,string("inputImage"));
	//	run through the inputs, getting their values and pushing them to the scene
	[itemArray rdlock];
	for (ISFUIItem *itemPtr in [itemArray array])	{
		//id		tmpVal = [itemPtr getNSObjectValue];
		//if (tmpVal != nil)	{
			//NSLog(@"\t\t%@ - %@",[itemPtr name],tmpVal);
		//	[scene setNSObjectVal:tmpVal forInputKey:[itemPtr name]];
		//}
		
		NSString			*itemName = [itemPtr name];
		if (itemName != nil)	{
			const ISFVal		tmpVal = [itemPtr getISFVal];
			scene->setValueForInputNamed(tmpVal, string([[itemPtr name] UTF8String]));
		}
	}
	[itemArray unlock];
	
	//VVBuffer		*returnMe = nil;
	VVGL::GLBufferRef		returnMe = nullptr;
	
	//returnMe = [scene
	//	allocAndRenderToBufferSized:((sceneIsFilter) ? [n srcRect].size : [self renderSize])
	//	prefer2DTex:NO
	//	passDict:d];
	//returnMe = [scene allocAndRenderToBufferSized:[n srcRect].size prefer2DTex:NO passDict:d];
	
	//returnMe = CreateRGBATex(VVGL::Size(n->srcRect.size));
	map<int32_t,GLBufferRef>		tmpPassDict;
	
	try	{
		//scene->renderToBuffer(returnMe, n->srcRect.size, (d==nil) ? nullptr : &tmpPassDict);
		NSSize		tmpSize = [self renderSize];
		VVGL::Size		renderSize = (sceneIsFilter) ? VVGL::Size(n->srcRect.size) : VVGL::Size(tmpSize.width, tmpSize.height);
		returnMe = scene->createAndRenderABuffer(renderSize, (d==nil)?nullptr:&tmpPassDict, GetGlobalBufferPool());
	}
	catch (ISFErr & exc)	{
		cout << "ERR: " << __PRETTY_FUNCTION__ << "-> caught exception: " << exc.getTypeString() << ": " << exc.general << ", " << exc.specific << endl;
		NSString			*general = [NSString stringWithUTF8String:exc.general.c_str()];
		NSString			*specific = [NSString stringWithUTF8String:exc.specific.c_str()];
		
		NSMutableDictionary		*errDict = [NSMutableDictionary dictionaryWithCapacity:0];
		map<string,string>		&details = exc.details;
		for (auto const & it : details)	{
			NSString		*key = [NSString stringWithUTF8String:it.first.c_str()];
			NSString		*val = [NSString stringWithUTF8String:it.second.c_str()];
			if (key!=nil && val!=nil)
				[errDict setObject:val forKey:key];
		}
		
		NSException		*ex = [NSException
			exceptionWithName:(general==nil) ? @"general" : general
			reason:(specific==nil) ? @"specific" : specific
			userInfo:errDict];
		[ex raise];
	}
	
	
	
	
	
	/*
	scene->renderToBuffer(returnMe, n->srcRect.size, (d==nil) ? nullptr : &tmpPassDict);
	*/
	for (auto const & it : tmpPassDict)	{
		//	key is first, val is second
		BufferObject		*bo = [BufferObject createWithBuffer:it.second];
		NSNumber			*num = [NSNumber numberWithInt:it.first];
		if (bo!=nil && num!=nil)
			[d setObject:bo forKey:num];
	}
	return returnMe;
}
//	only used to render for recording
- (void) renderIntoBuffer:(VVGL::GLBufferRef)b atTime:(double)t	{
	if (b==nullptr)
		return;
	//if ([scene filePath]==nil)
	//	return;
	ISFDocRef	tmpDoc = scene->getDoc();
	if (tmpDoc == nullptr)
		return;
	string		scenePath = tmpDoc->getPath();
	if (scenePath.length() < 1)
		return;
	//	run through the inputs, getting their values and pushing them to the scene
	[itemArray rdlock];
	for (ISFUIItem *itemPtr in [itemArray array])	{
		//id		tmpVal = [itemPtr getNSObjectValue];
		//if (tmpVal != nil)	{
			//NSLog(@"\t\t%@ - %@",[itemPtr name],tmpVal);
		//	[scene setNSObjectVal:tmpVal forInputKey:[itemPtr name]];
		//}
		NSString		*itemName = [itemPtr name];
		if (itemName != nil)	{
			const ISFVal		tmpVal = [itemPtr getISFVal];
			scene->setValueForInputNamed(tmpVal, string([itemName UTF8String]));
		}
	}
	[itemArray unlock];
	
	//[scene renderToBuffer:b sized:[b srcRect].size renderTime:t passDict:nil];
	scene->renderToBuffer(b, b->srcRect.size, t);
}


- (void) populateUI	{
	//NSLog(@"%s",__func__);
	[itemArray wrlock];
	for (ISFUIItem *itemPtr in [itemArray array])	{
		[itemPtr removeFromSuperview];
	}
	[itemArray removeAllObjects];
	[itemArray unlock];
	
	if (scene == nullptr)
		return;
	ISFDocRef	tmpDoc = scene->getDoc();
	if (tmpDoc == nullptr)
		return;
	string		scenePath = tmpDoc->getPath();
	if (scenePath.length() < 1)	{
		//NSLog(@"\t\tbailing, scene hasn't loaded a file, %s",__func__);
		return;
	}
	
	NSSize			newDocViewSize = [self calculateDocViewSize];
	NSView			*dv = [uiScrollView documentView];
	//NSRect			docVisRect = [uiScrollView documentVisibleRect];
	
	vector<ISFAttrRef>	sceneInputs = scene->getInputs();
	if (sceneInputs.size()<1)
		return;
	//	resize the doc view to hold everything
	//[dv setFrame:NSMakeRect(0, 0, docVisRect.size.width, (ISFITEMHEIGHT+ISFITEMSPACE)*[sceneInputs count])];
	[dv setFrame:NSMakeRect(0, 0, newDocViewSize.width, newDocViewSize.height)];
	//docVisRect = [uiScrollView documentVisibleRect];
	
	//	set the size of the temp rect used to create UI items
	NSRect			tmpRect;
	tmpRect.size.width = [dv frame].size.width;
	tmpRect.size.height = ISFITEMHEIGHT;
	tmpRect.origin.x = 0.0;
	//tmpRect.origin.y = VVMAXY([dv frame]) - tmpRect.size.height - ISFITEMSPACE;
	tmpRect.origin.y = 0;
	
	for (auto it=sceneInputs.rbegin(); it!=sceneInputs.rend(); ++it)	{
		//ISFAttrRef		attrib = it.value();
		ISFAttrRef		attrib = *it;
		if (attrib == nullptr)
			continue;
		
		ISFValType		attribType = attrib->getType();
		ISFUIItem		*newElement = nil;
		switch (attribType)	{
		case ISFValType_None:
			break;
		case ISFValType_Event:
		case ISFValType_Bool:
		case ISFValType_Long:
		case ISFValType_Float:
		case ISFValType_Color:
		case ISFValType_Image:
		case ISFValType_Audio:
		case ISFValType_AudioFFT:
			if (!attrib->getIsFilterInputImage())	{
				tmpRect.size.height = ISFITEMHEIGHT;
				newElement = [[ISFUIItem alloc] initWithFrame:tmpRect attrib:attrib];
				[dv addSubview:newElement];
				[itemArray lockAddObject:newElement];
				[newElement release];
				tmpRect.origin.y += (tmpRect.size.height + ISFITEMSPACE);
			}
			break;
		case ISFValType_Point2D:
			//tmpRect.size.height = 2.*ISFITEMHEIGHT;
			tmpRect.size.height = ISFITEMHEIGHT;
			newElement = [[ISFUIItem alloc] initWithFrame:tmpRect attrib:attrib];
			[dv addSubview:newElement];
			[itemArray lockAddObject:newElement];
			[newElement release];
			tmpRect.origin.y += (tmpRect.size.height + ISFITEMSPACE);
			break;
		case ISFValType_Cube:
			NSLog(@"\t\tskipping creation of UI item for cube input %s",attrib->getAttrDescription().c_str());
			break;
		}
	}
}
- (NSSize) calculateDocViewSize	{
	NSSize				returnMe = NSMakeSize([uiScrollView documentVisibleRect].size.width, 0);
	if (scene == nullptr)
		return returnMe;
	for (const auto & attrib : scene->getInputs())	{
		ISFValType		attribType = attrib->getType();
		switch (attribType)	{
		case ISFValType_None:
			break;
		case ISFValType_Event:
		case ISFValType_Bool:
		case ISFValType_Long:
		case ISFValType_Float:
		case ISFValType_Color:
		case ISFValType_Image:
		case ISFValType_Audio:
		case ISFValType_AudioFFT:
			returnMe.height += (ISFITEMHEIGHT + ISFITEMSPACE);
			break;
		case ISFValType_Point2D:
			//returnMe.height += ((2. * ISFITEMHEIGHT) + ISFITEMSPACE);
			returnMe.height += (ISFITEMHEIGHT + ISFITEMSPACE);
			break;
		case ISFValType_Cube:
			break;
		}
	}
	return returnMe;
}


- (void) passNormalizedMouseClickToPoints:(NSPoint)p	{
	//NSLog(@"%s ... (%0.2f, %0.2f)",__func__,p.x,p.y);
	if (scene == nullptr)
		return;
	for (const auto & attrib : scene->getInputsOfType(ISFValType_Point2D))	{
		string &		attribNameCPP = attrib->getName();
		NSString		*attribName = [NSString stringWithUTF8String:attribNameCPP.c_str()];
		
		ISFVal			minVal = attrib->getMinVal();
		ISFVal			maxVal = attrib->getMaxVal();
		NSPoint			tmpPoint;
		if (!minVal.isNullVal() || !maxVal.isNullVal() || (minVal.getPointValByIndex(0)==maxVal.getPointValByIndex(0) && minVal.getPointValByIndex(1)==maxVal.getPointValByIndex(1)))	{
			VVGL::Size		tmpSize = scene->getOrthoSize();
			NSSize			lastRenderSize = NSMakeSize(tmpSize.width, tmpSize.height);
			tmpPoint = NSMakePoint(p.x*lastRenderSize.width, p.y*lastRenderSize.height);
		}
		else
			tmpPoint = NSMakePoint(p.x*(maxVal.getPointValByIndex(0)-minVal.getPointValByIndex(0))+minVal.getPointValByIndex(0), p.y*(maxVal.getPointValByIndex(1)-minVal.getPointValByIndex(1))+minVal.getPointValByIndex(1));
		
		[itemArray rdlock];
		for (ISFUIItem *itemPtr in [itemArray array])	{
			if ([[itemPtr name] isEqualToString:attribName])	{
				[itemPtr setNSObjectValue:[NSValue valueWithPoint:tmpPoint]];
				break;
			}
		}
		[itemArray unlock];
	}
}


/*
- (ISFGLScene *) scene	{
	return scene;
}
*/
- (VVISF::ISFSceneRef) scene	{
	return scene;
}
- (NSString *) targetFile	{
	return targetFile;
}
- (void) reloadTargetFile	{
	if (targetFile==nil)
		return;
	[self loadFile:[[targetFile copy] autorelease]];
}


@end
