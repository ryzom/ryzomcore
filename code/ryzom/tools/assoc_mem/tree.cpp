// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <valarray>
#include <map>
#include <stdio.h>
#include <iostream>
#include <algorithm>

#include "tree.h"
#include "cond_node.h"
#include "result_node.h"

CTree::CTree()
{
	_RootNode = NULL;
}

CTree::~CTree()
{
	if ( _RootNode != NULL )
		delete _RootNode;
}

void CTree::setKey(int key)
{
	_Key = key;
}

int CTree::getKey() 
{
	return _Key;
}


bool CTree::getOutput(CRecord *input)
{
	if ( _RootNode != NULL )
		return _RootNode->propagRecord( input );
	else
		return false;
}

int CTree::getNbRecords(std::vector<CRecord *> &records,int key, IValue *value) //
{
	int nb = 0;
	std::vector<CRecord *>::iterator it_r = records.begin();
	while ( it_r != records.end() )
	{
		if (  *((**it_r)[key]) == value )
			nb++;
		it_r++;
	}
	return nb;
}

double CTree::log2(double val) const
{
	return (log(val) / log(2.0));
}

double CTree::entropy(double a, double b) const
{
	double p1; 
	double p2; 

	if ( a > 0 )
		p1 = a * log2(a);
	else
		p1 = 0;

	if ( b > 0 )
		p2 = b * log2(b);
	else
		p2 = 0;

	return ( p1 + p2 ) * -1;
}

double CTree::entropy(std::vector<double> &p) const
{
	double result = 0;
	std::vector<double>::iterator it_p = p.begin();
	while ( it_p != p.end() )
	{
		double val = *it_p;

		if ( val > 0 )
			result = result + val * log2( val );
		
		it_p++;
	}
	return result * -1;
}

double CTree::gain(std::vector<CRecord *> &records, int attrib, CField *field)
{
	int nb_values = (int)field->getPossibleValues().size();
	int nb_records = (int)records.size();

	CValue<bool> bool_true(true);

	double nb_key_true = getNbRecords(records, _Key, &bool_true );
	double nb_key_false = nb_records - nb_key_true;

	double entropy_records = entropy( nb_key_true / nb_records, nb_key_false / nb_records );

	double gain = entropy_records;

	int i;
	for ( i = 0; i < nb_values; i++ )
	{
		IValue *val = field->getPossibleValues()[i];

		int nb_records_val, nb_records_notval;
		splitRecords(records, attrib, val, nb_records_val, nb_records_notval );

		int nb_records_val_key, nb_records_val_notkey;
		splitRecords(records, attrib, val, true, nb_records_val_key, nb_records_val_notkey );

		double entropy_val = entropy( ((double)nb_records_val_key) / ((double)nb_records_val), ((double)nb_records_val_notkey) / ((double)nb_records_val) );

		gain = gain - ( ( (double)nb_records_val ) / ( (double) nb_records ) * entropy_val );
 	}	
	return gain;
}

std::vector<std::pair<double,int> > CTree::getSortedFields( std::vector<int> &attributes, std::vector<CRecord *> &records, std::vector<CField *> &fields )
{
	std::vector<std::pair<double,int> > attribs;

	if ( ! records.empty() )
	{
		std::vector<int>::iterator it_a = attributes.begin();
		while ( it_a != attributes.end() )
		{
			if ( (*it_a) != _Key )
				attribs.push_back( std::pair<double,int>( gain(records, (*it_a), fields[*it_a] ) , (*it_a) ) );
			it_a++;
		}
	}

	// Sorts the records by gain
	std::sort(attribs.begin(), attribs.end(), greater() );

	std::vector<std::pair<double,int> >::iterator it_f = attribs.begin();

	std::cout << "Attributes(gain) :" << std::endl;
	while ( it_f != attribs.end() )
	{
		std::cout <<  "  "  << fields[ (*it_f).second ]->getName() << " (" << (*it_f).first << ") " << std::endl;
		it_f++;
	}
	std::cout << std::endl;

	return attribs;
}

// Looks for the attrib with the most gain
int CTree::getBestAttrib( std::vector<int> &attributes, std::vector<CRecord *> &records,  std::vector<CField *> &fields )
{
	double	tmp_gain;
	double	max_gain = 0;
	int		best_attrib = -1;

	std::cout << "Attributes(gain) :" << std::endl;
	if ( ! records.empty() )
	{
		std::vector<int>::iterator it_a = attributes.begin();
		while ( it_a != attributes.end() )
		{
			if ( (*it_a) != _Key )
			{

				tmp_gain = gain( records, *it_a, fields[ *it_a ] );
				std::cout <<  "  "  << fields[ *it_a ]->getName() << " (" << tmp_gain << ") " << std::endl;
				if ( tmp_gain >= max_gain )
				{
					max_gain = tmp_gain;
					best_attrib = *it_a;
				}
			}	
			it_a++;
		}
	}
	return best_attrib;
}


void CTree::rebuild(std::vector<CRecord *> &records, std::vector<CField *> &fields)
{
	std::vector<int> left_fields;
	CRecord *first = *records.begin();

	for (int i = 0; i < first->size(); i++ )
		if ( i != _Key )
			left_fields.push_back( i );
	
	_RootNode = ID3( left_fields, records, fields );
}

float CTree::findNumKeyValue(std::vector<CRecord *> &records, int key)
{
	float sum_true = 0;
	float nb_true = 0;
	float sum_false = 0;
	float nb_false = 0;

	std::vector<CRecord *>::iterator it_r = records.begin();
	while ( it_r != records.end() )
	{
		bool result = ((CValue<bool> *)(**it_r)[ _Key ])->getValue();
		if ( result == true )
		{
			sum_true += ((CValue<int> *)(**it_r)[ key ])->getValue();
			nb_true ++;
		}
		else
		{
			sum_false += ((CValue<int> *)(**it_r)[ key ])->getValue();
			nb_false ++;
		}
		it_r++;
	}

	return ( sum_true / nb_true + sum_false / nb_false ) / 2;
}

std::string CTree::getDebugString(std::vector<CRecord *> &records, std::vector<CField *> &fields)
{
	std::string output;
	output += "CTree KEY = ";
	output += fields[ _Key ]->getName();
	return output;
}

INode *CTree::ID3(std::vector<int> &attributes, std::vector<CRecord *> &records, std::vector<CField *> &fields)
{
	if ( records.empty() )
	{
		return new CResultNode( false );
	}
	else
	{
		// If there is no attribute left and the records don't have the same key value,
		// returns a result node with the most frequent key value
		if ( attributes.empty() )
		{
			int nb_key_true; 
			int nb_key_false;  
			splitRecords( records, _Key , nb_key_true, nb_key_false );

			if ( nb_key_true > nb_key_false )
				return new CResultNode( true );
			else
				return new CResultNode( false );
		}


		// Tests if all records have the same key value, if so returns a result node with this key value.
		int nb_records = (int)records.size();		
		int nb_key_true; 
		int nb_key_false;

		splitRecords( records, _Key , nb_key_true, nb_key_false );

		if (  nb_key_true == nb_records )
			return new CResultNode( true );

		if ( nb_key_false == nb_records )
			return new CResultNode( false );


		// Gets the attribute with the most gain for the current record set,
		// and recursively builds the subnodes corresponding to each 
		// possible value for this attribute.
		int best_gain_attrib = getBestAttrib( attributes,  records, fields );

		std::vector< std::vector<CRecord *> > sorted_records;
		splitRecords( records, best_gain_attrib, fields, sorted_records );		// classifies the records depending on the value of the best gain attribute

		std::vector<int> new_attribs;
		for ( int i = 0; i < (int) attributes.size(); i++ )	// Creates a new attributes list from the current attributes list with the best gain attribute removed
			if ( attributes[i] != best_gain_attrib )
				new_attribs.push_back( attributes[i] );

		ICondNode *root_node = fields[best_gain_attrib]->createNode(_Key, best_gain_attrib, records);

		std::vector< std::vector<CRecord *> >::iterator it = sorted_records.begin();	// Constructs subnodes recursively
		while ( it != sorted_records.end() )
		{
			root_node->addNode( ID3( new_attribs, *it, fields ) );
			it++;
		}
		return root_node;
	}
}

std::vector<CRecord *> CTree::getRecords(std::vector<CRecord *> &records, int attrib, bool value)
{
	std::vector<CRecord *> result;
	std::vector<CRecord *>::iterator it_r = records.begin();
	while ( it_r != records.end() )
	{
		if (  ((CValue<bool> *)(**it_r)[attrib])->getValue() == value )
			result.push_back( *it_r );
		it_r++;
	}
	return result;
}

void CTree::splitRecords(std::vector<CRecord *> &records, int attrib, int &true_records, int &false_records) //
{
	true_records = 0;
	false_records = 0;

	std::vector<CRecord *>::iterator it_r = records.begin();
	while ( it_r != records.end() )
	{
		if (  ((CValue<bool> *)(**it_r)[attrib])->getValue() == true )
			true_records++;
		else
			false_records++;
		it_r++;
	}
}

void CTree::splitRecords(std::vector<CRecord *> &records, int attrib, IValue *val, int &true_records, int &false_records) //
{
	true_records = 0;
	false_records = 0;

	std::vector<CRecord *>::iterator it_r = records.begin();
	while ( it_r != records.end() )
	{
		const IValue *left_val = (**it_r)[ attrib ];
		if (  ( *left_val ) == val )
			true_records++;
		else
			false_records++;
		it_r++;
	}
}

// count records with a certain value for an attrib and true or false for the key attrib
void CTree::splitRecords(std::vector<CRecord *> &records, int attrib, IValue *val, bool key, int &true_records, int &false_records) //
{
	true_records = 0;
	false_records = 0;

	std::vector<CRecord *>::iterator it_r = records.begin();
	while ( it_r != records.end() )
	{
		if (  (* ( (**it_r)[attrib] ) ) == val )
		{
			if ( ( (CValue<bool> *) (**it_r)[ _Key ] )->getValue() == key )
				true_records++;
			else
				false_records++;
		}
		it_r++;
	}
}


// Sorts records according to the possibles values for an attribute.
void CTree::splitRecords( std::vector<CRecord *> &records, int attrib, std::vector<CField *> &fields, std::vector< std::vector<CRecord *> > &result) //
{
	if ( result.size() < fields[attrib]->getPossibleValues().size() )
	{
		int nb_missing = (int)(fields[attrib]->getPossibleValues().size() - result.size());
		for (int i = 0; i  <=  nb_missing; i++ )
		{
			result.push_back( std::vector<CRecord *>() );
		}
	}

	std::vector<CRecord *>::iterator it_r = records.begin();
	while ( it_r != records.end() )
	{
		std::vector<IValue *>::const_iterator it_vp = fields[attrib]->getPossibleValues().begin();
		std::vector< std::vector<CRecord *> >::iterator it = result.begin();
		int id_val = 0;
		while ( it_vp != fields[attrib]->getPossibleValues().end() )
		{
			const IValue *left_value = (**it_r)[attrib];
			IValue *right_value = *it_vp;
			if (  (*left_value) == right_value )
				(*it).push_back( *it_r );
			it_vp++;
			it++;
		}
		it_r++;
	}
}
