#import <UIKit/UIKit.h>
#import <VVGL/VVGL.hpp>
#import <VVISFKit/ISFKit.h>
#import "VVBufferGLKView.h"




using namespace VVGL;
using namespace VVISF;




@interface ViewController : UIViewController	{
	VVGLContextRef		baseCtx;
	ISFSceneRef			isfScene;
	
	IBOutlet VVBufferGLKView		*bufferView;
}


@end

