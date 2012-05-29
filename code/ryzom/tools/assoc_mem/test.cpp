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

#include "attribute.h"
#include "node.h"
#include "brain.h"
#include "tree.h"
#include "value.h"
#include "cond_node.h"
#include "result_node.h"

#include <iostream>
#include <valarray>
#include <math.h>

void test1()
{
	CBrain				brain(10,10,10,10,10);
	CTree				tree;

	CBoolField				viande("Viande");
	CBoolField				grand("Grand");
	CBoolField				bon("Bon");
	std::vector<std::string> vals_couleur;
	vals_couleur.push_back(std::string("Vert"));
	vals_couleur.push_back(std::string("Rouge"));
	CStringField			couleur(std::string("Couleur"), vals_couleur);

	brain.addField(&viande);
	brain.addField(&grand);
	brain.addField(&bon);
	brain.addField( (CField *) &couleur);

	tree.setKey(2);
	brain.addTree( &tree );

	CValue<bool>	val1(true);
	CValue<bool>	val2(false);
	CValue<std::string>	val3("Vert");
	CValue<std::string>	val4("Rouge");
	CValue<std::string>	val5("Orange");

	CRecord			r1;
	r1.addValue( &val1);
	r1.addValue( &val1);
	r1.addValue( &val1 );
	r1.addValue( &val4 );

	CRecord			r2;
	r2.addValue( &val2);
	r2.addValue( &val2);
	r2.addValue( &val2 );
	r2.addValue( &val4 );

	CRecord			r3;
	r3.addValue( &val1 );
	r3.addValue( &val1 );
	r3.addValue( &val1 );
	r3.addValue( &val3 );

	CRecord			r4;
	r4.addValue( &val2);
	r4.addValue( &val2);
	r4.addValue( &val2);
	r4.addValue( &val5);

	CRecord			r5;
	r5.addValue( &val2 );
	r5.addValue( &val2 );
	r5.addValue( &val2 );
	r5.addValue( &val3);

	CRecord			r6;
	r6.addValue( &val2 );
	r6.addValue( &val1 );
	r6.addValue( &val2 );
	r6.addValue( &val4 );

	CRecord			r7;
	r7.addValue( &val1 );
	r7.addValue( &val2 );
	r7.addValue( &val2 );
	r7.addValue( &val3 );

	CRecord			r8;
	r8.addValue( &val1 );
	r8.addValue( &val1 );
	r8.addValue( &val1 );
	r8.addValue( &val3 );

	CRecord			r9;
	r9.addValue( &val1 );
	r9.addValue( &val1 );
	r9.addValue( &val1 );
	r9.addValue( &val3 );

	CRecord			r10;
	r10.addValue( &val2 );
	r10.addValue( &val1 );
	r10.addValue( &val2 );
	r10.addValue( &val4 );

	brain.setInput(&r1);
	brain.setInput(&r2);
	brain.setInput(&r3);
	brain.setInput(&r4);
	brain.setInput(&r5);
	brain.setInput(&r6);
	brain.setInput(&r7);
	brain.setInput(&r8);
	brain.setInput(&r9);
	brain.setInput(&r10);

	brain.build();
	std::cout << brain.getDebugString();

	brain.setInput(&r7);
}	


void test2()
{
	std::string age_old("old");
	std::string age_mid("mid");
	std::string age_new("new");

	std::vector<std::string> vals_age;
	vals_age.push_back( age_old );
	vals_age.push_back( age_mid );
	vals_age.push_back( age_new );
	CStringField			age(std::string("AGE"), vals_age);

	CValue<bool> val_true(true);
	CValue<bool> val_false(false);


	CBoolField competition(std::string("Competition"));

	std::string type_swr("swr");
	std::string type_hwr("hwr");

	std::vector<std::string> vals_type;
	vals_type.push_back( type_swr );
	vals_type.push_back( type_hwr );
	CStringField			type(std::string("TYPE"), vals_type);

	CBoolField	profit("Profit");

	CBrain				brain(10,10,10,10,10);
	CTree				tree;

	brain.addField( &age );
	brain.addField( &competition );
	brain.addField( &type );
	brain.addField( &profit );

	tree.setKey( 3 );
	brain.addTree( &tree );

	CRecord r1;
	r1.addValue( age_old );
	r1.addValue( &val_true );
	r1.addValue( type_swr );
	r1.addValue( &val_false );
	brain.addRecord( &r1 );

	CRecord r2;
	r2.addValue( age_old );
	r2.addValue( &val_false );
	r2.addValue( type_swr );
	r2.addValue( &val_false );
	brain.addRecord( &r2 );

	CRecord r3;
	r3.addValue( age_old );
	r3.addValue( &val_false );
	r3.addValue( type_hwr );
	r3.addValue( &val_false );
	brain.addRecord( &r3 );

	CRecord r4;
	r4.addValue( age_mid );
	r4.addValue( &val_true );
	r4.addValue( type_swr );
	r4.addValue( &val_false );
	brain.addRecord( &r4 );

	CRecord r5;
	r5.addValue( age_mid );
	r5.addValue( &val_true );
	r5.addValue( type_hwr );
	r5.addValue( &val_false );
	brain.addRecord( &r5 );

	CRecord r6;
	r6.addValue( age_mid );
	r6.addValue( &val_false );
	r6.addValue( type_hwr );
	r6.addValue( &val_true );
	brain.addRecord( &r6 );

	CRecord r7;
	r7.addValue( age_mid );
	r7.addValue( &val_false );
	r7.addValue( type_swr );
	r7.addValue( &val_true );
	brain.addRecord( &r7 );

	CRecord r8;
	r8.addValue( age_new );
	r8.addValue( &val_true );
	r8.addValue( type_swr );
	r8.addValue( &val_true );
	brain.addRecord( &r8 );

	CRecord r9;
	r9.addValue( age_new );
	r9.addValue( &val_false );
	r9.addValue( type_hwr );
	r9.addValue( &val_true );
	brain.addRecord( &r9 );

	CRecord r10;
	r10.addValue( age_new );
	r10.addValue( &val_false );
	r10.addValue( type_swr );
	r10.addValue( &val_true );
	brain.addRecord( &r10 );

	brain.build();
	brain.setInput( &r5 );
}

int main(int, char *[])
{
	test2();
}	
