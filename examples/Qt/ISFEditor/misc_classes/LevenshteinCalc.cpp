#include "LevenshteinCalc.h"

#include <functional>




double CalculateLevenshtein(const QString & p_string1, const QString & p_string2)	{
	int l_string_length1 = p_string1.length();
	int l_string_length2 = p_string2.length();
	
	//	this crashes with large files, so mem is moved to the heap and we use blocks to access it
	//int d[l_string_length1+1][l_string_length2+1];
	int		*d = static_cast<int*>(malloc(sizeof(int) * ((l_string_length1+1) * (l_string_length2+1)) ) );
	std::function<int(int,int)>		GetVal = [&](int x, int y)	{
		return *(d + (y * (l_string_length1+1)) + (x));
	};
	std::function<void(int,int,int)>	SetVal = [&](int x, int y, int newVal)	{
		*(d + (y * (l_string_length1+1)) + (x)) = newVal;
	};
	

	int i;
	int j;
	int l_cost;

	for (i = 0;i <= l_string_length1;i++)
	{
		//d[i][0] = i;
		SetVal(i, 0, i);
	}
	for(j = 0; j<= l_string_length2; j++)
	{
		//d[0][j] = j;
		SetVal(0, j, j);
	}
	for (i = 1;i <= l_string_length1;i++)
	{
		for(j = 1; j<= l_string_length2; j++)
		{
			if( p_string1[i-1] == p_string2[j-1] )
			{
				l_cost = 0;
			}
			else
			{
				l_cost = 1;
			}
			//d[i][j] = std::min(
			//	d[i-1][j] + 1,					// delete
			//	std::min(d[i][j-1] + 1,			// insert
			//		d[i-1][j-1] + l_cost)			// substitution
			//);
			int		tmpVal = std::min(GetVal(i, j-1) + 1, GetVal(i-1, j-1) + l_cost);
			tmpVal = std::min(tmpVal, GetVal(i-1, j) + 1);
			SetVal(i, j, tmpVal);
			
			if( (i > 1) && 
				(j > 1) && 
				(p_string1[i-1] == p_string2[j-2]) && 
				(p_string1[i-2] == p_string2[j-1])) 
			{
				//d[i][j] = std::min(
				//	d[i][j],
				//	 d[i-2][j-2] + l_cost	// transposition
				//	);
				tmpVal = std::min(GetVal(i, j), GetVal(i-2, j-2) + l_cost);
				SetVal(i, j, tmpVal);
			}
		}
	}
	//int			tmpReturnMe = d[l_string_length1][l_string_length2];
	int			tmpReturnMe = GetVal(l_string_length1, l_string_length2);
	double		returnMe = double(tmpReturnMe)/double(l_string_length1+l_string_length2);
	return returnMe;
}



