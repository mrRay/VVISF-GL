#include "ISFStringUtils.hpp"

#include "ISFVal.hpp"
//#include "VVGL_Range.hpp"

#include "exprtk/exprtk.hpp"




namespace VVISF
{


using namespace std;


/*	========================================	*/
#pragma mark --------------------- string manipulation functions


ISFVal ParseStringAsBool(const string & n)	{
	string			testStrings[4] = { string("yes"), string("true"), string("no"), string("false") };
	for (int testStrIdx=0; testStrIdx<4; ++testStrIdx)	{
		if (testStrings[testStrIdx].size() == n.size())	{
			bool		match = true;
			auto		itA = testStrings[testStrIdx].begin();
			auto		itB = n.begin();
			for (itA=testStrings[testStrIdx].begin(), itB=n.begin(); itA!=testStrings[testStrIdx].end(); ++itA, ++itB)	{
				if (tolower(*itA) != (*itB))	{
					match = false;
					break;
				}
			}
			if (match)	{
				if (testStrIdx < 2)
					return ISFBoolVal(true);
				else
					return ISFBoolVal(false);
			}
		}
	}
	return ISFNullVal();
}

ISFVal ISFValByEvaluatingString(const string & n, const map<string, double> & inSymbols)	{
	exprtk::expression<double>		expr;
	exprtk::symbol_table<double>	table;
	exprtk::parser<double>			parser;
	
	size_t			inSymbolsCount = inSymbols.size();
	//double			tmpVars[inSymbolsCount];
	double			*tmpVars = (double*)malloc(sizeof(double)*inSymbolsCount);
	
	if (inSymbolsCount > 0)	{
		//vector<double>		tmpVars;
		//tmpVars->reserve(inSymbolsCount);
		int				i=0;
		for (auto const & it : inSymbols)	{
			//double		tmpDouble = it.second;
			//table.add_variable(it.first.c_str(), tmpDouble);
			tmpVars[i] = it.second;
			table.add_variable(it.first.c_str(), tmpVars[i]);
			++i;
		}
		expr.register_symbol_table(table);
	}
	free(tmpVars);
	tmpVars = nullptr;
	
	parser.compile(n, expr);
	
	return ISFFloatVal(expr.value());
}

Range LexFunctionCall(const string & inBaseStr, const Range & inFuncNameRange, vector<string> & outVarArray)	{
	//cout << __PRETTY_FUNCTION__ << endl;
	
	if (inFuncNameRange.len==0 || inFuncNameRange.max()>inBaseStr.size())
		return Range(0,0);
	//if (inFuncLen==0 || ((inFuncLen+inFuncLen)>inBaseStr.size()))
	//	return Range(0,0);
	size_t		searchStartIndex = inFuncNameRange.max();
	size_t		lexIndex = searchStartIndex;
	size_t		openGroupingCount = 0;
	Range	substringRange(0,0);
	substringRange.loc = searchStartIndex + 1;
	do	{
		switch (inBaseStr[lexIndex])	{
		case '(':
		case '{':
			++openGroupingCount;
			break;
		case ')':
		case '}':
			--openGroupingCount;
			if (openGroupingCount == 0)	{
				substringRange.len = lexIndex - substringRange.loc;
				string		groupString = inBaseStr.substr(substringRange.loc, substringRange.len);
				groupString = TrimWhitespace(groupString);
				outVarArray.push_back(groupString);
			}
			break;
		case ',':
			if (openGroupingCount == 1)	{
				substringRange.len = lexIndex - substringRange.loc;
				string		groupString = inBaseStr.substr(substringRange.loc, substringRange.len);
				groupString = TrimWhitespace(groupString);
				outVarArray.push_back(groupString);
				substringRange.loc = lexIndex + 1;
			}
			break;
		}
		++lexIndex;
	} while (openGroupingCount > 0);
	Range	rangeToReplace = Range(inFuncNameRange.loc, lexIndex-inFuncNameRange.loc);
	return rangeToReplace;
}
string TrimWhitespace(const string & inBaseStr)	{
	Range		wholeRange(0, inBaseStr.size());
	//cout << "\t" << __PRETTY_FUNCTION__ << "- FINISHED\n";
	//return inBaseStr.substr(wholeRange.loc, wholeRange.len);
	
	Range		trimmedRange = wholeRange;
	size_t			tmpPos = inBaseStr.find_last_not_of(" \t\r\n");
	if (tmpPos != string::npos)
		trimmedRange.len = tmpPos+1;
	tmpPos = inBaseStr.find_first_not_of(" \t\r\n");
	if (tmpPos != string::npos)
		trimmedRange.loc = tmpPos;
	trimmedRange.len -= trimmedRange.loc;
	if (wholeRange == trimmedRange)
		return inBaseStr;
	return inBaseStr.substr(trimmedRange.loc, trimmedRange.len);
	
}
void FindAndReplaceInPlace(string & inSearch, string & inReplace, string & inBase)	{
	size_t		pos = 0;
	while ((pos=inBase.find(inSearch, pos)) != string::npos)	{
		inBase.replace(pos, inSearch.length(), inReplace);
		pos += inReplace.length();
	}
}
void FindAndReplaceInPlace(const char * inSearch, const char * inReplace, string & inBase)	{
	string		is(inSearch);
	string		ir(inReplace);
	FindAndReplaceInPlace(is, ir, inBase);
}


string FullPath(const string & inRelativePath)	{
#if _WIN32
	char		*outPath = _fullpath(NULL, inRelativePath.c_str(), 0);
	return std::string(outPath);
#else
	char		*outPath = realpath(inRelativePath.c_str(), NULL);
	return std::string(outPath);
#endif
}


}

