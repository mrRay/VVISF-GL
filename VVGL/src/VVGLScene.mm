#include "VVGLScene.hpp"

#if ISF_TARGET_IOS
	#import <GLKit/GLKit.h>
#endif



namespace VVGL
{



#if ISF_TARGET_IOS
VVGLScene::~VVGLScene()	{
	//cout << __PRETTY_FUNCTION__ << "->" << this << endl;
	if (!deleted)
		prepareToBeDeleted();
	
	if (context != nullptr)	{
		delete context;
		context = nullptr;
	}
	
	if (projMatrixEffect != nil)	{
		[(GLKBaseEffect *)projMatrixEffect release];
		projMatrixEffect = nil;
	}
}
void VVGLScene::_configProjMatrixEffect()	{
	if (projMatrixEffect == nil)
		projMatrixEffect = (void *)[[GLKBaseEffect alloc] init];
	GLKEffectPropertyTransform		*trans = [(GLKBaseEffect *)projMatrixEffect transform];
	if (trans != nil)	{
		[trans setModelviewMatrix:GLKMatrix4Identity];
		//[trans setProjectionMatrix:projectionMatrix];
		GLKMatrix4		tmpMatrix = GLKMatrix4Make(
			projMatrix[0], projMatrix[1], projMatrix[2], projMatrix[3],
			projMatrix[4], projMatrix[5], projMatrix[6], projMatrix[7],
			projMatrix[8], projMatrix[9], projMatrix[10], projMatrix[11],
			projMatrix[12], projMatrix[13], projMatrix[14], projMatrix[15]);
		[trans setProjectionMatrix:tmpMatrix];
	}
}
#endif




}