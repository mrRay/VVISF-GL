#include "VVStringUtils.hpp"
#include <algorithm>
#include <cstring>
#include <cstdarg>




namespace VVGL
{




/*	========================================	*/
#pragma mark --------------------- string manipulation functions


vector<string> PathComponents(const string & n)	{
	//cout << __PRETTY_FUNCTION__ << endl;
	
	vector<string>		returnMe(0);
	//	count the # of the path delineators in the string, reserve space in the vector we're returning
	size_t				delimCount = count(n.begin(), n.end(), '/') + 1;
	returnMe.reserve(delimCount);
	//	get the ptr to the string data
	char				*inString = const_cast<char*>(n.data());
	//	tokenize the string data, creating strings in the vector we'll be returning
	const char			*delimPtr = "/";
	char				*token = strtok(inString, delimPtr);
	while (token != nullptr)	{
		//cout << "\ttoken: " << token << endl;
		returnMe.push_back(string(token));
		token = strtok(NULL, delimPtr);
	}
	
	return returnMe;
}
string LastPathComponent(const string & n)	{
	size_t		lastSlashIndex = n.find_last_of('/');
	if (lastSlashIndex == string::npos)
		return n;
	else if (lastSlashIndex == (n.length()-1))
		return n;
	return n.substr(lastSlashIndex+1);
}
string StringByDeletingLastPathComponent(const string & n)	{
	size_t		lastSlashIndex = n.find_last_of('/');
	if (lastSlashIndex == string::npos)
		return n;
	else if (lastSlashIndex == 0)
		return n;
	return n.substr(0, lastSlashIndex);
}
string StringByDeletingExtension(const string & n)	{
	size_t			extensionIndex = n.find_last_of(".");
	if (extensionIndex == string::npos)
		return n;
	return n.substr(0, extensionIndex);
}
string StringByDeletingLastAndAddingFirstSlash(const string & n)	{
	if (n.size()<1)
		return string("");
	
	bool		hasFirst = (n[0]=='/') ? true : false;
	bool		hasLast = (n[n.size()-1]=='/') ? true : false;
	string		returnMe = n;
	if (hasLast)
		returnMe.pop_back();
	if (!hasFirst)
		returnMe.insert(0, 1, '/');
	return returnMe;
}
string FmtString(const char * fmt, ...)	{
	va_list			args;
	va_start(args, fmt);
	int				tmpLen = vsnprintf(nullptr, 0, fmt, args) + 1;
	va_end(args);
	
	if (tmpLen < 1)
		return string("");
	
	va_start(args, fmt);
	char			buf[tmpLen];
	memset(buf, 0, tmpLen);
	vsnprintf(buf, tmpLen, fmt, args);
	va_end(args);
	
	return string(buf);
}




}
