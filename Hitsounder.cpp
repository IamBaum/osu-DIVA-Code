#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <regex>

using namespace std;

string artist = "Artist";
string title = "Title";
string creator = "Creator";
string CurrentLine;
ofstream Create;
int HitsoundRotation[4] = {0, 0, 0, 0};
int Line_Counter = 0;

string GetPartialN(string line, int offset) 
{
	while (offset > 0) 
	{
		line = line.substr(line.find(",")+1);
		offset--;
	}
	line = line.substr(0, line.find(","));
	return line;
}

int GetObjectTypeN(int Type)
{
	/*
	Read the pos_x variable and determine the Object Type.
	0-63	- Key 0 - Triangle
	64-127	- Key 1 - Square
	128-191	- Key 2 - Cross
	191-255	- Key 3 - Circle
	256-319	- Key 4 - UP
	320-383	- Key 5 - Left
	384-447	- Key 6 - Down
	447-512	- Key 7 - Right
	*/
	
	int Column;

	if (Type < 64)
	{
		Column = 0;
	}
	else if (Type < 128)
	{
		Column = 3;
	}
	else if (Type < 192)
	{
		Column = 2;
	}
	else if (Type < 256)
	{
		Column = 1;
	}
	else if (Type < 320)
	{
		Column = 4;
	}
	else if (Type < 384)
	{
		Column = 7;
	}
	else if (Type < 448)
	{
		Column = 6;
	}
	else if (Type <= 512)
	{
		Column = 5;
	}
	return Column;
};

string GetHitsoundType (int ObjectType, int Hitsound)
{
/*
This will determine the HitsoundType based on the HitObject Code
endsyntax 1: = Normal Soundset.
endsyntax 2: = Soft Soundset.
endsyntax 3: = Drum Soundset.
hitsound 2: = Whistle.
hitsound 4: = Finish.
hitsound 8: = Clap.
-----------------------------
These Combinations can't be used as trigger.
hitsound 6: = Whistle/Finish
hitsound 10: = Clap/Whistle.
hitsound 12: = Clap/Finish.
Hitsound 14: Clap/Whistle/Finish.
*/
	string HitsoundType;

	if (ObjectType == 0)
	{
		switch(Hitsound)
		{
			case 0: HitsoundType = "2,1:0:0:0:"; break;
			case 1: HitsoundType = "4,1:0:0:0:"; break;
			case 2: HitsoundType = "8,3:0:0:0:"; break;
		}
	}
	else if(ObjectType == 1)
	{
		switch(Hitsound)
		{
			case 0: HitsoundType = "8,1:0:0:0:"; break;
			case 1: HitsoundType = "2,2:0:0:0:"; break;
			case 2: HitsoundType = "8,3:0:0:0:"; break;
		}
	}
	else if(ObjectType == 2)
	{
		switch(Hitsound)
		{
			case 0: HitsoundType = "4,2:0:0:0:"; break;
			case 1: HitsoundType = "8,2:0:0:0:"; break;
			case 2: HitsoundType = "8,3:0:0:0:"; break;
		}
	}
	else if(ObjectType == 3)
	{
		switch(Hitsound)
		{
			case 0: HitsoundType = "2,3:0:0:0:"; break;
			case 1: HitsoundType = "4,3:0:0:0:"; break;
			case 2: HitsoundType = "8,3:0:0:0:"; break;
		}
	}
	else if(ObjectType == 4)
	{
		switch(Hitsound)
		{
			case 0: HitsoundType = "2,1:0:0:0:"; break;
			case 1: HitsoundType = "4,1:0:0:0:"; break;
			case 2: HitsoundType = "8,3:0:0:0:"; break;
		}
	}
	else if(ObjectType == 5)
	{
		switch(Hitsound)
		{
			case 0: HitsoundType = "8,1:0:0:0:"; break;
			case 1: HitsoundType = "2,2:0:0:0:"; break;
			case 2: HitsoundType = "8,3:0:0:0:"; break;
		}
	}
	else if(ObjectType == 6)
	{
		switch(Hitsound)
		{
			case 0: HitsoundType = "4,2:0:0:0:"; break;
			case 1: HitsoundType = "8,2:0:0:0:"; break;
			case 2: HitsoundType = "8,3:0:0:0:"; break;
		}
	}
	else
	{
		switch(Hitsound)
		{
			case 0: HitsoundType = "2,3:0:0:0:"; break;
			case 1: HitsoundType = "4,3:0:0:0:"; break;
			case 2: HitsoundType = "8,3:0:0:0:"; break;
		}
	}

	return HitsoundType;
};

int GetHitsoundTypeLN1 (int ObjectType, int Hitsound)
{
	int HitsoundType;

	if (ObjectType == 0)
	{
		switch(Hitsound)
		{
			case 0: HitsoundType = 2; break;
			case 1: HitsoundType = 4; break;
			case 2: HitsoundType = 8; break;
		}
	}
	else if(ObjectType == 1)
	{
		switch(Hitsound)
		{
			case 0: HitsoundType = 8; break;
			case 1: HitsoundType = 2; break;
			case 2: HitsoundType = 8; break;
		}
	}
	else if(ObjectType == 2)
	{
		switch(Hitsound)
		{
			case 0: HitsoundType = 4; break;
			case 1: HitsoundType = 8; break;
			case 2: HitsoundType = 8; break;
		}
	}
	else if(ObjectType == 3)
	{
		switch(Hitsound)
		{
			case 0: HitsoundType = 2; break;
			case 1: HitsoundType = 4; break;
			case 2: HitsoundType = 8; break;
		}
	}
	else if(ObjectType == 4)
	{
		switch(Hitsound)
		{
			case 0: HitsoundType = 2; break;
			case 1: HitsoundType = 4; break;
			case 2: HitsoundType = 8; break;
		}
	}
	else if(ObjectType == 5)
	{
		switch(Hitsound)
		{
			case 0: HitsoundType = 8; break;
			case 1: HitsoundType = 2; break;
			case 2: HitsoundType = 8; break;
		}
	}
	else if(ObjectType == 6)
	{
		switch(Hitsound)
		{
			case 0: HitsoundType = 4; break;
			case 1: HitsoundType = 8; break;
			case 2: HitsoundType = 8; break;
		}
	}
	else if(ObjectType == 7)
	{
		switch(Hitsound)
		{
			case 0: HitsoundType = 2; break;
			case 1: HitsoundType = 4; break;
			case 2: HitsoundType = 8; break;
		}
	}
	

	return HitsoundType;
};

string GetHitsoundTypeLN2 (int ObjectType, int Hitsound)
{
	string HitsoundType;

	if (ObjectType == 0)
	{
		switch(Hitsound)
		{
			case 0: HitsoundType = "1:0:0:0:"; break;
			case 1: HitsoundType = "1:0:0:0:"; break;
			case 2: HitsoundType = "3:0:0:0:"; break;
		}
	}
	else if(ObjectType == 1)
	{
		switch(Hitsound)
		{
			case 0: HitsoundType = "1:0:0:0:"; break;
			case 1: HitsoundType = "2:0:0:0:"; break;
			case 2: HitsoundType = "3:0:0:0:"; break;
		}
	}
	else if(ObjectType == 2)
	{
		switch(Hitsound)
		{
			case 0: HitsoundType = "2:0:0:0:"; break;
			case 1: HitsoundType = "2:0:0:0:"; break;
			case 2: HitsoundType = "3:0:0:0:"; break;
		}
	}
	else if(ObjectType == 3)
	{
		switch(Hitsound)
		{
			case 0: HitsoundType = "3:0:0:0:"; break;
			case 1: HitsoundType = "3:0:0:0:"; break;
			case 2: HitsoundType = "3:0:0:0:"; break;
		}
	}
	else if(ObjectType == 4)
	{
		switch(Hitsound)
		{
			case 0: HitsoundType = "1:0:0:0:"; break;
			case 1: HitsoundType = "1:0:0:0:"; break;
			case 2: HitsoundType = "3:0:0:0:"; break;
		}
	}
	else if(ObjectType == 5)
	{
		switch(Hitsound)
		{
			case 0: HitsoundType = "1:0:0:0:"; break;
			case 1: HitsoundType = "2:0:0:0:"; break;
			case 2: HitsoundType = "3:0:0:0:"; break;
		}
	}
	else if(ObjectType == 6)
	{
		switch(Hitsound)
		{
			case 0: HitsoundType = "2:0:0:0:"; break;
			case 1: HitsoundType = "2:0:0:0:"; break;
			case 2: HitsoundType = "3:0:0:0:"; break;
		}
	}
	else if(ObjectType == 7)
	{
		switch(Hitsound)
		{
			case 0: HitsoundType = "3:0:0:0:"; break;
			case 1: HitsoundType = "3:0:0:0:"; break;
			case 2: HitsoundType = "3:0:0:0:"; break;
		}
	}
	

	return HitsoundType;
};

int GetLineCount()
{
	return Line_Counter;
}

// The Hitsounder will read all notes in a beatmap, and replaces their normal hitsounds, to a rotation between clap, whisper and Finish / Normal, Soft, Drums.
//The Reason is for the hitsound trigger determination.
int mainN(string FileName)
{
	cout<<"New .osu File Creation Finished."<<endl;

	ifstream Read("Notes.txt");
	ofstream Create(FileName);
	ofstream Hitsounds;
	Hitsounds.open(FileName);

	int Code_Type = 1;

	int Dummy;
	int ObjectType;
	int HitsoundTypeLN1;
	string HitsoundTypeLN2;
	string HitsoundType;
	bool osuDIVA = false;
	
	while(getline (Read, CurrentLine))
	{	
		if (Code_Type != 0)
		{
			if (CurrentLine.find("SkinPreference:") == 0)
			{
		
			}
			else if (CurrentLine.find("Version:") == 0)
			{
				size_t found (CurrentLine.find("osu!DIVA"));
				if (found!=std::string::npos)
					osuDIVA = true;		

				Hitsounds << CurrentLine << endl;
			}
			else if (CurrentLine.find("Tags:") == 0)
			{
				size_t found (CurrentLine.find("osu!DIVA"));
				if (found!=std::string::npos)
					osuDIVA = true;		

				if (osuDIVA == true)
				{
					Hitsounds << CurrentLine << endl;
				}
				else
				{
					Hitsounds << CurrentLine << " osu!DIVA" << endl;
				}
			}
			else if (CurrentLine.find("SpecialStyle:") == 0)
			{
				Hitsounds << "SkinPreference:osu!DIVA" << endl;
				Hitsounds << CurrentLine << endl;
			}
			else if (CurrentLine.find("WidescreenStoryboard:") == 0)
			{
				Hitsounds << "WidescreenStoryboard: 1" << endl;
			}
			else 
			{
				Hitsounds << CurrentLine << endl;
			}
		}
		if (Code_Type == 0)
		{
			int pos_x = std::stoi(GetPartialN(CurrentLine, 0));
			int pos_y = std::stoi(GetPartialN(CurrentLine, 1));
			int offset = std::stoi(GetPartialN(CurrentLine, 2));
			int type = std::stoi(GetPartialN(CurrentLine, 3));
			int hitsound = std::stoi(GetPartialN(CurrentLine, 4));
			int endsyntax = std::stoi(GetPartialN(CurrentLine, 5));

			/*
			type<6 = Auto hitsounding for Normal Notes.
			Type>=6 = Auto hitsounding for Long Notes.
			*/

			if (type < 6)
			{
				ObjectType = GetObjectTypeN(pos_x);

				if (ObjectType > 3)
				{
					ObjectType = ObjectType - 4;
				}
				HitsoundType = GetHitsoundType(ObjectType, HitsoundRotation[ObjectType]);
				Hitsounds << pos_x << ',' << pos_y << ',' << offset << ',' << type << ',' << HitsoundType << endl;

				HitsoundRotation[ObjectType]++;

				if (HitsoundRotation[ObjectType] == 3)
				{
					for (Dummy = 0; Dummy < 4; Dummy++)
					{
						if (HitsoundRotation[Dummy] >= 2)
							HitsoundRotation[Dummy] = 0;
					}
				}
			}
			else
			{
				ObjectType = GetObjectTypeN(pos_x);

				HitsoundTypeLN1 = GetHitsoundTypeLN1(ObjectType, HitsoundRotation[ObjectType]);
				HitsoundTypeLN2 = GetHitsoundTypeLN2(ObjectType, HitsoundRotation[ObjectType]);

				Hitsounds << pos_x << ',' << pos_y << ',' << offset << ',' << type << ',' << HitsoundTypeLN1 << ',' << endsyntax << ':' << HitsoundTypeLN2 << endl;

				HitsoundRotation[ObjectType]++;

				if (HitsoundRotation[ObjectType] == 3)
				{
					for (Dummy = 0; Dummy < 4; Dummy++)
					{
						if (HitsoundRotation[Dummy] >= 2)
							HitsoundRotation[Dummy] = 0;
					}
				}
			}
		}
		
		if (CurrentLine == "[HitObjects]")
		{
			Code_Type = 0;
		}
		Line_Counter++;
	}
	Read.close();
	Hitsounds.close();
	return Line_Counter;
}
