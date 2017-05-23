#include "VVRPICamManager.hpp"
#include <iostream>

#include "VVStringUtils.hpp"
#include <stdexcept>




VVRPICamManager::VVRPICamManager()	{
	using namespace std;
	
	MMAL_STATUS_T		status;
	
	//	create the component
	status = mmal_component_create(MMAL_COMPONENT_DEFAULT_CAMERA, &camComponent);
	if (status != MMAL_SUCCESS)	{
		throw std::runtime_error(VVGL::FmtString("\tERR: can't create component, %d/%x\n",status,status).c_str());
	}
	
}
VVRPICamManager::~VVRPICamManager()	{

}
