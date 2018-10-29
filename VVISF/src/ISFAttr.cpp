#include "ISFAttr.hpp"




namespace VVISF
{




using namespace std;




ISFAttr::ISFAttr(
const string & inName, 
const string & inDesc, 
const string & inLabel, 
const ISFValType & inType, 
const ISFVal & inMinVal, 
const ISFVal & inMaxVal, 
const ISFVal & inDefVal, 
const ISFVal & inIdenVal, 
const vector<string> * inLabels, 
const vector<int32_t> * inVals) : _name(inName), _description(inDesc), _label(inLabel)	{
	//cout << __PRETTY_FUNCTION__ << endl;
	//_name = inName;
	//_description = inDesc;
	//_label = inLabel;
	_type = inType;
	_currentVal = inDefVal;
	_minVal = inMinVal;
	_maxVal = inMaxVal;
	_defaultVal = inDefVal;
	_identityVal = inIdenVal;
	_labelArray = (inLabels==nullptr) ? vector<string>() : vector<string>(*inLabels);
	_valArray = (inVals==nullptr) ? vector<int32_t>() : vector<int32_t>(*inVals);
}
ISFAttr::~ISFAttr()	{
}


double * ISFAttr::updateAndGetEvalVariable()	{
	switch (_type)	{
	case ISFValType_None:
		break;
	case ISFValType_Event:
	case ISFValType_Bool:
	case ISFValType_Long:
	case ISFValType_Float:
		//	update the eval variable's value from the current val
		_evalVariable = _currentVal.getDoubleVal();
		//	return the address of the eval variable
		return &_evalVariable;
		break;
	case ISFValType_Point2D:
	case ISFValType_Color:
	case ISFValType_Cube:
	case ISFValType_Image:
	case ISFValType_Audio:
	case ISFValType_AudioFFT:
		return nullptr;
		break;
	}
	return nullptr;
}


ostream & operator<<(ostream & os, const ISFAttr & n)	{
	os << "<ISFAttr " << const_cast<ISFAttr&>(n).name() << ": " << StringFromISFValType(const_cast<ISFAttr&>(n).type()) << ">";
	return os;
}


void ISFAttr::lengthyDescription()	{
	cout << "<ISFAttr " << name() << ": " << StringFromISFValType(type());
	cout << "\tcurrent:\t" << _currentVal << endl;
	cout << "\tmin:\t\t" << _minVal << endl;
	cout << "\tmax:\t\t" << _maxVal << endl;
	cout << "\tdef:\t\t" << _defaultVal << endl;
	cout << "\tiden:\t" << _identityVal << ">" << endl;
}
string ISFAttr::getAttrDescription()	{
	return FmtString("<ISFAttr %s: %s>", name().c_str(), StringFromISFValType(type()).c_str());
}








}
