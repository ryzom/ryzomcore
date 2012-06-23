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

struct 
{
	int deltaLevel;
	float percentSuccess;
	int maxLevelBoost;
	float XPMultiplier;	
}
TableChanceMeleeHitByAdversaryLevel[]=
{
	{7,	95.0f,	 0,	2.1f},
	{6,	95.0f,	 0,	2.1f},	
	{5,	95.0f,	 0,	2.1f},	
	{4,	90.0f,	 0,	1.9f},	
	{3,	80.0f,	 0,	1.7f},	
	{2,	70.0f,	 0,	1.5f},	
	{1,	60.0f,	 1,	1.2f},	
	{0,	50.0f,	 0,	1.0f},	
	{-1,	40.0f,	 0,	0.8f},	
	{-2,	30.0f,	 0,	0.3f},	
	{-3,	20.0f,	 0,	0.1f},	
	{-4,	10.0f,	 0,	0.0f},	
	{-5,	5.0f,	 0,	0.0f}
};

struct 
{
	int deltaLevel;
	float percentSuccess;
	int maxLevelBoost;
	float XPMultiplier;	
}
TableChanceSpellHitByAdversaryLevel[]=
{
	{7,	95.0f, 	0,	2.1f},
	{6,	95.0f,	 0,	2.1f},	
	{5,	95.0f,	 0,	2.1f},	
	{4,	90.0f,	 0,	1.9f},	
	{3,	80.0f,	 0,	1.7f},	
	{2,	70.0f,	 0,	1.5f},	
	{1,	60.0f,	1,	1.2f},	
	{0,	50.0f,	 0,	1.0f},	
	{-1,	40.0f,	 0,	0.8f},	
	{-2,	30.0f,	 0,	0.3f},	
	{-3,	20.0f,	 0,	0.1f},	
	{-4,	10.0f,	 0,	0.0f},	
	{-5,	5.0f,	 0,	0.0f}
};

// note on size modifiers
// attacker is smaller than defender +1
// attacker is a bit bigger -2
// attacker is much bigger -4


struct
{
int quality;
int rateOfFire;
float damageFactor;
float secondaryDamageFactor; // damage factor when entity level > weapon quality
}
TableLightMeleeWeaponGenerics[]=
{
	{1,1,6.0f,2.5f},
	{2,1,3.75f,2.25f},
	{3,1,3.0f,2.25f},
	{4,1,2.25f,2.25f},
	{5,1,2.25f,2.25f},
	{6,1,2.25f,2.25f},
	{7,1,2.25f,2.25f},
	{8,1,3.0f,3.0f},
	{9,1,3.0f,3.0f},
	{10,1,3.0f,3.0f},
	{11,1,3.75f,3.75f},
	{12,1,3.75f,3.75f},
	{13,1,3.75f,3.75f},
	{14,1,4.5f,4.5f},
	{15,1,5.25f,5.25f},
	{16,1,6.0f,6.0f},
	{17,1,6.75f,6.75f},
	{18,1,7.5f,7.5f},
	{19,1,8.25f,8.25f},
	{20,1,9.75f,9.75f},
	{21,1,11.25f,11.25f},
},
TableMediumMeleeWeaponGenerics[]=
{
	{1,1,8.0f,3.7f},
	{2,1,5.0f,3.0f},
	{3,1,4.0f,3.0f},
	{4,1,3.0f,3.0f},
	{5,1,3.0f,3.0f},
	{6,1,3.0f,3.0f},
	{7,1,3.0f,3.0f},
	{8,1,4.0f,4.0f},
	{9,1,4.0f,4.0f},
	{10,1,4.0f,4.0f},
	{11,1,5.0f,5.0f},
	{12,1,5.0f,5.0f},
	{13,1,5.0f,5.0f},
	{14,1,6.0f,6.0f},
	{15,1,7.0f,7.0f},
	{16,1,8.0f,8.0f},
	{17,1,9.0f,9.0f},
	{18,1,10.0f,10.0f},
	{19,1,11.0f,11.0f},
	{20,1,13.0f,13.0f},
	{21,1,15.0f,15.0f},
},
TableHeavyMeleeWeaponGenerics[]=
{
	{1,1,17.92f,8.5f},
	{2,1,11.2f,7.0f},
	{3,1,8.96f,7.0f},
	{4,1,6.72f,6.72f},
	{5,1,6.72f,6.72f},
	{6,1,6.72f,6.72f},
	{7,1,6.72f,6.72f},
	{8,1,8.96f,8.96f},
	{9,1,8.96f,8.96f},
	{10,1,8.96f,8.96f},
	{11,1,11.2f,11.2f},
	{12,1,11.2f,11.2f},
	{13,1,11.2f,11.2f},
	{14,1,13.44f,13.44f},
	{15,1,15.68f,15.68f},
	{16,1,17.92f,17.92f},
	{17,1,20.16f,20.16f},
	{18,1,22.4f,22.4f},
	{19,1,24.64f,24.64f},
	{20,1,29.12f,29.12f},
	{21,1,33.6f,33.6f},
} ;

struct
{
int quality ;
int percentProtection;
int maxProtection ;
}
TableLightArmorGenerics[]=
{
	{1,20,2},
	{2,20,2},
	{3,20,2},
	{4,20,3},
	{5,20,3},
	{6,20,4},
	{7,20,5},
	{8,20,6},
	{9,20,7},
	{10,20,8},
	{11,20,10},
	{12,20,12},
	{13,20,14},
	{14,20,17},
	{15,20,21},
	{16,20,25},
	{17,20,30},
	{18,20,35},
	{19,20,43},
	{20,20,51},
	{21,20,61},
},
TableMediumArmorGenerics[]=
{
{1,35,3},
{2,35,3},
{3,35,4},
{4,35,5},
{5,35,6},
{6,35,7},
{7,35,8},
{8,35,10},
{9,35,12},
{10,35,14},
{11,35,17},
{12,35,21},
{13,35,25},
{14,35,30},
{15,35,36},
{16,35,43},
{17,35,52},
{18,35,62},
{19,35,75},
{20,35,89},
{21,35,107},
},
TableHeavyArmorGenerics[]=
{
{1,50,4},
{2,50,5},
{3,50,6},
{4,50,7},
{5,50,8},
{6,50,10},
{7,50,12},
{8,50,14},
{9,50,17},
{10,50,21},
{11,50,25},
{12,50,30},
{13,50,36},
{14,50,43},
{15,50,51},
{16,50,62},
{17,50,74},
{18,50,89},
{19,50,106},
{20,50,128},
{21,50,153},
};

