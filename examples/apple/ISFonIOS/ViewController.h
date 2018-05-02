#import <UIKit/UIKit.h>
#import <VVGL/VVGL.hpp>
#import <VVISF/VVISF.hpp>
#import "VVBufferGLKView.h"




using namespace VVGL;
using namespace VVISF;




@interface ViewController : UIViewController	{
	GLContextRef		baseCtx;
	ISFSceneRef			isfScene;
	
	IBOutlet VVBufferGLKView		*bufferView;
}


@end

