 #include "record.h"

CRecord::CRecord()
{
}

CRecord::CRecord(std::vector<IValue *> &)
{
}

CRecord::~CRecord()
{
/*	std::vector<IValue *>::iterator it_val = _Values.begin();
	while ( it_val != _Values.end() )
	{
		IValue *val = *it_val;
		it_val++;
		delete val;
	}
*/	
}

void CRecord::addValue(IValue *value)
{
	_Values.push_back( value );
}

void CRecord::addValue(std::string &str)
{
	_Values.push_back( new CValue<std::string>(str) );
}

void CRecord::addValue(bool b)
{
	_Values.push_back( new CValue<bool>(b) );
}

void CRecord::addValue(int val)
{
	_Values.push_back( new CValue<int>(val) );
}

const std::vector<IValue *> &CRecord::getValues()
{
	return _Values;
}

const IValue *CRecord::operator[](int index)
{
	return _Values[index];
}

int CRecord::size()
{
	return _Values.size();
}