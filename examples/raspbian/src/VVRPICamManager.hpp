#ifndef VVRPICamManager_hpp
#define VVRPICamManager_hpp


//#include "bcm_host.h"
//#include "interface/vcos/vcos.h"

//#include "interface/vcos/vcos.h"
//#include "EGL/eglext_brcm.h"

#include "interface/mmal/mmal.h"
#include "interface/mmal/mmal_logging.h"
#include "interface/mmal/mmal_buffer.h"
#include "interface/mmal/util/mmal_util.h"
#include "interface/mmal/util/mmal_util_params.h"
#include "interface/mmal/util/mmal_default_components.h"
#include "interface/mmal/util/mmal_connection.h"




class VVRPICamManager	{
	public:
		MMAL_COMPONENT_T		*camComponent = nullptr;
		
		
		VVRPICamManager();
		~VVRPICamManager();
};


#endif /*	VVRPICamManager_hpp	*/