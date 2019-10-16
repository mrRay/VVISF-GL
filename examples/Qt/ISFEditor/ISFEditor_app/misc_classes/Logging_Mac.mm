#include "Logging_Mac.h"

#import "VVLogger.h"




void ConfigureLogging_Mac()	{

#if defined(QT_DEBUG)
	
#elif defined(QT_NO_DEBUG)
	[[VVLogger alloc] initWithFolderName:@"ISFEditor" maxNumLogs:20];
	[[VVLogger globalLogger] redirectLogs];
#endif

}



