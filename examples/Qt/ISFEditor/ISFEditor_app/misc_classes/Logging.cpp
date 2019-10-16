#include "Logging.h"

#if defined(Q_OS_MAC)
#include "Logging_Mac.h"
#elif defined(Q_OS_WIN)
//#include "Logging_Win.h"
#endif




void ConfigureLogging()	{

#if defined(Q_OS_MAC)
	ConfigureLogging_Mac();
#elif defined(Q_OS_WIN)
	//	do win stuff here
#endif

}



