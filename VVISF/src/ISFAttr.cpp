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
const vector<int32_t> * inVals) : name(inName), description(inDesc), label(inLabel)	{
	//cout << __PRETTY_FUNCTION__ << endl;
	//name = inName;
	//description = inDesc;
	//label = inLabel;
	type = inType;
	currentVal = inDefVal;
	minVal = inMinVal;
	maxVal = inMaxVal;
	defaultVal = inDefVal;
	identityVal = inIdenVal;
	labelArray = (inLabels==nullptr) ? vector<string>() : vector<string>(*inLabels);
	valArray = (inVals==nullptr) ? vector<int32_t>() : vector<int32_t>(*inVals);
}
ISFAttr::~ISFAttr()	{
}


double * ISFAttr::updateAndGetEvalVariable()	{
	switch (type)	{
	case ISFValType_None:
		break;
	case ISFValType_Event:
	case ISFValType_Bool:
	case ISFValType_Long:
	case ISFValType_Float:
		//	update the eval variable's value from the current val
		evalVariable = currentVal.getDoubleVal();
		//	return the address of the eval variable
		return &evalVariable;
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
	os << "<ISFAttr " << const_cast<ISFAttr&>(n).getName() << ": " << ISFValTypeString(const_cast<ISFAttr&>(n).getType()) << ">";
	return os;
}


void ISFAttr::lengthyDescription()	{
	cout << "<ISFAttr " << getName() << ": " << ISFValTypeString(getType());
	cout << "\tcurrent:\t" << currentVal << endl;
	cout << "\tmin:\t\t" << minVal << endl;
	cout << "\tmax:\t\t" << maxVal << endl;
	cout << "\tdef:\t\t" << defaultVal << endl;
	cout << "\tiden:\t" << identityVal << ">" << endl;
}
string ISFAttr::getAttrDescription()	{
	return FmtString("<ISFAttr %s: %s>", getName().c_str(), ISFValTypeString(getType()).c_str());
}








}
