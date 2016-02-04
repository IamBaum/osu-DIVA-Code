#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <random>
#include <time.h>
#include <math.h>
#include <regex>
#include <cmath>
#include <Shobjidl.h>

using namespace std;

int mainN(string FileNameOsu);

ofstream storyboard;
ofstream create;
string Artist = "Artist";
string Title = "Title";
string Creator = "Creator";
string Version = "Version";
string HitsoundType;
string HitsoundType2;
string previousLine;
string currentLine = "0";
int Position_Y = 0;
int Position_X = 0;
int Pattern_Direction;
int Pattern_Direction_old = 9;
int SnapDistance = 4;
double SnapDistance_Value;
int offset_old = 0;
int last_offset = 0;
int Position_LR = 0;

// Timing Section segments

int KiaiTimeNotes = 0;
double BPMT;
int SVCounter = 1;
int KiaiCounter = 0;
int BPMs[50];
int BPMOffsets[50];
double SVChange[50];
int SVOffsets[50];
int SV_Kiai[50];
int SV_Kiai2[50];
int SV_KiaiOffsets[50];
int KiaiNoteAmount[50];
int SV_Kiai_Notes[1000];
double SV_Value = 1;

// Song Setup
int Difficulty_Position;
string Difficulty[7] = {"Easy","Normal","Hard","Insane","Extreme","Exhaust","Undefined"};
int Diff = 0;
int OD;
int TimingOD[11][5] = {	{151,127,97,64,16},
						{148,124,94,61,16},
						{145,121,91,58,16},
						{142,118,88,55,16},
						{139,115,85,52,16},
						{136,112,82,49,16},
						{133,109,79,46,16},
						{130,106,76,43,16},
						{127,103,73,40,16},
						{124,100,70,37,16},
						{121,97,67,34,16}};


string GetPartial(string line, int offset) 
{
	while (offset > 0) 
	{
		line = line.substr(line.find(",")+1);
		offset--;
	}
	line = line.substr(0, line.find(","));
	return line;
}

int GetObjectType(int Type)
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

int CheckForBorders(int Pos_Y, int Pos_X, int Pat_Direction)
{
	/* 
	CheckForBorders will check, if The next Object would be placed too close to the Borders.
	Largest Border that will be allowed:
	ChangeDirection = 0: No Change.
	ChangeDirection = 1: Change.
	Position_X = 40 - 600.
	Position_Y = 100 - 400.
	*/

	int ChangeDirection = 0;
	
	switch (Pat_Direction)
	{
		case 0: Pos_Y = Pos_Y + floor(SnapDistance_Value); break;
		case 1: Pos_Y = Pos_Y + floor(SnapDistance_Value / 1.5); Pos_X = Pos_X + floor(SnapDistance_Value / 1.5); break;
		case 2: Pos_X = Pos_X + floor(SnapDistance_Value); break;
		case 3: Pos_X = Pos_X + floor(SnapDistance_Value / 1.5); Pos_Y = Pos_Y - floor(SnapDistance_Value / 1.5); break;
		case 4: Pos_Y = Pos_Y - floor(SnapDistance_Value); break;
		case 5: Pos_Y = Pos_Y - floor(SnapDistance_Value / 1.5); Pos_X = Pos_X - floor(SnapDistance_Value / 1.5); break;
		case 6: Pos_X = Pos_X - floor(SnapDistance_Value); break;
		case 7: Pos_X = Pos_X - floor(SnapDistance_Value / 1.5); Pos_Y = Pos_Y + floor(SnapDistance_Value / 1.5); break;
	}

	if (Pos_Y < 100 || Pos_Y > 400 || Pos_X < 40 || Pos_X > 600)
	{
		ChangeDirection = 1;
	}
	return ChangeDirection;
};

int CheckForInvertedDirection(int Pat_Direction_old)
{
	/*
	This Function will check the pattern direction, where it came from.
	*/
	int Inversed_Direction;

	switch (Pattern_Direction)
	{
	case 0: Inversed_Direction = 4; break;
	case 1: Inversed_Direction = 5; break;
	case 2: Inversed_Direction = 6; break;
	case 3: Inversed_Direction = 7; break;
	case 4: Inversed_Direction = 0; break;
	case 5: Inversed_Direction = 1; break;
	case 6: Inversed_Direction = 2; break;
	case 7: Inversed_Direction = 3; break;
	}
	return Inversed_Direction;
};

bool CheckForPatternDistance(int Pos_X, int old_Pos_X, int Pos_Y, int old_Pos_Y)
{

//This will Prevent that new patterns will be placed too close, to the previous one.


	int Distance_X;
	int Distance_Y;

	if (Pos_X > old_Pos_X)
	{
		Distance_X = Pos_X - old_Pos_X;
	}
	else
	{
		Distance_X = old_Pos_X - Pos_X;
	}

	if (Pos_Y > old_Pos_Y)
	{
		Distance_Y = Pos_Y - old_Pos_Y;
	}
	else
	{
		Distance_Y = old_Pos_Y - Pos_Y;
	}

	if (Distance_X < 100 || Distance_Y < 75)
	{
		return false;
	}
	else
	{
		return true;
	}

};

int GetSnapDistance(int Offset, int Offset_old)
{
	/*
	The SnapDistance will determine the distance between the Objects in a pattern, based on the amount of "bars" in 1/4 snap Divisor.
	The + 10 is there, to prevent wrong snappings to result in wrong Values.
	*/
	int Snap_Distance;
	int Difference = Offset - Offset_old;

	if (Difference < (BPMT / 4) + 10)
	{
		Snap_Distance = 1;
	}
	else if (Difference < (BPMT / 2) + 10)
	{
		Snap_Distance = 2;
	}
	else if (Difference < ((BPMT / 4) * 3) + 10)
	{
		Snap_Distance = 3;
	}
	else if (Difference < BPMT + 10)
	{
		Snap_Distance = 4;
	}
	else
	{
		Snap_Distance = 8;
	}
	return Snap_Distance;
};

int GetSnapDistance_Value(int offset, int offset_old)
{
	/*
	The SnapDistance will determine the distance between the Objects in a pattern, based on the amount of time between the 2 notes.
	*/
	double Snap_Distance_Value;
	int Difference = offset - offset_old;

	Snap_Distance_Value = Difference / 4; 
	if (Snap_Distance_Value > 175)
		Snap_Distance_Value = 175;

	if (Snap_Distance_Value < 0)
		Snap_Distance_Value = 0;

	return Snap_Distance_Value;
};

void GetHitsoundType(int hitsound, int endsyntax)
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
	if (endsyntax == 1)
	{
		switch(hitsound)
		{
			case 2: HitsoundType = " T,HitSoundNormalWhistle,"; break;
			case 4: HitsoundType = " T,HitSoundNormalFinish,"; break;
			case 8: HitsoundType = " T,HitSoundNormalClap,"; break;
		}
	}
	else if (endsyntax == 2)
	{
		switch(hitsound)
		{
			case 2: HitsoundType = " T,HitSoundSoftWhistle,"; break;
			case 4: HitsoundType = " T,HitSoundSoftFinish,"; break;
			case 8: HitsoundType = " T,HitSoundSoftClap,"; break;
		}
	}
	else if (endsyntax == 3)
	{
		switch(hitsound)
		{
			case 2: HitsoundType = " T,HitSoundDrumWhistle,"; break;
			case 4: HitsoundType = " T,HitSoundDrumFinish,"; break;
			case 8: HitsoundType = " T,HitSoundDrumClap,"; break;
		}
	}
};

void GenerateKiaiNumbers(int Number, int Position, int Offset_Old, int Offset)
{
	switch(Number)
	{
		case 0: storyboard << "Sprite,Background,Centre," << '"' << "SB/Number_0.png" << '"' << ",320,240" << endl; break;
		case 1: storyboard << "Sprite,Background,Centre," << '"' << "SB/Number_1.png" << '"' << ",320,240" << endl; break;
		case 2: storyboard << "Sprite,Background,Centre," << '"' << "SB/Number_2.png" << '"' << ",320,240" << endl; break;
		case 3: storyboard << "Sprite,Background,Centre," << '"' << "SB/Number_3.png" << '"' << ",320,240" << endl; break;
		case 4: storyboard << "Sprite,Background,Centre," << '"' << "SB/Number_4.png" << '"' << ",320,240" << endl; break;
		case 5: storyboard << "Sprite,Background,Centre," << '"' << "SB/Number_5.png" << '"' << ",320,240" << endl; break;
		case 6: storyboard << "Sprite,Background,Centre," << '"' << "SB/Number_6.png" << '"' << ",320,240" << endl; break;
		case 7: storyboard << "Sprite,Background,Centre," << '"' << "SB/Number_7.png" << '"' << ",320,240" << endl; break;
		case 8: storyboard << "Sprite,Background,Centre," << '"' << "SB/Number_8.png" << '"' << ",320,240" << endl; break;
		case 9: storyboard << "Sprite,Background,Centre," << '"' << "SB/Number_9.png" << '"' << ",320,240" << endl; break;
	}
	switch(Position)
	{
		case 0: storyboard << " M,0," << Offset_Old << ',' <<  Offset << ",35,102" << endl;  break;
		case 1:	storyboard << " M,0," << Offset_Old << ',' <<  Offset << ",55,102" << endl; break;
		case 2:	storyboard << " M,0," << Offset_Old << ',' <<  Offset << ",75,102" << endl; break;
	}
	storyboard << "  S,0," << Offset_Old << ",," << "0.825" << endl;
};

void DetermineNumbers(int Number, int Offset_Old, int Offset)
{
	int Number1;
	int Number10;
	int Number100;

	if (Number > 99)
	{
		Number100 = Number / 100;
		Number10 = Number / 10;
		while(Number10 > 9)
		{
			Number10 = Number10 - 10;
		}
		Number1 = (Number - (Number100 * 100)) - (Number10 * 10);
	}
	else if (Number > 9)
	{
		Number100 = 0;
		Number10 = Number / 10;
		Number1 = Number - (Number10 * 10);
	}
	else
	{
		Number100 = 0;
		Number10 = 0;
		Number1 = Number; 
	}
	 
	if (Number100 != 0)
	GenerateKiaiNumbers(Number100,0,Offset_Old,Offset);

	if (Number10 != 0 || Number100 != 0)
	GenerateKiaiNumbers(Number10,1,Offset_Old,Offset);

	GenerateKiaiNumbers(Number1,2,Offset_Old,Offset);
};

string GetArtist()
{
	return Artist;
};

string GetTitle()
{
	return Title;
};

string GetCreator()
{
	return Creator;
};


int main ()
{
	int Line_Count;
	int Line = 0;
	srand(time(0));
	ifstream ChordCheck("Notes.txt");
	ifstream Positions("Positions.txt");
	
	/*
	Position Y and X determine the position of the Objects on the screen.
	SPosition Y and X determine the Start Position of the appreaoching Objects.
	Position LR determines, if the approaching Objects will start slightly left or right from for target Object.
	Position AD determines, if the approaching Object will start at the top or Bottom side of the screen.
	Random Curve will be used, to create a random approach way, so that the approaching Object won't move straight to the target.
	ObjectType will be used, to determine which Image will be used for each storyboard 
	Pattern_Direction determines the directions, in which the next note of the same object type will be placed.
	Random_Direction_Change will be randomized after each object, and might change the direction of the patterns at a random time.
	Inverted_Direction = The direction, where the pattern previously came from.
	Dummy will be used as a counter for loops.
	SOffset will be used, for the position changes in the Curve calculation of approaching Objects.
	*/
	int Chords[1000];
	int ChordCounter = 0;
	int ChordTimer = 0;
	int Percent = 1;
	int Old_Position_X;
	int Old_Position_Y;
	int SPosition_Y;
	int SPosition_X;
	int Position_X_old;
	int ObjectType = 8;
	int ObjectType_old;
	int ObjectType_Old;
	int Random_Direction_Change;
	int ChangeDirection = 0;
	int Inverted_Direction;
	int Code_Type = 2;
	int FirstOffset = -10000;
	int Dummy;
	int SOffset;
	int Determine_Method = 1;
	bool Correct_Pattern_Distance;
	bool Valid_Menu_Input = false;
	bool Valid_Option_Input = false;
	bool False_Arrows = false;
	bool isArrow = false;
	bool Check_Arrows = true;
	int Option_Menu = 1;
	int SBPositions_Offset[10000];
	int SBPositions_X[10000];
	int SBPositions_Y[10000];
	int Note_Counter = 0;
	double Background_Dim = 0;

	for (Dummy = 0; Dummy < 10000; Dummy++)
	{
		SBPositions_Offset[Dummy] = -1000;
		SBPositions_X[Dummy] = -1000;
		SBPositions_Y[Dummy] = -1000;
	}


// Timing Section code segments.
	int OffsetT;
	int Part3T;
	int Part4T;
	int Part5T;
	int Part6T;
	int InheritedT;
	int KiaiT;
	int BPMCounter = 0;

	for (Dummy = 0; Dummy < 50; Dummy++)
	{
		BPMs[Dummy] = 0;
		BPMOffsets[Dummy] = 0;
		SVChange[Dummy] = -100;
		SVOffsets[Dummy] = 0;
		SV_Kiai[Dummy] = 0;
		SV_Kiai2[Dummy] = 0;
		SV_KiaiOffsets[Dummy] = 0;
		KiaiNoteAmount[Dummy] = 0;
	}
	for (Dummy = 0; Dummy < 1000; Dummy++)
	{
		SV_Kiai_Notes[Dummy] = 0;
		Chords[Dummy] = 0;
	}

	int OffsetDifference = 100;
	int old_offset = -1000;
	ChordCounter = 0;

	while(Valid_Menu_Input == false)
	{
		cout<<"-------------------------------------------------"<<endl;
		cout<<"Menu: How do you want to create your Storyboard?"<<endl;
		cout<<"-------------------------------------------------"<<endl;
		cout<<"1. Automatic generate the Storyboard."<<endl;
		cout<<"2. Use your own Positions."<<endl;
		cout<<"3. Options."<<endl;

		cin>>Option_Menu;
		
		while(Option_Menu < 1 || Option_Menu > 3)
		{
			cout<<"Wrong Input!"<<endl;
			cin>>Option_Menu;
		}

		if (Option_Menu == 1)
		{
			Valid_Menu_Input = true;
			Determine_Method = 1;
		}
		else if (Option_Menu == 2)
		{
			Valid_Menu_Input = true;
			Determine_Method = 2;
		}
		else if(Option_Menu == 3)
		{
			Valid_Option_Input = true;
		}

		while (Valid_Option_Input == true)
		{
			cout<<"-------------------------------------------------"<<endl;
			cout<<"                 Option Menu:"<<endl;
			cout<<"-------------------------------------------------"<<endl;
			cout<<"1. Background Dim."<<endl;
			cout<<"2. Deactivate the Arrow Check."<<endl;
			cout<<"3. Back to the main Menu."<<endl;

			cin>>Option_Menu;
			while (Option_Menu < 1 || Option_Menu > 3)
			{
				cout<<"Invalid Input!"<<endl;
				cin>>Option_Menu;
			}

			if (Option_Menu == 1)
			{
				cout<<"Choose a Value between 0 and 100 (0% Dim is the base Value)."<<endl;
				cin>>Background_Dim;
				while (Background_Dim < 0 || Background_Dim > 100)
				{
					cout<<"Invalid Input!"<<endl;
					cin>>Background_Dim;
				}
			}
			else if(Option_Menu == 2)
			{
				if (Check_Arrows == true)
				{
					Check_Arrows = false;
					cout<<"The Arrow Check has been disabled."<<endl;
				}
				else
				{
					Check_Arrows = true;
					cout<<"The Arrow Check has been enabled."<<endl;
				}
				
			}
			else if(Option_Menu == 3)
			{
				Valid_Option_Input = false;
			}
		}
	}
	Background_Dim = floor(Background_Dim);
	Background_Dim = Background_Dim / 100;

	Dummy = 0;

	//------------ This will load all Storyboard Positions from the Positions.txt File ---------
	if (Determine_Method == 2)
	{
		while(getline (Positions,currentLine))
		{
			if (Code_Type == 0)
			{
				int pos_x = std::stoi(GetPartial(currentLine, 0));
				int pos_y = std::stoi(GetPartial(currentLine, 1));
				int offset = std::stoi(GetPartial(currentLine, 2));

				SBPositions_Offset[Dummy] = offset;
				SBPositions_X[Dummy] = pos_x;
				SBPositions_Y[Dummy] = pos_y;
				Dummy++;
			}
			if(currentLine.find("[HitObjects]") == 0)
			{
				Code_Type = 0;
			}
		}
	}

	Code_Type = 2;
	Dummy = 0;

// ---------------- Storyboard Preperations --------------------
	while(getline (ChordCheck, currentLine))
	{
		/*
		 *	Code_Type = 0: Timing Section.
		 *	Code_Type = 1: Hit Objects.
		 *	Code_Type = 2: MetaData
	  	 */

		if (Code_Type == 0)
		{
			std::regex regExTimingSection("-?[0-9]+,-?[0-9]+(.[0-9]+)?,[0-9],([0-9]),[0-9],[0-9]+,(0|1),[0-9]");

			if (std::regex_search(currentLine, regExTimingSection)) 
			{
				/*
				Dividing the Line Format, into it's components.
				Part5T = CustomSampleset.
				*/
				OffsetT = std::stoi(GetPartial(currentLine, 0));
				BPMT = std::stoi(GetPartial(currentLine, 1));
				Part3T = std::stoi(GetPartial(currentLine, 2));
				Part4T = std::stoi(GetPartial(currentLine, 3));
				Part5T = std::stoi(GetPartial(currentLine, 4));
				Part6T = std::stoi(GetPartial(currentLine, 5));
				InheritedT = std::stoi(GetPartial(currentLine, 6));
				KiaiT = std::stoi(GetPartial(currentLine, 7));
			}
			/*
			BPMT > 0 = Timing Point (BPM)
			BPMT < 0 = Inherited Timing Point (SV)
			*/
			if (BPMT > 0)
			{
				BPMOffsets[BPMCounter] = OffsetT;
				BPMs[BPMCounter] = BPMT;
				BPMCounter++;
			}
			else if (BPMT < 0)
			{
				if (KiaiT == 1)
				{
					SV_Kiai[SVCounter] = 1;
					SV_Kiai2[SVCounter] = 1;
				}
				SVChange[SVCounter] = BPMT;
				SVOffsets[SVCounter] = OffsetT;
				SV_KiaiOffsets[SVCounter] = OffsetT;
				SVCounter++;
			}
			BPMT = 0;
			KiaiT = 0;
		}

		if (Code_Type == 2)
		{
			if (currentLine.find("Artist:") == 0)
			{
				Artist = currentLine.substr(currentLine.find(":")+1);
			}
			if (currentLine.find("Title:") == 0)
			{
				Title = currentLine.substr(currentLine.find(":")+1);
			}
			if (currentLine.find("Creator:") == 0)
			{
				Creator = currentLine.substr(currentLine.find(":")+1);
			}
			if (currentLine.find("Version:") == 0)
			{
				Version = currentLine.substr(currentLine.find(":")+1);
			}

			if (currentLine.find("OverallDifficulty") == 0) 
			{
				OD = std::stoi(currentLine.substr(currentLine.find(":")+1));
			}
			// This will check the Difficulty, it will be set to Normal, if no valid Name is found.
			if (currentLine.find("Version:") == 0)
			{
				for (Dummy = 0; Dummy < 7; Dummy++)
				{
					std::size_t Difficulty_Position = currentLine.find(Difficulty[Diff]);
					if (Difficulty_Position < 200)
					{
						Dummy = 7;
					}
					else
					{
						Diff = Dummy;
					}
				}
			}
		}

		if (Code_Type == 1)
		{
			int pos_x = std::stoi(GetPartial(currentLine, 0));
			int pos_y = std::stoi(GetPartial(currentLine, 1));
			int offset = std::stoi(GetPartial(currentLine, 2));
			int type = std::stoi(GetPartial(currentLine, 3));
			int hitsound = std::stoi(GetPartial(currentLine, 4));
			int endsyntax = std::stoi(GetPartial(currentLine, 5));

			ObjectType = GetObjectType(pos_x);
			
			if (FirstOffset == -10000)
			{
				FirstOffset = offset - 3000;
				if (FirstOffset > 1000)
					FirstOffset = 0;
			}
			// The Offset Difference will check, if there is a Chord here.
			OffsetDifference = offset - old_offset;

			if (OffsetDifference < 5)
			{
				Chords[ChordCounter] = offset;
				ChordCounter++;
			}

			// The KiaiNoteAmount will count the amount of Notes, for each Kiai Time Zone.
			if (offset > SV_KiaiOffsets[Dummy + 1] && Dummy < 50)
				Dummy++;


			if (SV_Kiai2[Dummy] == 1)
			{
				if (offset >= SV_KiaiOffsets[Dummy] && offset <= SV_KiaiOffsets[Dummy + 1] && ObjectType < 4)
				{
					KiaiNoteAmount[Dummy]++;
				}
			}

			//Detection of Wrong Arrow Combination.
			if (Check_Arrows == true)
			{
				if (offset == old_offset)
				{
					if (ObjectType - 4 != ObjectType_Old)
					{
						if (ObjectType + 4 != ObjectType_Old)
						{
							cout<<"Wrong Arrow Placement at Offset: " << offset <<"."<<endl;
							False_Arrows = true;
						}
					}
				}

				if (ObjectType <= 3 && isArrow == true && offset != old_offset)
				{
					cout<<"Wrong Arrow Placement at Offset: " << old_offset <<"."<<endl;
					False_Arrows = true;
				}
				
				if (ObjectType > 3)
				{
					if (offset == old_offset)
					{
						isArrow = false;
					}
					else
					{
						isArrow = true;
					}
				}
				else
				{
					isArrow = false;
				}
			}

			//This type will check, if the note is a LN or normal note. If it's a LN, then the LN Ending will be declared as the last offset timing.

			if (type < 6)
			{
				offset_old = offset;
			}
			else
			{
				offset_old = endsyntax;
			}
			old_offset = offset;
			ObjectType_Old = ObjectType;
		}
			
		if(currentLine.find("[HitObjects]") == 0)
		{
			for (Dummy = 0; Dummy < 49; Dummy++)
			{
				if (SV_Kiai2[Dummy] == 1 && SV_Kiai2[Dummy + 1] == 1)
				{
					for(int Dummy2 = Dummy + 1; Dummy2 < 49; Dummy2++)
					{
						SV_Kiai2[Dummy2] = SV_Kiai2[Dummy2 + 1];
						SV_KiaiOffsets[Dummy2] = SV_KiaiOffsets[Dummy2 + 1];
					}
					Dummy--;
				}
			}
			Code_Type = 1;
			BPMCounter = 0;
			SVCounter = 0;
			Dummy = 0;
		}

		if (currentLine == "[TimingPoints]")
		{
			Code_Type = 0;
		}
	}

	if (False_Arrows == true)
	{
		cout<<"Recheck your Beatmap before proceeding, take a look into the Mapping Tutorial, if you aren't sure what is wrong."<<endl;
		getch();
		abort();
	}


// ----------------- Overlay Storyboard Creation. -------------------------------

	string FileNameOsu = Artist + " - " + Title + " (" + Creator + ") [" + Version + "].osu";
	string FileNameOsb = Artist + " - " + Title + " (" + Creator + ").osb";

	Line_Count = mainN(FileNameOsu);
	ifstream Read(FileNameOsu);

	ofstream create(FileNameOsb);
	storyboard.open(FileNameOsb);

	// The Loopscount will calculate, how ofteen the loops have to repeat, to last until the last note.
	int Loopcount = (offset_old / 4500) + 1;
	int Overlay_Time = 0;
	int Position_Time_X = 99;
	int Position_Time_Y = 458;

	storyboard << "[General]" << endl;
	storyboard << "UseSkinSprites: 1\n" << endl;
	storyboard << "[Events]" << endl;
	storyboard << "//Background and Video events" << endl;
	storyboard << "//Storyboard Layer 0 (Background)" << endl;
	storyboard << "//Storyboard Layer 1 (Fail)" << endl;
	storyboard << "//Storyboard Layer 2 (Pass)" << endl;
	storyboard << "//Storyboard Layer 3 (Foreground)" << endl;

	storyboard << "Sprite,Background,Centre," << '"' << "SB/Background_Dim.png" << '"' << ",320,240" << endl;
	storyboard << " M,0," << FirstOffset << ',' << offset_old + 5000 << ",320,245" << endl;
	storyboard << " F,0," << FirstOffset << "," << offset_old + 4000 << "," << Background_Dim << ","<< Background_Dim << endl;
	storyboard << " F,0," << offset_old + 4000 << "," << offset_old + 5000 << ",1,0" << endl;

	storyboard << "Sprite,Background,Centre," << '"' << "SB/Overlay.png" << '"' << ",320,240" << endl;
	storyboard << " M,0," << FirstOffset << ',' << offset_old + 5000 << ",320,245" << endl;
	storyboard << " F,0," << offset_old + 4000 << "," << offset_old + 5000 << ",1,0" << endl;
	
	storyboard << "Sprite,Background,Centre," << '"' << "SB/Overlay_Time_Shine.png" << '"' << ",320,240" << endl;
	storyboard << " M,0," << FirstOffset << ',' << offset_old + 5000 << ",320,235" << endl;
	storyboard << " L,0," << Loopcount << endl;
	storyboard << "  F,0,0,1500,0,0" << endl;
	storyboard << "  F,0,1500,3000,0,0.6" << endl;
	storyboard << "  F,0,3000,4500,0.6,0" << endl;
	storyboard << " F,0," << offset_old + 4000 << "," << offset_old + 5000 << ",0,0" << endl;

	storyboard << "Sprite,Background,Centre," << '"' << "SB/Overlay_Circle.png" << '"' << ",320,240" << endl;
	storyboard << " M,0," << FirstOffset << ',' << offset_old + 5000 << ",0,450" << endl;
	storyboard << " S,0," << FirstOffset << ',' << offset_old + 5000 << ",0.6" << endl;
	storyboard << " F,0," << offset_old + 4000 << "," << offset_old + 5000 << ",1,0" << endl;

	storyboard << "Sprite,Background,Centre," << '"' << "SB/Overlay_Circle_Shine.png" << '"' << ",320,240" << endl;
	storyboard << " M,0," << FirstOffset << ',' << offset_old + 5000 << ",0,450" << endl;
	storyboard << " S,0," << FirstOffset << ',' << offset_old + 5000 << ",0.6" << endl;
	storyboard << " L,0," << Loopcount << endl;
	storyboard << "  F,0,0,1500,0,0" << endl;
	storyboard << "  F,0,1500,3000,0,1" << endl;
	storyboard << "  F,0,3000,4500,1,0" << endl;
	storyboard << " F,0," << offset_old + 4000 << "," << offset_old + 5000 << ",0,0" << endl;

	storyboard << "Sprite,Background,Centre," << '"' << "SB/Difficulty_" << Difficulty[Diff] << ".png" << '"' << ",320,240" << endl;
	storyboard << " M,0," << FirstOffset << ',' << offset_old + 5000 << ",320,27" << endl; 
	storyboard << " F,0," << offset_old + 4000 << "," << offset_old + 5000 << ",1,0" << endl;

	storyboard << "Sprite,Background,Centre," << '"' << "SB/Diva.png" << '"' << ",320,240" << endl;
	storyboard << " M,0," << FirstOffset << ',' << offset_old + 5000 << ",80,27" << endl; 
	storyboard << " S,0," << FirstOffset << ',' << offset_old + 5000 << ",0.8" << endl;
	storyboard << " F,0," << offset_old + 4000 << "," << offset_old + 5000 << ",1,0" << endl;

	storyboard << "Sprite,Background,Centre," << '"' << "SB/Time.png" << '"' << ",320,240" << endl;
	storyboard << " M,0," << FirstOffset << ',' << offset_old + 5000 << ",81,457" << endl; 
	storyboard << " S,0," << FirstOffset << ',' << offset_old + 5000 << ",0.8" << endl;
	storyboard << " F,0," << offset_old + 4000 << "," << offset_old + 5000 << ",1,0" << endl;

	storyboard << "Sprite,Background,Centre," << '"' << "SB/Score.png" << '"' << ",320,240" << endl;
	storyboard << " M,0," << FirstOffset << ',' << offset_old + 5000 << ",495,27" << endl; 
	storyboard << " S,0," << FirstOffset << ',' << offset_old + 5000 << ",0.8" << endl;
	storyboard << " F,0," << offset_old + 4000 << "," << offset_old + 5000 << ",1,0" << endl;

	for (Dummy = 275; Dummy > 1; Dummy--)
	{
		Overlay_Time = Overlay_Time + (offset_old / 275);
		storyboard << "Sprite,Background,Centre," << '"' << "SB/Overlay_time_Empty.png" << '"' << ",320,240" << endl;
		storyboard << " M,0,0," << Overlay_Time << ',' << Position_Time_X << ',' << Position_Time_Y << endl; 
		storyboard << " F,0," << Overlay_Time << ',' << Overlay_Time + 1000 << ",1,0" << endl;
		Position_Time_X = Position_Time_X + 2;
	}

	for (Dummy = 0; Dummy < 50; Dummy++)
	{
		if(SV_Kiai2[Dummy] == 1)
		{
			Loopcount = ((SV_KiaiOffsets[Dummy + 1] - (SV_KiaiOffsets[Dummy] - 750)) / 2400);
			storyboard << "Sprite,Background,Centre," << '"' << "SB/Kiai_Time.png" << '"' << ",320,240" << endl;
			storyboard << " M,0," << SV_KiaiOffsets[Dummy] - 750 << ',' << SV_KiaiOffsets[Dummy] - 250 << ",-200,90,25,90" << endl;
			storyboard << " M,0," << SV_KiaiOffsets[Dummy] - 250 << ',' << SV_KiaiOffsets[Dummy + 1] << ",25,90" << endl;
			storyboard << " S,0," << SV_KiaiOffsets[Dummy] - 750 << ",,0.5" << endl;

			storyboard << "Sprite,Background,Centre," << '"' << "SB/Kiai_Time_Teal.png" << '"' << ",320,240" << endl;
			storyboard << " F,0," << SV_KiaiOffsets[Dummy + 1] - 500 << ',' << SV_KiaiOffsets[Dummy + 1] - 250 << ",0,1" << endl;
			storyboard << " M,0," << SV_KiaiOffsets[Dummy + 1] - 250 << ',' << SV_KiaiOffsets[Dummy + 1] + 1500 << ",25,90" << endl;
			storyboard << " M,0," << SV_KiaiOffsets[Dummy + 1] + 1500 << ',' << SV_KiaiOffsets[Dummy + 1] + 2133 << ",25,90,-200,90" << endl;
			storyboard << " S,0," << SV_KiaiOffsets[Dummy + 1] - 500 << ",,0.5" << endl;

			storyboard << "Sprite,Background,Centre," << '"' << "SB/Kiai_Time_Glow.png" << '"' << ",320,240" << endl;
			storyboard << " M,0," << SV_KiaiOffsets[Dummy] - 750 << ',' << SV_KiaiOffsets[Dummy] - 250 << ",-130,90,25,90" << endl;
			storyboard << " M,0," << SV_KiaiOffsets[Dummy] - 250 << ',' << SV_KiaiOffsets[Dummy + 1] << ",25,90" << endl;
			storyboard << " M,0," << SV_KiaiOffsets[Dummy + 1] + 1500 << ',' << SV_KiaiOffsets[Dummy + 1] + 2133 << ",25,90,-200,90" << endl;
			storyboard << " S,0," << SV_KiaiOffsets[Dummy] - 750 << ",,0.5" << endl;
			storyboard << " L," << SV_KiaiOffsets[Dummy] - 750 <<',' << Loopcount << endl;
			storyboard << "  F,0,0,1200,0.4,0.8" << endl;
			storyboard << "  F,0,1200,2400,0.8,0.4" << endl;
			storyboard << " L," << SV_KiaiOffsets[Dummy + 1] - 500 <<',' << 4 << endl;
			storyboard << "  F,0,0,333,0.4,0.8" << endl;
			storyboard << "  F,0,333,666,0.8,0.4" << endl;
		}
	}

// --------------------------- End of Overlay Creation -------------------------------------
	ChordCheck.close();
	Code_Type = 2;
	ChordCounter = 0;

// ---------------------------- Start of Storyboard Creation -----------------------
	
	while(getline (Read, currentLine))
	{
		/*
		 * [HitObject] Section.
		 * Line Format Examples:
		 * 320,192,9558,1,0,1:0:0:0:
		 * 192,192,9817,128,0,10334:1:0:0:0:
		 * Type 1 = Normal Note
		 * Type 128 = Long Note
		 * Type + 4 = First Note after a Break.
		 */			

		if (Code_Type == 1)
		{
			/*	
			Dividing the Line Format, into it's components.
			endsyntax = Part of Hitsounds for normal notes and the ending offset for LN's
			type = Normal note (1) or LN (128)
			*/
			int pos_x = std::stoi(GetPartial(currentLine, 0));
			int pos_y = std::stoi(GetPartial(currentLine, 1));
			int offset = std::stoi(GetPartial(currentLine, 2));
			int type = std::stoi(GetPartial(currentLine, 3));
			int hitsound = std::stoi(GetPartial(currentLine, 4));
			int endsyntax = std::stoi(GetPartial(currentLine, 5));

			ObjectType = GetObjectType(pos_x);

			/* 
			Determining of the Object Position.

			The Positions will randomize for every 1st ObjectType and then it will follow a random direction.
			If the Object get's too close to an border of the screen, then it will change its direction.
			The Pattern will also sometimes change it's direction by Random.
			These Positions in the switch start at the top with case 0 and then continue clockwise.
			The Inverted_Direction part prevents the pattern to turn back to the path, which it came from.
			The Random Direction Change is also influenced, by the Snap distance, the larger the stap distance,
			the higher the chance to get a random direction change. 
			*/
		
			if (Determine_Method == 1)
			{
				if (Position_X == 0 && Position_Y == 0)
				{
					Position_X = 120 + rand() % 420;
					Position_Y = 80 + rand() % 320;
				}
				if (ChordTimer <= 0)
				{
					Inverted_Direction = CheckForInvertedDirection(Pattern_Direction_old);

					if (offset_old != 0)
						SnapDistance = GetSnapDistance(offset, offset_old);
		
			
					//	SnapDistance2 had to be created, to prevent a performance problem, without SnapDistance2, the program would strangely drop to 1/100s of the actual speed.
					int SnapDistance2 = SnapDistance;

					Random_Direction_Change = 1;

					if (SnapDistance == 8)
					{
						Random_Direction_Change = rand() % 2;
					}
					else if (SnapDistance == 4)
					{
						Random_Direction_Change = rand() % 4;
					}
					else if (SnapDistance == 3)
					{
						Random_Direction_Change = rand() % 7;
					}
					else if (SnapDistance == 2)
					{
						Random_Direction_Change = rand() % 15;
					}

					if (Random_Direction_Change == 0)
					{
						while (Pattern_Direction != Inverted_Direction)
						{
							Pattern_Direction = rand() % 8;
						}
					}

					SnapDistance_Value = GetSnapDistance_Value(offset, offset_old);

					// This will prevent the first Object to cause a permanent loop.
					if (Position_Y != 0 && Position_X != 0)
						ChangeDirection = CheckForBorders(Position_Y, Position_X, Pattern_Direction);

					while (ChangeDirection == 1 || Pattern_Direction == Inverted_Direction)
					{
						Pattern_Direction = rand() % 8;
						ChangeDirection = CheckForBorders(Position_Y, Position_X, Pattern_Direction);
					}

					if (ObjectType_old > 3)
						ObjectType_old = ObjectType_old - 4;

					if (ObjectType_old != ObjectType && SnapDistance2 > 4)
					{
						Old_Position_X = Position_X;
						Old_Position_Y = Position_Y;
						Pattern_Direction = rand() % 8;

						do 
						{
						Position_X = 40 + rand() % 560;
						Position_Y = 140 + rand() % 260;
						Correct_Pattern_Distance = CheckForPatternDistance(Position_X,Old_Position_X,Position_Y,Old_Position_Y);
						}
						while (Correct_Pattern_Distance == false);
					}
					else
					{
						// This calculates the distance of the new object.
						switch (Pattern_Direction)
						{
							case 0: Position_Y = Position_Y + floor(SnapDistance_Value); break;
							case 1: Position_Y = Position_Y + floor(SnapDistance_Value / 1.5); Position_X = Position_X + floor(SnapDistance_Value / 1.5); break;
							case 2: Position_X = Position_X + floor(SnapDistance_Value); break;
							case 3: Position_X = Position_X + floor(SnapDistance_Value / 1.5); Position_Y = Position_Y - floor(SnapDistance_Value / 1.5); break;
							case 4: Position_Y = Position_Y - floor(SnapDistance_Value); break;
							case 5: Position_Y = Position_Y - floor(SnapDistance_Value / 1.5); Position_X = Position_X - floor(SnapDistance_Value / 1.5); break;
							case 6: Position_X = Position_X - floor(SnapDistance_Value); break;
							case 7: Position_X = Position_X - floor(SnapDistance_Value / 1.5); Position_Y = Position_Y + floor(SnapDistance_Value / 1.5); break;
						}
					}
					if(Chords[ChordCounter] <= (offset + 10) && Chords[ChordCounter] != 0)
					{
					ChordTimer = 2;
					}
				}
			}
			else
			{
				if (Position_X == 0 && Position_Y == 0)
				{
					Position_X = SBPositions_X[Note_Counter] + 65;
					Position_Y = SBPositions_Y[Note_Counter] + 73;
				}

				while (offset > SBPositions_Offset[Note_Counter])
				{
					Note_Counter++;
					Position_X = SBPositions_X[Note_Counter] + 65;
					Position_Y = SBPositions_Y[Note_Counter] + 73;
				}

				if(ChordTimer <= 0)
				{
					if(Chords[ChordCounter] <= (offset + 10) && Chords[ChordCounter] != 0)
					{
						ChordTimer = 2;
					}
				}

				SnapDistance = GetSnapDistance(offset, offset_old);

				if (SnapDistance > 4)
				{
					if (SBPositions_X[Note_Counter + 1] + 65 < Position_X)
					{
						Pattern_Direction = 6;
					}
					else if (SBPositions_X[Note_Counter + 1] + 65 > Position_X)
					{
						Pattern_Direction = 2;
					}
				}
				else
				{
					if (Position_X < Position_X_old)
					{
						Pattern_Direction = 6;
					}
					else if (Position_X > Position_X_old)
					{
						Pattern_Direction = 2;
					}
				}
				
			}
			

			if (Chords[ChordCounter] <= (offset + 10) && ObjectType < 4 && Chords[ChordCounter] != 0)
			{				
				ChordCounter++;
				GetHitsoundType(hitsound,endsyntax);
				// Creation of the Scores. This is a dublicate, for the non-created notes.
					storyboard << "Sprite,Background,Centre," << '"' << "SB/pd_hit300g.png" << '"' << ",320,240" << endl;
					storyboard << HitsoundType <<  offset - TimingOD[OD][4] << ',' << offset + TimingOD[OD][4] << endl;
					storyboard << "  M,0," << 0 << ',' <<  400 << ',' << Position_X - 27 << ',' << Position_Y - 50 << endl;
					storyboard << "  F,0," << 0 << ',' << 50 << ",0,1" << endl;
					storyboard << "  F,0," << 350 << ',' << 400 << "1,0" << endl;
					storyboard << "  S,0," << 0 << ',' <<  400 << ',' << "0.3" << endl;

			// ----------------------------------------------------------------------------------------------

					storyboard << "Sprite,Background,Centre," << '"' << "SB/pd_hit300.png" << '"' << ",320,240" << endl;
					storyboard << HitsoundType <<  offset - TimingOD[OD][3] << ',' << offset - TimingOD[OD][4] - 1 << endl;
					storyboard << "  M,0," << 0 << ',' << 400 << ',' << Position_X - 27 << ',' << Position_Y - 50 << endl;
					storyboard << "  F,0," << 0 << ',' << 50 << ",0,1" << endl;
					storyboard << "  F,0," << 350 << ',' << 400 << "1,0" << endl;
					storyboard << "  S,0," << 0 << ',' <<  400 << ',' << "0.3" << endl;

					storyboard << "Sprite,Background,Centre," << '"' << "SB/pd_hit300.png" << '"' << ",320,240" << endl;
					storyboard << HitsoundType <<  offset + TimingOD[OD][4] + 1 << ',' << offset + TimingOD[OD][3] << endl;
					storyboard << "  M,0," << 0 << ',' << 400 << ',' << Position_X - 27 << ',' << Position_Y - 50 << endl;
					storyboard << "  F,0," << 0 << ',' << 50 << ",0,1" << endl;
					storyboard << "  F,0," << 350 << ',' << 400 << "1,0" << endl;
					storyboard << "  S,0," << 0 << ',' <<  400 << ',' << "0.3" << endl;

			// ----------------------------------------------------------------------------------------------

					storyboard << "Sprite,Background,Centre," << '"' << "SB/pd_hit200.png" << '"' << ",320,240" << endl;
					storyboard << HitsoundType <<  offset - TimingOD[OD][2] << ',' << offset - TimingOD[OD][3] - 1 << endl;
					storyboard << "  M,0," << 0 << ',' << 400 << ',' << Position_X - 27 << ',' << Position_Y - 50 << endl;
					storyboard << "  F,0," << 0 << ',' << 50 << ",0,1" << endl;
					storyboard << "  F,0," << 350 << ',' << 400 << "1,0" << endl;
					storyboard << "  S,0," << 0 << ',' <<  400 << ',' << "0.3" << endl;

					storyboard << "Sprite,Background,Centre," << '"' << "SB/pd_hit200.png" << '"' << ",320,240" << endl;
					storyboard << HitsoundType <<  offset + TimingOD[OD][3] + 1 << ',' << offset + TimingOD[OD][2] + 1 << endl;
					storyboard << "  M,0," << 0 << ',' << 400 << ',' << Position_X - 27 << ',' << Position_Y - 50 << endl;
					storyboard << "  F,0," << 0 << ',' << 50 << ",0,1" << endl;
					storyboard << "  F,0," << 350 << ',' << 400 << "1,0" << endl;
					storyboard << "  S,0," << 0 << ',' <<  400 << ',' << "0.3" << endl;

			// ----------------------------------------------------------------------------------------------

					storyboard << "Sprite,Background,Centre," << '"' << "SB/pd_hit100.png" << '"' << ",320,240" << endl;
					storyboard << HitsoundType <<  offset - TimingOD[OD][1] << ',' << offset - TimingOD[OD][2] - 1 << endl;
					storyboard << "  M,0," << 0 << ',' << 400 << ',' << Position_X - 27 << ',' << Position_Y - 50 << endl;
					storyboard << "  F,0," << 0 << ',' << 50 << ",0,1" << endl;
					storyboard << "  F,0," << 350 << ',' << 400 << "1,0" << endl;
					storyboard << "  S,0," << 0 << ',' <<  400 << ',' << "0.3" << endl;

					storyboard << "Sprite,Background,Centre," << '"' << "SB/pd_hit100.png" << '"' << ",320,240" << endl;
					storyboard << HitsoundType <<  offset + TimingOD[OD][2] + 1 << ',' << offset + TimingOD[OD][1] + 1 << endl;
					storyboard << "  M,0," << 0 << ',' << 400 << ',' << Position_X - 27 << ',' << Position_Y - 50 << endl;
					storyboard << "  F,0," << 0 << ',' << 50 << ",0,1" << endl;
					storyboard << "  F,0," << 350 << ',' << 400 << "1,0" << endl;
					storyboard << "  S,0," << 0 << ',' <<  400 << ',' << "0.3" << endl;

			// ----------------------------------------------------------------------------------------------

					storyboard << "Sprite,Background,Centre," << '"' << "SB/pd_hit50.png" << '"' << ",320,240" << endl;
					storyboard << HitsoundType <<  offset - TimingOD[OD][0] << ',' << offset - TimingOD[OD][1] - 1 << endl;
					storyboard << "  M,0," << 0 << ',' << 400 << ',' << Position_X - 27 << ',' << Position_Y - 50 << endl;
					storyboard << "  F,0," << 0 << ',' << 50 << ",0,1" << endl;
					storyboard << "  F,0," << 350 << ',' << 400 << "1,0" << endl;
					storyboard << "  S,0," << 0 << ',' <<  400 << ',' << "0.3" << endl;

					storyboard << "Sprite,Background,Centre," << '"' << "SB/pd_hit50.png" << '"' << ",320,240" << endl;
					storyboard << HitsoundType <<  offset + TimingOD[OD][1] + 1 << ',' << offset + TimingOD[OD][0] + 1 << endl;
					storyboard << "  M,0," << 0 << ',' << 400 << ',' << Position_X - 27 << ',' << Position_Y - 50 << endl;
					storyboard << "  F,0," << 0 << ',' << 50 << ",0,1" << endl;
					storyboard << "  F,0," << 350 << ',' << 400 << "1,0" << endl;
					storyboard << "  S,0," << 0 << ',' <<  400 << ',' << "0.3" << endl;
					offset = offset_old;
		
			}
			else
			{
				ObjectType_old = ObjectType;
				ObjectType = GetObjectType(pos_x);

				/* 
				This will determine the BPM and Inherited Time points, for each Hit Object.
				The SVChange Value will be listed as a negative large number.
				Therefor to get an actual Percentage value, the program will recalculate a new value, which it can work with.
				The BPMT will determine the Time for 4 bars in 1/4 Snap Divisor.
				The SV_Value will include the actual SV value.
				*/
				if (offset >= BPMOffsets[BPMCounter + 1] && BPMOffsets[BPMCounter + 1] != 0)
					BPMCounter++;
			
				BPMT = BPMs[BPMCounter];
			
				if(SV_Kiai[SVCounter] == 1)
				{
					if (offset > SVOffsets[SVCounter + 1] && SVOffsets[SVCounter + 1] != 0)
						SVCounter++;
				}
				else
				{
					if (offset >= SVOffsets[SVCounter + 1] && SVOffsets[SVCounter + 1] != 0)
						SVCounter++;
				}

				if(SV_Kiai2[KiaiCounter] == 1)
				{
					if (offset > SV_KiaiOffsets[KiaiCounter + 1] && SV_KiaiOffsets[KiaiCounter + 1] != 0)
						KiaiCounter++;
				}
				else
				{
					if (offset >= SV_KiaiOffsets[KiaiCounter + 1] && SV_KiaiOffsets[KiaiCounter + 1] != 0)
						KiaiCounter++;
				}

				SV_Value = 1 / (100 / ((-1) *SVChange[SVCounter]));

				// ------------------- Start of Kiai Note Count Creation ---------------
				
				if (offset >= SV_KiaiOffsets[KiaiCounter] && KiaiNoteAmount[KiaiCounter] != 0)
				{
					DetermineNumbers(KiaiNoteAmount[KiaiCounter], last_offset, offset);
					KiaiNoteAmount[KiaiCounter]--;
				}

				// ------------------- End of Kiai Note Count Creation ---------------
				
				

				// Creation of the Position and Size modificiation code.

				// This switch determines the target Object for normal Notes.
				if (type < 6)
				{
					if (SV_Kiai2[KiaiCounter] == 0)
					{
						switch (ObjectType)
						{
							case 0: storyboard << "Sprite,Foreground,Centre," << '"' << "SB/TriangleD.png" << '"' << ",320,240" << endl; break;		
							case 1: storyboard << "Sprite,Foreground,Centre," << '"' << "SB/CircleD.png" << '"' << ",320,240" << endl; break;	
							case 2: storyboard << "Sprite,Foreground,Centre," << '"' << "SB/CrossD.png" << '"' << ",320,240" << endl; break;	
							case 3: storyboard << "Sprite,Foreground,Centre," << '"' << "SB/SquareD.png" << '"' << ",320,240" << endl; break;	
							case 4: storyboard << "Sprite,Foreground,Centre," << '"' << "SB/ArrowUpD.png" << '"' << ",320,240" << endl; break;	
							case 5: storyboard << "Sprite,Foreground,Centre," << '"' << "SB/ArrowRightD.png" << '"' << ",320,240" << endl; break;	
							case 6: storyboard << "Sprite,Foreground,Centre," << '"' << "SB/ArrowDownD.png" << '"' << ",320,240" << endl; break;	
							case 7: storyboard << "Sprite,Foreground,Centre," << '"' << "SB/ArrowLeftD.png" << '"' << ",320,240" << endl; break;	
						}
					}
					else
					{
						switch (ObjectType)
						{
							case 0: storyboard << "Sprite,Foreground,Centre," << '"' << "SB/TriangleD_Gold.png" << '"' << ",320,240" << endl; break;		
							case 1: storyboard << "Sprite,Foreground,Centre," << '"' << "SB/CircleD_Gold.png" << '"' << ",320,240" << endl; break;	
							case 2: storyboard << "Sprite,Foreground,Centre," << '"' << "SB/CrossD_Gold.png" << '"' << ",320,240" << endl; break;	
							case 3: storyboard << "Sprite,Foreground,Centre," << '"' << "SB/SquareD_Gold.png" << '"' << ",320,240" << endl; break;	
							case 4: storyboard << "Sprite,Foreground,Centre," << '"' << "SB/ArrowUpD_Gold.png" << '"' << ",320,240" << endl; break;	
							case 5: storyboard << "Sprite,Foreground,Centre," << '"' << "SB/ArrowRightD_Gold.png" << '"' << ",320,240" << endl; break;	
							case 6: storyboard << "Sprite,Foreground,Centre," << '"' << "SB/ArrowDownD_Gold.png" << '"' << ",320,240" << endl; break;	
							case 7: storyboard << "Sprite,Foreground,Centre," << '"' << "SB/ArrowLeftD_Gold.png" << '"' << ",320,240" << endl; break;	
						}
					}
				}
				else
				{
					if (SV_Kiai2[KiaiCounter] == 0)
					{
						switch (ObjectType)
						{
							case 0: storyboard << "Sprite,Foreground,Centre," << '"' << "SB/TriangleD_LN.png" << '"' << ",320,240" << endl; break;		
							case 1: storyboard << "Sprite,Foreground,Centre," << '"' << "SB/CircleD_LN.png" << '"' << ",320,240" << endl; break;	
							case 2: storyboard << "Sprite,Foreground,Centre," << '"' << "SB/CrossD_LN.png" << '"' << ",320,240" << endl; break;	
							case 3: storyboard << "Sprite,Foreground,Centre," << '"' << "SB/SquareD_LN.png" << '"' << ",320,240" << endl; break;	
							case 4: storyboard << "Sprite,Foreground,Centre," << '"' << "SB/ArrowUpD_LN.png" << '"' << ",320,240" << endl; break;	
							case 5: storyboard << "Sprite,Foreground,Centre," << '"' << "SB/ArrowRightD_LN.png" << '"' << ",320,240" << endl; break;	
							case 6: storyboard << "Sprite,Foreground,Centre," << '"' << "SB/ArrowDownD_LN.png" << '"' << ",320,240" << endl; break;	
							case 7: storyboard << "Sprite,Foreground,Centre," << '"' << "SB/ArrowLeftD_LN.png" << '"' << ",320,240" << endl; break;	
						}
					}
					else
					{
						switch (ObjectType)
						{
							case 0: storyboard << "Sprite,Foreground,Centre," << '"' << "SB/TriangleD_LN_Gold.png" << '"' << ",320,240" << endl; break;		
							case 1: storyboard << "Sprite,Foreground,Centre," << '"' << "SB/CircleD_LN_Gold.png" << '"' << ",320,240" << endl; break;	
							case 2: storyboard << "Sprite,Foreground,Centre," << '"' << "SB/CrossD_LN_Gold.png" << '"' << ",320,240" << endl; break;	
							case 3: storyboard << "Sprite,Foreground,Centre," << '"' << "SB/SquareD_LN_Gold.png" << '"' << ",320,240" << endl; break;	
							case 4: storyboard << "Sprite,Foreground,Centre," << '"' << "SB/ArrowUpD_LN_Gold.png" << '"' << ",320,240" << endl; break;	
							case 5: storyboard << "Sprite,Foreground,Centre," << '"' << "SB/ArrowRightD_LN_Gold.png" << '"' << ",320,240" << endl; break;	
							case 6: storyboard << "Sprite,Foreground,Centre," << '"' << "SB/ArrowDownD_LN_Gold.png" << '"' << ",320,240" << endl; break;	
							case 7: storyboard << "Sprite,Foreground,Centre," << '"' << "SB/ArrowLeftD_LN_Gold.png" << '"' << ",320,240" << endl; break;	
						}
					}
				}

				storyboard << " M,0," << offset - floor(1500 * SV_Value) << ',' << offset << ',' << Position_X << ',' << Position_Y << endl;
				storyboard << " S,0," << offset - floor(1500 * SV_Value) << ',' << offset - floor(1300 * SV_Value) << ',' << "0.8" << endl;
				storyboard << " S,0," << offset - floor(1500 * SV_Value) << ',' << offset - floor(1219 * SV_Value) << ',' << "0.525" << endl;

	// ---------------------------- Target Object for LN's ---------------------- 
	
				if (type > 5)
				{
					if (SV_Kiai2[KiaiCounter] == 0)
					{
						switch (ObjectType)
						{
							case 0: storyboard << "Sprite,Foreground,Centre," << '"' << "SB/TriangleD_LN.png" << '"' << ",320,240" << endl; break;		
							case 1: storyboard << "Sprite,Foreground,Centre," << '"' << "SB/CircleD_LN.png" << '"' << ",320,240" << endl; break;	
							case 2: storyboard << "Sprite,Foreground,Centre," << '"' << "SB/CrossD_LN.png" << '"' << ",320,240" << endl; break;	
							case 3: storyboard << "Sprite,Foreground,Centre," << '"' << "SB/SquareD_LN.png" << '"' << ",320,240" << endl; break;	
							case 4: storyboard << "Sprite,Foreground,Centre," << '"' << "SB/ArrowUpD_LN.png" << '"' << ",320,240" << endl; break;	
							case 5: storyboard << "Sprite,Foreground,Centre," << '"' << "SB/ArrowRightD_LN.png" << '"' << ",320,240" << endl; break;	
							case 6: storyboard << "Sprite,Foreground,Centre," << '"' << "SB/ArrowDownD_LN.png" << '"' << ",320,240" << endl; break;	
							case 7: storyboard << "Sprite,Foreground,Centre," << '"' << "SB/ArrowLeftD_LN.png" << '"' << ",320,240" << endl; break;	
						}
					}
					else
					{
						switch (ObjectType)
						{
							case 0: storyboard << "Sprite,Foreground,Centre," << '"' << "SB/TriangleD_LN_Gold.png" << '"' << ",320,240" << endl; break;		
							case 1: storyboard << "Sprite,Foreground,Centre," << '"' << "SB/CircleD_LN_Gold.png" << '"' << ",320,240" << endl; break;	
							case 2: storyboard << "Sprite,Foreground,Centre," << '"' << "SB/CrossD_LN_Gold.png" << '"' << ",320,240" << endl; break;	
							case 3: storyboard << "Sprite,Foreground,Centre," << '"' << "SB/SquareD_LN_Gold.png" << '"' << ",320,240" << endl; break;	
							case 4: storyboard << "Sprite,Foreground,Centre," << '"' << "SB/ArrowUpD_LN_Gold.png" << '"' << ",320,240" << endl; break;	
							case 5: storyboard << "Sprite,Foreground,Centre," << '"' << "SB/ArrowRightD_LN_Gold.png" << '"' << ",320,240" << endl; break;	
							case 6: storyboard << "Sprite,Foreground,Centre," << '"' << "SB/ArrowDownD_LN_Gold.png" << '"' << ",320,240" << endl; break;	
							case 7: storyboard << "Sprite,Foreground,Centre," << '"' << "SB/ArrowLeftD_LN_Gold.png" << '"' << ",320,240" << endl; break;	
						}
					}
					storyboard << " M,0,"<< offset << ',' << endsyntax << ',' << Position_X << ',' << Position_Y << endl;
					storyboard << " S,0," << offset << ',' << endsyntax - floor(1500 * SV_Value) << ',' << "0.525" << endl;
					storyboard << " S,0," << endsyntax - floor(1500 * SV_Value) << ',' << endsyntax - floor(1300 * SV_Value) << ',' << "0.8" << endl;
					storyboard << " S,0," << endsyntax - floor(1300 * SV_Value) << ",," << "0.525" << endl;
				}

				// Creation of the Pointer.

				storyboard << "Sprite,Foreground,Centre," << '"' << "SB/Pointer.png" << '"' << ",320,240" << endl;
				storyboard << " M,0," << offset - floor(1500 * SV_Value) << ',' << offset << ',' << Position_X << ',' << Position_Y << endl;
				storyboard << " S,0," << offset - floor(1500 * SV_Value) << ',' << offset - floor(1300 * SV_Value) << ',' << "0.75" << endl;
				storyboard << " S,0," << offset - floor(1200 * SV_Value) << ',' << offset - floor(1219 * SV_Value) << ',' << "0.525" << endl;
				storyboard << " R,0," << offset - floor(1500 * SV_Value) << ',' << offset << ",0,6.283" << endl;
				if (type > 5)
				{
					storyboard << " M,0," << offset << ',' << endsyntax << ',' << Position_X << ',' << Position_Y << endl;

					storyboard << "Sprite,Foreground,Centre," << '"' << "SB/Pointer.png" << '"' << ",320,240" << endl;
					storyboard << " M,0," << endsyntax - floor(1500 * SV_Value) << ',' << endsyntax << ',' << Position_X << ',' << Position_Y << endl;
					storyboard << " S,0," << endsyntax - floor(1500 * SV_Value) << ',' << endsyntax - floor(1300 * SV_Value) << ',' << "0.75" << endl;
					storyboard << " S,0," << endsyntax - floor(1200 * SV_Value) << ',' << endsyntax - floor(1219 * SV_Value) << ',' << "0.525" << endl;
					storyboard << " R,0," << endsyntax - floor(1500 * SV_Value) << ',' << endsyntax << ",0,6.283" << endl;
				}

				// Creation of the appearence Effect.

				storyboard << "Sprite,Foreground,Centre," << '"' << "SB/Approach.png" << '"' << ",320,240" << endl;
				storyboard << " M,0," << offset - floor(1500 * SV_Value) << ',' << offset - floor(1100 * SV_Value) << ',' << Position_X << ',' << Position_Y << endl;
				storyboard << " S,0," << offset - floor(1500 * SV_Value) << ',' << offset - floor(1100 * SV_Value) << ',' << "0.25,1.0" << endl;

				if (type > 5)
				{
					storyboard << "Sprite,Foreground,Centre," << '"' << "SB/Approach.png" << '"' << ",320,240" << endl;
					storyboard << " M,0," << endsyntax - floor(1500 * SV_Value) << ',' << endsyntax - floor(1100 * SV_Value) << ',' << Position_X << ',' << Position_Y << endl;
					storyboard << " S,0," << endsyntax - floor(1500 * SV_Value) << ',' << endsyntax - floor(1100 * SV_Value) << ',' << "0.25,1.0" << endl;
				}

				// Creation of the Hit Effect.
				if (type < 6)
				{
					GetHitsoundType(hitsound, endsyntax);
				}
				else
				{
					string HitsoundType2 = currentLine.substr(currentLine.find(":")+1,1);
					int HitsoundType3 = atoi(HitsoundType2.c_str());
					GetHitsoundType(hitsound,HitsoundType3);
				}
				if (type < 6)
				{
					storyboard << "Sprite,Background,Centre," << '"' << "SB/HitEffect.png" << '"' << ",320,240" << endl;
					storyboard << HitsoundType <<  offset - TimingOD[OD][0] << ',' << offset + TimingOD[OD][0] << endl;
					storyboard << "  M,0," << 0 << ',' <<  400 << ',' << Position_X << ',' << Position_Y << endl;
					storyboard << "  S,0," << 0 << ',' <<  400 << ',' << "0.20,0.70" << endl;
					storyboard << "  F,0," << 250 << ',' << 400 << ",1,0" << endl;
				}
				else
				{
					storyboard << "Sprite,Background,Centre," << '"' << "SB/HitEffect.png" << '"' << ",320,240" << endl;
					storyboard << "  M,0," << endsyntax << ',' <<  endsyntax + 400 << ',' << Position_X << ',' << Position_Y << endl;
					storyboard << "  S,0," << endsyntax << ',' <<  endsyntax + 400 << ',' << "0.20,0.70" << endl;
					storyboard << "  F,0," << endsyntax + 250 << ',' << endsyntax + 400 << ",1,0" << endl;
				}
			
				
				// Creation of the Scores.
				if (type < 6)
				{
					if (ObjectType < 4)
					{
						// Score Creation for Normal Notes
						storyboard << "Sprite,Background,Centre," << '"' << "SB/pd_hit300g.png" << '"' << ",320,240" << endl;
						storyboard << HitsoundType <<  offset - TimingOD[OD][4] << ',' << offset + TimingOD[OD][4] << endl;
						storyboard << "  M,0," << 0 << ',' <<  400 << ',' << Position_X << ',' << Position_Y - 50 << endl;
						storyboard << "  F,0," << 0 << ',' << 50 << ",0,1" << endl;
						storyboard << "  F,0," << 350 << ',' << 400 << "1,0" << endl;
						storyboard << "  S,0," << 0 << ',' <<  400 << ',' << "0.4" << endl;

				// ----------------------------------------------------------------------------------------------

						storyboard << "Sprite,Background,Centre," << '"' << "SB/pd_hit300.png" << '"' << ",320,240" << endl;
						storyboard << HitsoundType <<  offset - TimingOD[OD][3] << ',' << offset - TimingOD[OD][4] - 1 << endl;
						storyboard << "  M,0," << 0 << ',' << 400 << ',' << Position_X << ',' << Position_Y - 50 << endl;
						storyboard << "  F,0," << 0 << ',' << 50 << ",0,1" << endl;
						storyboard << "  F,0," << 350 << ',' << 400 << "1,0" << endl;
						storyboard << "  S,0," << 0 << ',' <<  400 << ',' << "0.4" << endl;

						storyboard << "Sprite,Background,Centre," << '"' << "SB/pd_hit300.png" << '"' << ",320,240" << endl;
						storyboard << HitsoundType <<  offset + TimingOD[OD][4] + 1 << ',' << offset + TimingOD[OD][3] << endl;
						storyboard << "  M,0," << 0 << ',' << 400 << ',' << Position_X << ',' << Position_Y - 50 << endl;
						storyboard << "  F,0," << 0 << ',' << 50 << ",0,1" << endl;
						storyboard << "  F,0," << 350 << ',' << 400 << "1,0" << endl;
						storyboard << "  S,0," << 0 << ',' <<  400 << ',' << "0.4" << endl;

				// ----------------------------------------------------------------------------------------------

						storyboard << "Sprite,Background,Centre," << '"' << "SB/pd_hit200.png" << '"' << ",320,240" << endl;
						storyboard << HitsoundType <<  offset - TimingOD[OD][2] << ',' << offset - TimingOD[OD][3] - 1 << endl;
						storyboard << "  M,0," << 0 << ',' << 400 << ',' << Position_X << ',' << Position_Y - 50 << endl;
						storyboard << "  F,0," << 0 << ',' << 50 << ",0,1" << endl;
						storyboard << "  F,0," << 350 << ',' << 400 << "1,0" << endl;
						storyboard << "  S,0," << 0 << ',' <<  400 << ',' << "0.4" << endl;

						storyboard << "Sprite,Background,Centre," << '"' << "SB/pd_hit200.png" << '"' << ",320,240" << endl;
						storyboard << HitsoundType <<  offset + TimingOD[OD][3] + 1 << ',' << offset + TimingOD[OD][2] + 1 << endl;
						storyboard << "  M,0," << 0 << ',' << 400 << ',' << Position_X << ',' << Position_Y - 50 << endl;
						storyboard << "  F,0," << 0 << ',' << 50 << ",0,1" << endl;
						storyboard << "  F,0," << 350 << ',' << 400 << "1,0" << endl;
						storyboard << "  S,0," << 0 << ',' <<  400 << ',' << "0.4" << endl;

				// ----------------------------------------------------------------------------------------------

						storyboard << "Sprite,Background,Centre," << '"' << "SB/pd_hit100.png" << '"' << ",320,240" << endl;
						storyboard << HitsoundType <<  offset - TimingOD[OD][1] << ',' << offset - TimingOD[OD][2] - 1 << endl;
						storyboard << "  M,0," << 0 << ',' << 400 << ',' << Position_X << ',' << Position_Y - 50 << endl;
						storyboard << "  F,0," << 0 << ',' << 50 << ",0,1" << endl;
						storyboard << "  F,0," << 350 << ',' << 400 << "1,0" << endl;
						storyboard << "  S,0," << 0 << ',' <<  400 << ',' << "0.4" << endl;

						storyboard << "Sprite,Background,Centre," << '"' << "SB/pd_hit100.png" << '"' << ",320,240" << endl;
						storyboard << HitsoundType <<  offset + TimingOD[OD][2] + 1 << ',' << offset + TimingOD[OD][1] + 1 << endl;
						storyboard << "  M,0," << 0 << ',' << 400 << ',' << Position_X << ',' << Position_Y - 50 << endl;
						storyboard << "  F,0," << 0 << ',' << 50 << ",0,1" << endl;
						storyboard << "  F,0," << 350 << ',' << 400 << "1,0" << endl;
						storyboard << "  S,0," << 0 << ',' <<  400 << ',' << "0.4" << endl;

				// ----------------------------------------------------------------------------------------------

						storyboard << "Sprite,Background,Centre," << '"' << "SB/pd_hit50.png" << '"' << ",320,240" << endl;
						storyboard << HitsoundType <<  offset - TimingOD[OD][0] << ',' << offset - TimingOD[OD][1] - 1 << endl;
						storyboard << "  M,0," << 0 << ',' << 400 << ',' << Position_X << ',' << Position_Y - 50 << endl;
						storyboard << "  F,0," << 0 << ',' << 50 << ",0,1" << endl;
						storyboard << "  F,0," << 350 << ',' << 400 << "1,0" << endl;
						storyboard << "  S,0," << 0 << ',' <<  400 << ',' << "0.4" << endl;

						storyboard << "Sprite,Background,Centre," << '"' << "SB/pd_hit50.png" << '"' << ",320,240" << endl;
						storyboard << HitsoundType <<  offset + TimingOD[OD][1] + 1 << ',' << offset + TimingOD[OD][0] + 1 << endl;
						storyboard << "  M,0," << 0 << ',' << 400 << ',' << Position_X << ',' << Position_Y - 50 << endl;
						storyboard << "  F,0," << 0 << ',' << 50 << ",0,1" << endl;
						storyboard << "  F,0," << 350 << ',' << 400 << "1,0" << endl;
						storyboard << "  S,0," << 0 << ',' <<  400 << ',' << "0.4" << endl;
					}
					else
					{
						// Score Creation for Arrows 
						storyboard << "Sprite,Background,Centre," << '"' << "SB/pd_hit300g.png" << '"' << ",320,240" << endl;
						storyboard << HitsoundType <<  offset - TimingOD[OD][4] << ',' << offset + TimingOD[OD][4] << endl;
						storyboard << "  M,0," << 0 << ',' <<  400 << ',' << Position_X + 27 << ',' << Position_Y - 50 << endl;
						storyboard << "  F,0," << 0 << ',' << 50 << ",0,1" << endl;
						storyboard << "  F,0," << 350 << ',' << 400 << "1,0" << endl;
						storyboard << "  S,0," << 0 << ',' <<  400 << ',' << "0.3" << endl;

				// ----------------------------------------------------------------------------------------------

						storyboard << "Sprite,Background,Centre," << '"' << "SB/pd_hit300.png" << '"' << ",320,240" << endl;
						storyboard << HitsoundType <<  offset - TimingOD[OD][3] << ',' << offset - TimingOD[OD][4] - 1 << endl;
						storyboard << "  M,0," << 0 << ',' << 400 << ',' << Position_X + 27 << ',' << Position_Y - 50 << endl;
						storyboard << "  F,0," << 0 << ',' << 50 << ",0,1" << endl;
						storyboard << "  F,0," << 350 << ',' << 400 << "1,0" << endl;
						storyboard << "  S,0," << 0 << ',' <<  400 << ',' << "0.3" << endl;

						storyboard << "Sprite,Background,Centre," << '"' << "SB/pd_hit300.png" << '"' << ",320,240" << endl;
						storyboard << HitsoundType <<  offset + TimingOD[OD][4] + 1 << ',' << offset + TimingOD[OD][3] << endl;
						storyboard << "  M,0," << 0 << ',' << 400 << ',' << Position_X + 27 << ',' << Position_Y - 50 << endl;
						storyboard << "  F,0," << 0 << ',' << 50 << ",0,1" << endl;
						storyboard << "  F,0," << 350 << ',' << 400 << "1,0" << endl;
						storyboard << "  S,0," << 0 << ',' <<  400 << ',' << "0.3" << endl;

				// ----------------------------------------------------------------------------------------------

						storyboard << "Sprite,Background,Centre," << '"' << "SB/pd_hit200.png" << '"' << ",320,240" << endl;
						storyboard << HitsoundType <<  offset - TimingOD[OD][2] << ',' << offset - TimingOD[OD][3] - 1 << endl;
						storyboard << "  M,0," << 0 << ',' << 400 << ',' << Position_X + 27 << ',' << Position_Y - 50 << endl;
						storyboard << "  F,0," << 0 << ',' << 50 << ",0,1" << endl;
						storyboard << "  F,0," << 350 << ',' << 400 << "1,0" << endl;
						storyboard << "  S,0," << 0 << ',' <<  400 << ',' << "0.3" << endl;

						storyboard << "Sprite,Background,Centre," << '"' << "SB/pd_hit200.png" << '"' << ",320,240" << endl;
						storyboard << HitsoundType <<  offset + TimingOD[OD][3] + 1 << ',' << offset + TimingOD[OD][2] + 1 << endl;
						storyboard << "  M,0," << 0 << ',' << 400 << ',' << Position_X + 27 << ',' << Position_Y - 50 << endl;
						storyboard << "  F,0," << 0 << ',' << 50 << ",0,1" << endl;
						storyboard << "  F,0," << 350 << ',' << 400 << "1,0" << endl;
						storyboard << "  S,0," << 0 << ',' <<  400 << ',' << "0.3" << endl;

				// ----------------------------------------------------------------------------------------------

						storyboard << "Sprite,Background,Centre," << '"' << "SB/pd_hit100.png" << '"' << ",320,240" << endl;
						storyboard << HitsoundType <<  offset - TimingOD[OD][1] << ',' << offset - TimingOD[OD][2] - 1 << endl;
						storyboard << "  M,0," << 0 << ',' << 400 << ',' << Position_X + 27 << ',' << Position_Y - 50 << endl;
						storyboard << "  F,0," << 0 << ',' << 50 << ",0,1" << endl;
						storyboard << "  F,0," << 350 << ',' << 400 << "1,0" << endl;
						storyboard << "  S,0," << 0 << ',' <<  400 << ',' << "0.3" << endl;

						storyboard << "Sprite,Background,Centre," << '"' << "SB/pd_hit100.png" << '"' << ",320,240" << endl;
						storyboard << HitsoundType <<  offset + TimingOD[OD][2] + 1 << ',' << offset + TimingOD[OD][1] + 1 << endl;
						storyboard << "  M,0," << 0 << ',' << 400 << ',' << Position_X + 27 << ',' << Position_Y - 50 << endl;
						storyboard << "  F,0," << 0 << ',' << 50 << ",0,1" << endl;
						storyboard << "  F,0," << 350 << ',' << 400 << "1,0" << endl;
						storyboard << "  S,0," << 0 << ',' <<  400 << ',' << "0.3" << endl;

				// ----------------------------------------------------------------------------------------------

						storyboard << "Sprite,Background,Centre," << '"' << "SB/pd_hit50.png" << '"' << ",320,240" << endl;
						storyboard << HitsoundType <<  offset - TimingOD[OD][0] << ',' << offset - TimingOD[OD][1] - 1 << endl;
						storyboard << "  M,0," << 0 << ',' << 400 << ',' << Position_X + 27 << ',' << Position_Y - 50 << endl;
						storyboard << "  F,0," << 0 << ',' << 50 << ",0,1" << endl;
						storyboard << "  F,0," << 350 << ',' << 400 << "1,0" << endl;
						storyboard << "  S,0," << 0 << ',' <<  400 << ',' << "0.3" << endl;

						storyboard << "Sprite,Background,Centre," << '"' << "SB/pd_hit50.png" << '"' << ",320,240" << endl;
						storyboard << HitsoundType <<  offset + TimingOD[OD][1] + 1 << ',' << offset + TimingOD[OD][0] + 1 << endl;
						storyboard << "  M,0," << 0 << ',' << 400 << ',' << Position_X + 27 << ',' << Position_Y - 50 << endl;
						storyboard << "  F,0," << 0 << ',' << 50 << ",0,1" << endl;
						storyboard << "  F,0," << 350 << ',' << 400 << "1,0" << endl;
						storyboard << "  S,0," << 0 << ',' <<  400 << ',' << "0.3" << endl;
					}
				}
				else
				{
					// This is the Score timing for Longnotes.
					int LengthLN = endsyntax - offset;

					storyboard << "Sprite,Background,Centre," << '"' << "SB/pd_hit300g.png" << '"' << ",320,240" << endl;
					storyboard << HitsoundType <<  offset - TimingOD[OD][4] << ',' << offset + TimingOD[OD][4] << endl;
					storyboard << "  M,0," << LengthLN << ',' <<  LengthLN + 400 << ',' << Position_X << ',' << Position_Y - 50 << endl;
					storyboard << "  F,0," << LengthLN << ',' << LengthLN + 50 << ",0,1" << endl;
					storyboard << "  F,0," << LengthLN + 350 << ',' << LengthLN + 400 << "1,0" << endl;
					storyboard << "  S,0," << LengthLN << ',' <<  LengthLN + 400 << ',' << "0.4" << endl;

			// ----------------------------------------------------------------------------------------------

					storyboard << "Sprite,Background,Centre," << '"' << "SB/pd_hit300.png" << '"' << ",320,240" << endl;
					storyboard << HitsoundType <<  offset - TimingOD[OD][3] << ',' << offset - TimingOD[OD][4] - 1 << endl;
					storyboard << "  M,0," << LengthLN << ',' << LengthLN + 400 << ',' << Position_X << ',' << Position_Y - 50 << endl;
					storyboard << "  F,0," << LengthLN << ',' << LengthLN + 50 << ",0,1" << endl;
					storyboard << "  F,0," << LengthLN + 350 << ',' << LengthLN + 400 << "1,0" << endl;
					storyboard << "  S,0," << LengthLN << ',' <<  LengthLN + 400 << ',' << "0.4" << endl;

					storyboard << "Sprite,Background,Centre," << '"' << "SB/pd_hit300.png" << '"' << ",320,240" << endl;
					storyboard << HitsoundType <<  offset + TimingOD[OD][4] + 1 << ',' << offset + TimingOD[OD][3] << endl;
					storyboard << "  M,0," << LengthLN << ',' << LengthLN + 400 << ',' << Position_X << ',' << Position_Y - 50 << endl;
					storyboard << "  F,0," << LengthLN << ',' << LengthLN + 50 << ",0,1" << endl;
					storyboard << "  F,0," << LengthLN + 350 << ',' << LengthLN + 400 << "1,0" << endl;
					storyboard << "  S,0," << LengthLN << ',' <<  LengthLN + 400 << ',' << "0.4" << endl;

			// ----------------------------------------------------------------------------------------------

					storyboard << "Sprite,Background,Centre," << '"' << "SB/pd_hit200.png" << '"' << ",320,240" << endl;
					storyboard << HitsoundType <<  offset - TimingOD[OD][2] << ',' << offset - TimingOD[OD][3] - 1 << endl;
					storyboard << "  M,0," << LengthLN << ',' << LengthLN + 400 << ',' << Position_X << ',' << Position_Y - 50 << endl;
					storyboard << "  F,0," << LengthLN << ',' << LengthLN + 50 << ",0,1" << endl;
					storyboard << "  F,0," << LengthLN + 350 << ',' << LengthLN + 400 << "1,0" << endl;
					storyboard << "  S,0," << LengthLN << ',' <<  LengthLN + 400 << ',' << "0.4" << endl;

					storyboard << "Sprite,Background,Centre," << '"' << "SB/pd_hit200.png" << '"' << ",320,240" << endl;
					storyboard << HitsoundType <<  offset + TimingOD[OD][3] + 1 << ',' << offset + TimingOD[OD][2] + 1 << endl;
					storyboard << "  M,0," << LengthLN << ',' << LengthLN + 400 << ',' << Position_X << ',' << Position_Y - 50 << endl;
					storyboard << "  F,0," << LengthLN << ',' << LengthLN + 50 << ",0,1" << endl;
					storyboard << "  F,0," << LengthLN + 350 << ',' << LengthLN + 400 << "1,0" << endl;
					storyboard << "  S,0," << LengthLN << ',' << LengthLN + 400 << ',' << "0.4" << endl;

			// ----------------------------------------------------------------------------------------------

					storyboard << "Sprite,Background,Centre," << '"' << "SB/pd_hit100.png" << '"' << ",320,240" << endl;
					storyboard << HitsoundType <<  offset - TimingOD[OD][1] << ',' << offset - TimingOD[OD][2] - 1 << endl;
					storyboard << "  M,0," << LengthLN << ',' << LengthLN + 400 << ',' << Position_X << ',' << Position_Y - 50 << endl;
					storyboard << "  F,0," << LengthLN << ',' << LengthLN + 50 << ",0,1" << endl;
					storyboard << "  F,0," << LengthLN + 350 << ',' << LengthLN + 400 << "1,0" << endl;
					storyboard << "  S,0," << LengthLN << ',' << LengthLN + 400 << ',' << "0.4" << endl;

					storyboard << "Sprite,Background,Centre," << '"' << "SB/pd_hit100.png" << '"' << ",320,240" << endl;
					storyboard << HitsoundType <<  offset + TimingOD[OD][2] + 1 << ',' << offset + TimingOD[OD][1] + 1 << endl;
					storyboard << "  M,0," << LengthLN << ',' << LengthLN + 400 << ',' << Position_X << ',' << Position_Y - 50 << endl;
					storyboard << "  F,0," << LengthLN << ',' << LengthLN + 50 << ",0,1" << endl;
					storyboard << "  F,0," << LengthLN + 350 << ',' << LengthLN + 400 << "1,0" << endl;
					storyboard << "  S,0," << LengthLN << ',' << LengthLN + 400 << ',' << "0.4" << endl;

			// ----------------------------------------------------------------------------------------------

					storyboard << "Sprite,Background,Centre," << '"' << "SB/pd_hit50.png" << '"' << ",320,240" << endl;
					storyboard << HitsoundType <<  offset - TimingOD[OD][0] << ',' << offset - TimingOD[OD][1] - 1 << endl;
					storyboard << "  M,0," << LengthLN << ',' << LengthLN + 400 << ',' << Position_X << ',' << Position_Y - 50 << endl;
					storyboard << "  F,0," << LengthLN << ',' << LengthLN + 50 << ",0,1" << endl;
					storyboard << "  F,0," << LengthLN + 350 << ',' << LengthLN + 400 << "1,0" << endl;
					storyboard << "  S,0," << LengthLN << ',' << LengthLN + 400 << ',' << "0.4" << endl;

					storyboard << "Sprite,Background,Centre," << '"' << "SB/pd_hit50.png" << '"' << ",320,240" << endl;
					storyboard << HitsoundType <<  offset + TimingOD[OD][1] + 1 << ',' << offset + TimingOD[OD][0] + 1 << endl;
					storyboard << "  M,0," << LengthLN << ',' << LengthLN + 400 << ',' << Position_X << ',' << Position_Y - 50 << endl;
					storyboard << "  F,0," << LengthLN << ',' << LengthLN + 50 << ",0,1" << endl;
					storyboard << "  F,0," << LengthLN + 350 << ',' << LengthLN + 400 << "1,0" << endl;
					storyboard << "  S,0," << LengthLN << ',' << LengthLN + 400 << ',' << "0.4" << endl;
				}

	// ----------------------------------------------------------------------------------------------

				// Creation and determination of the Approaching Object.
				if (type < 6)
				{
					if (SV_Kiai2[KiaiCounter] == 0)
					{
						switch (ObjectType)
						{
							case 0: storyboard << "Sprite,Foreground,Centre," << '"' << "SB/Triangle.png" << '"' << ",320,240" << endl; break;		
							case 1: storyboard << "Sprite,Foreground,Centre," << '"' << "SB/Circle.png" << '"' << ",320,240" << endl; break;	
							case 2: storyboard << "Sprite,Foreground,Centre," << '"' << "SB/Cross.png" << '"' << ",320,240" << endl; break;	
							case 3: storyboard << "Sprite,Foreground,Centre," << '"' << "SB/Square.png" << '"' << ",320,240" << endl; break;	
							case 4: storyboard << "Sprite,Foreground,Centre," << '"' << "SB/ArrowUp.png" << '"' << ",320,240" << endl; break;	
							case 5: storyboard << "Sprite,Foreground,Centre," << '"' << "SB/ArrowRight.png" << '"' << ",320,240" << endl; break;	
							case 6: storyboard << "Sprite,Foreground,Centre," << '"' << "SB/ArrowDown.png" << '"' << ",320,240" << endl; break;	
							case 7: storyboard << "Sprite,Foreground,Centre," << '"' << "SB/ArrowLeft.png" << '"' << ",320,240" << endl; break;	
						}
					}
					else
					{
						switch (ObjectType)
						{
							case 0: storyboard << "Sprite,Foreground,Centre," << '"' << "SB/Triangle_Gold.png" << '"' << ",320,240" << endl; break;		
							case 1: storyboard << "Sprite,Foreground,Centre," << '"' << "SB/Circle_Gold.png" << '"' << ",320,240" << endl; break;	
							case 2: storyboard << "Sprite,Foreground,Centre," << '"' << "SB/Cross_Gold.png" << '"' << ",320,240" << endl; break;	
							case 3: storyboard << "Sprite,Foreground,Centre," << '"' << "SB/Square_Gold.png" << '"' << ",320,240" << endl; break;	
							case 4: storyboard << "Sprite,Foreground,Centre," << '"' << "SB/ArrowUp_Gold.png" << '"' << ",320,240" << endl; break;	
							case 5: storyboard << "Sprite,Foreground,Centre," << '"' << "SB/ArrowRight_Gold.png" << '"' << ",320,240" << endl; break;	
							case 6: storyboard << "Sprite,Foreground,Centre," << '"' << "SB/ArrowDown_Gold.png" << '"' << ",320,240" << endl; break;	
							case 7: storyboard << "Sprite,Foreground,Centre," << '"' << "SB/ArrowLeft_Gold.png" << '"' << ",320,240" << endl; break;	
						}
					}
				}
				else
				{
					if (SV_Kiai2[KiaiCounter] == 0)
					{
						switch (ObjectType)
						{
							case 0: storyboard << "Sprite,Foreground,Centre," << '"' << "SB/Triangle_LN.png" << '"' << ",320,240" << endl; break;		
							case 1: storyboard << "Sprite,Foreground,Centre," << '"' << "SB/Circle_LN.png" << '"' << ",320,240" << endl; break;	
							case 2: storyboard << "Sprite,Foreground,Centre," << '"' << "SB/Cross_LN.png" << '"' << ",320,240" << endl; break;	
							case 3: storyboard << "Sprite,Foreground,Centre," << '"' << "SB/Square_LN.png" << '"' << ",320,240" << endl; break;	
							case 4: storyboard << "Sprite,Foreground,Centre," << '"' << "SB/ArrowUp_LN.png" << '"' << ",320,240" << endl; break;	
							case 5: storyboard << "Sprite,Foreground,Centre," << '"' << "SB/ArrowRight_LN.png" << '"' << ",320,240" << endl; break;	
							case 6: storyboard << "Sprite,Foreground,Centre," << '"' << "SB/ArrowDown_LN.png" << '"' << ",320,240" << endl; break;	
							case 7: storyboard << "Sprite,Foreground,Centre," << '"' << "SB/ArrowLeft_LN.png" << '"' << ",320,240" << endl; break;	
						}
					}
					else
					{
						switch (ObjectType)
						{
							case 0: storyboard << "Sprite,Foreground,Centre," << '"' << "SB/Triangle_LN_Gold.png" << '"' << ",320,240" << endl; break;		
							case 1: storyboard << "Sprite,Foreground,Centre," << '"' << "SB/Circle_LN_Gold.png" << '"' << ",320,240" << endl; break;	
							case 2: storyboard << "Sprite,Foreground,Centre," << '"' << "SB/Cross_LN_Gold.png" << '"' << ",320,240" << endl; break;	
							case 3: storyboard << "Sprite,Foreground,Centre," << '"' << "SB/Square_LN_Gold.png" << '"' << ",320,240" << endl; break;	
							case 4: storyboard << "Sprite,Foreground,Centre," << '"' << "SB/ArrowUp_LN_Gold.png" << '"' << ",320,240" << endl; break;	
							case 5: storyboard << "Sprite,Foreground,Centre," << '"' << "SB/ArrowRight_LN_Gold.png" << '"' << ",320,240" << endl; break;	
							case 6: storyboard << "Sprite,Foreground,Centre," << '"' << "SB/ArrowDown_LN_Gold.png" << '"' << ",320,240" << endl; break;	
							case 7: storyboard << "Sprite,Foreground,Centre," << '"' << "SB/ArrowLeft_LN_Gold.png" << '"' << ",320,240" << endl; break;	
						}
					}
				}
				/*
				Pattern_Direction = 1/2/3: The pattern moves to the right.
				Pattern_Direction = 5/6/7: The pattern moves to the left.
				Position_LR = 0: Left.
				Position_LR = 1: Right.
				*/
				
				if (Pattern_Direction == 1 || Pattern_Direction == 2 || Pattern_Direction == 3)
				{
					Position_LR = 0;
				}
				else if (Pattern_Direction == 5 || Pattern_Direction == 6 || Pattern_Direction == 7)
				{
					Position_LR = 1;
				}

				/*
				Determining the Start Position of the Approaching Object and the movement Path
				The Movement will be determined by the Pattern direction and if the Target Objects are above or below the Center of the screen.
				The Movement also always turns in a curve to the target, it will move a straight way for 750ms and then turn slowly into the direction of the Hit Object.
				*/

	// ------------------------------ Start of Approaching Object Creation --------------------------

				if (Position_Y < 240)
				{	
					SPosition_Y = Position_Y + 540;

					if (Position_LR == 0)
					{
						storyboard << " M,0," << offset - floor(1500 * SV_Value) << ',' << offset - floor(750 * SV_Value) << ',' << Position_X - 60 << ',' << SPosition_Y << ',' << Position_X - 75 << ',' << SPosition_Y - 270 << endl;
						SOffset = offset - floor(750 * SV_Value);
						SPosition_X = Position_X - 75;
						SPosition_Y = Position_Y + 270;
						for (Dummy = 0; Dummy < 10; Dummy++)
						{
							storyboard << " M,0," <<SOffset << ',' << SOffset + floor(73 * SV_Value) << ',' << SPosition_X << ',' << SPosition_Y << ',' << SPosition_X + 3 + Dummy << ',' << SPosition_Y  - 26 << endl;
							SOffset = SOffset + floor(73 * SV_Value);
							SPosition_X = SPosition_X + 3 + Dummy;
							SPosition_Y = SPosition_Y - 27;
						}
						storyboard << " M,0," <<SOffset << ',' << offset << ',' << SPosition_X << ',' << SPosition_Y << ',' << Position_X << ',' << Position_Y << endl;
					}
					else
					{
						storyboard << " M,0," << offset - floor(1500 * SV_Value) << ',' << offset - floor(750 * SV_Value) << ',' << Position_X + 60 << ',' << SPosition_Y << ',' << Position_X + 75 << ',' << SPosition_Y - 270 << endl;
						SOffset = offset - floor(750 * SV_Value);
						SPosition_X = Position_X + 75;
						SPosition_Y = Position_Y + 270;
						for (Dummy = 0; Dummy < 10; Dummy++)
						{
							storyboard << " M,0," <<SOffset << ',' << SOffset + floor(73 * SV_Value) << ',' << SPosition_X << ',' << SPosition_Y << ',' << SPosition_X - 3 - Dummy << ',' << SPosition_Y  - 26 << endl;
							SOffset = SOffset + floor(73 * SV_Value);
							SPosition_X = SPosition_X - 3 - Dummy;
							SPosition_Y = SPosition_Y - 27;
						}
						storyboard << " M,0," <<SOffset << ',' << offset << ',' << SPosition_X << ',' << SPosition_Y << ',' << Position_X << ',' << Position_Y << endl;
					}
				}
				else
				{
					SPosition_Y = Position_Y - 540;

					if (Position_LR == 0)
					{
						storyboard << " M,0," << offset - floor(1500 * SV_Value) << ',' << offset - floor(750 * SV_Value) << ',' << Position_X - 60 << ',' << SPosition_Y << ',' << Position_X - 75 << ',' << SPosition_Y + 270 << endl;
						SOffset = offset - floor(750 * SV_Value);
						SPosition_X = Position_X - 75;
						SPosition_Y = Position_Y - 270;
						for (Dummy = 0; Dummy < 10; Dummy++)
						{
							storyboard << " M,0," <<SOffset << ',' << SOffset + floor(73 * SV_Value) << ',' << SPosition_X << ',' << SPosition_Y << ',' << SPosition_X + 3 + Dummy << ',' << SPosition_Y + 26 << endl;
							SOffset = SOffset + floor(73 * SV_Value);
							SPosition_X = SPosition_X + 3 + Dummy;
							SPosition_Y = SPosition_Y + 27;
						}
						storyboard << " M,0," <<SOffset << ',' << offset << ',' << SPosition_X << ',' << SPosition_Y << ',' << Position_X << ',' << Position_Y << endl;
					}
					else
					{
						storyboard << " M,0," << offset - floor(1500 * SV_Value) << ',' << offset - floor(750 * SV_Value) << ',' << Position_X + 60 << ',' << SPosition_Y << ',' << Position_X + 75 << ',' << SPosition_Y + 270 << endl;
						SOffset = offset - floor(750 * SV_Value);
						SPosition_X = Position_X + 75;
						SPosition_Y = Position_Y - 270;
						for (Dummy = 0; Dummy < 10; Dummy++)
						{
							storyboard << " M,0," <<SOffset << ',' << SOffset + floor(73 * SV_Value) << ',' << SPosition_X << ',' << SPosition_Y << ',' << SPosition_X - 3 - Dummy << ',' << SPosition_Y + 26 << endl;
							SOffset = SOffset + floor(73 * SV_Value);
							SPosition_X = SPosition_X - 3 - Dummy;
							SPosition_Y = SPosition_Y + 27;
						}
						storyboard << " M,0," <<SOffset << ',' << offset << ',' << SPosition_X << ',' << SPosition_Y << ',' << Position_X << ',' << Position_Y << endl;
					}
				}
				storyboard << " S,0," << offset << ",," << "0.525" << endl;

	// -------------------- End of Approaching Object Creation -------------------

	// -------------------- Creation of the Movement Trail -----------------------
				/*
				There are 4 cases:
				LR = will the note fly in a left or right curve.
				The 3rd and 4th case will just change the Y direction, so if the notes will fly from top to bottom, or bottom to top.

				There are always 2 For loops for each ot the 4 cases:
				The first loop will calculate the star rotation part for the "straight movement".
				The 2nd one will determine the star rotation part, for the "curve movement".
				*/
				if (type < 6)
				{
					int offset_Movement_Trail = offset + floor(15 * SV_Value);
					int Position_X_Movement_Trail;

					for (int Dummy2 = 0; Dummy2 < 30; Dummy2++)
					{
						if(SV_Kiai2[KiaiCounter] == 1)
						{
							storyboard << "Sprite,Background,Centre," << '"' << "SB/Star_Gold.png" << '"' << ",320,240" << endl;
						}
						else
						{
							switch (ObjectType)
							{
								case 0: storyboard << "Sprite,Background,Centre," << '"' << "SB/Star_Green.png" << '"' << ",320,240" << endl; break;		
								case 1: storyboard << "Sprite,Background,Centre," << '"' << "SB/Star_Red.png" << '"' << ",320,240" << endl; break;	
								case 2: storyboard << "Sprite,Background,Centre," << '"' << "SB/Star_Blue.png" << '"' << ",320,240" << endl; break;	
								case 3: storyboard << "Sprite,Background,Centre," << '"' << "SB/Star_Pink.png" << '"' << ",320,240" << endl; break;	
								case 4: storyboard << "Sprite,Background,Centre," << '"' << "SB/Star_Green.png" << '"' << ",320,240" << endl; break;	
								case 5: storyboard << "Sprite,Background,Centre," << '"' << "SB/Star_Red.png" << '"' << ",320,240" << endl; break;	
								case 6: storyboard << "Sprite,Background,Centre," << '"' << "SB/Star_Blue.png" << '"' << ",320,240" << endl; break;	
								case 7: storyboard << "Sprite,Background,Centre," << '"' << "SB/Star_Pink.png" << '"' << ",320,240" << endl; break;	
							}
						}
						offset_Movement_Trail = offset_Movement_Trail + floor(15 * SV_Value);

						if (Position_Y < 240)
						{	
						SPosition_Y = Position_Y + 540;
			
						if (Position_LR == 0)
						{
							SOffset = offset_Movement_Trail - floor(1500 * SV_Value);
							SPosition_X = Position_X - 55;
							Position_X_Movement_Trail = SPosition_X - 10 + rand() % 20;

							for (Dummy = 0; Dummy < 10; Dummy++)
							{
								storyboard << " M,0," <<SOffset << ',' << SOffset + floor(75 * SV_Value) << ',' << Position_X_Movement_Trail << ',' << SPosition_Y << ',' << Position_X_Movement_Trail - 2 << ',' << SPosition_Y  - 27 << endl;
								SOffset = SOffset + floor(75 * SV_Value);
								SPosition_X = SPosition_X - 2;
								SPosition_Y = SPosition_Y - 27;
								Position_X_Movement_Trail = SPosition_X - 10 + rand() % 20;
							}

							for (Dummy = 0; Dummy < 10 && SOffset + floor(80 * SV_Value) < offset; Dummy++)
							{
								storyboard << " M,0," <<SOffset << ',' << SOffset + floor(73 * SV_Value) << ',' << Position_X_Movement_Trail << ',' << SPosition_Y << ',' << Position_X_Movement_Trail + 3 + Dummy << ',' << SPosition_Y  - 26 << endl;
								SOffset = SOffset + floor(73 * SV_Value);
								SPosition_X = SPosition_X + 3 + Dummy;
								SPosition_Y = SPosition_Y - 27;
								Position_X_Movement_Trail = SPosition_X - 10 + rand() % 20;
							}
						}
						else
						{
							SOffset = offset_Movement_Trail - floor(1500 * SV_Value);
							SPosition_X = Position_X + 55;
							Position_X_Movement_Trail = SPosition_X - 10 + rand() % 20;

							for (Dummy = 0; Dummy < 10; Dummy++)
							{
								storyboard << " M,0," <<SOffset << ',' << SOffset + floor(75 * SV_Value) << ',' << Position_X_Movement_Trail << ',' << SPosition_Y << ',' << Position_X_Movement_Trail + 2 << ',' << SPosition_Y  - 27 << endl;
								SOffset = SOffset + floor(75 * SV_Value);
								SPosition_X = SPosition_X + 2;
								SPosition_Y = SPosition_Y - 27;
								Position_X_Movement_Trail = SPosition_X - 10 + rand() % 20;
							}
							for (Dummy = 0; Dummy < 10 && SOffset + floor(80 * SV_Value) < offset; Dummy++)
							{
								storyboard << " M,0," <<SOffset << ',' << SOffset + floor(73 * SV_Value) << ',' << Position_X_Movement_Trail << ',' << SPosition_Y << ',' << Position_X_Movement_Trail - 3 - Dummy << ',' << SPosition_Y  - 26 << endl;
								SOffset = SOffset + floor(73 * SV_Value);
								SPosition_X = SPosition_X - 3 - Dummy;
								SPosition_Y = SPosition_Y - 27;
								Position_X_Movement_Trail;
								Position_X_Movement_Trail = SPosition_X - 10 + rand() % 20;
							}
						}
					}
					else
					{
						SPosition_Y = Position_Y - 540;

						if (Position_LR == 0)
						{
							SOffset = offset_Movement_Trail - floor(1500 * SV_Value);
							SPosition_X = Position_X - 55;
							Position_X_Movement_Trail = SPosition_X - 10 + rand() % 20;

							for (Dummy = 0; Dummy < 10; Dummy++)
							{
								storyboard << " M,0," <<SOffset << ',' << SOffset + floor(75 * SV_Value) << ',' << Position_X_Movement_Trail << ',' << SPosition_Y << ',' << Position_X_Movement_Trail - 2 << ',' << SPosition_Y  + 27 << endl;
								SOffset = SOffset + floor(75 * SV_Value);
								SPosition_X = SPosition_X - 2;
								SPosition_Y = SPosition_Y + 27;
								Position_X_Movement_Trail = SPosition_X - 10 + rand() % 20;
							}
							for (Dummy = 0; Dummy < 10 && SOffset + floor(80 * SV_Value) < offset; Dummy++)
							{
								storyboard << " M,0," <<SOffset << ',' << SOffset + floor(73 * SV_Value) << ',' << Position_X_Movement_Trail << ',' << SPosition_Y << ',' << Position_X_Movement_Trail + 3 + Dummy << ',' << SPosition_Y + 26 << endl;
								SOffset = SOffset + floor(73 * SV_Value);
								SPosition_X = SPosition_X + 3 + Dummy;
								SPosition_Y = SPosition_Y + 27;
								Position_X_Movement_Trail = SPosition_X - 10 + rand() % 20;
							}
						}
						else
						{
							SOffset = offset_Movement_Trail - floor(1500 * SV_Value);
							SPosition_X = Position_X + 55;
							Position_X_Movement_Trail = SPosition_X - 10 + rand() % 20;

							for (Dummy = 0; Dummy < 10; Dummy++)
							{
								storyboard << " M,0," <<SOffset << ',' << SOffset + floor(75 * SV_Value) << ',' << Position_X_Movement_Trail << ',' << SPosition_Y << ',' << Position_X_Movement_Trail + 2 << ',' << SPosition_Y  + 27 << endl;
								SOffset = SOffset + floor(75 * SV_Value);
								SPosition_X = SPosition_X + 2;
								SPosition_Y = SPosition_Y + 27;
								Position_X_Movement_Trail = SPosition_X - 10 + rand() % 20;
							}
							for (Dummy = 0; Dummy < 10 && SOffset + floor(80 * SV_Value) < offset; Dummy++)
							{
								storyboard << " M,0," <<SOffset << ',' << SOffset + floor(73 * SV_Value) << ',' << Position_X_Movement_Trail << ',' << SPosition_Y << ',' << Position_X_Movement_Trail - 3 - Dummy << ',' << SPosition_Y + 26 << endl;
								SOffset = SOffset + floor(73 * SV_Value);
								SPosition_X = SPosition_X - 3 - Dummy;
								SPosition_Y = SPosition_Y + 27;
								Position_X_Movement_Trail = SPosition_X - 10 + rand() % 20;
							}
						}
					}
					storyboard << " S,0," << offset - floor(750 * SV_Value) << ",," << "0." << 10 + rand() % 15  << endl;
				}
	//------------------------------------------ End of Movement Trail Creation ------------------------------
			}
			else
			{
	// -------------------------------------------- Creation of the Longnote Trail --------------------------------------------------------------------
				int offset_Movement_Trail = offset;
				int offset_LN = endsyntax + floor(15 * SV_Value);
				int LN_Length = (endsyntax - offset) / 3;

				for (int Dummy2 = 0; Dummy2 < LN_Length; Dummy2++)
				{
					if(SV_Kiai2[KiaiCounter] == 1)
					{
						storyboard << "Sprite,Background,Centre," << '"' << "SB/LN_Gold.png" << '"' << ",320,240" << endl;
					}
					else
					{
						switch (ObjectType)
						{
							case 0: storyboard << "Sprite,Background,Centre," << '"' << "SB/LN_Green.png" << '"' << ",320,240" << endl; break;		
							case 1: storyboard << "Sprite,Background,Centre," << '"' << "SB/LN_Red.png" << '"' << ",320,240" << endl; break;	
							case 2: storyboard << "Sprite,Background,Centre," << '"' << "SB/LN_Blue.png" << '"' << ",320,240" << endl; break;	
							case 3: storyboard << "Sprite,Background,Centre," << '"' << "SB/LN_Pink.png" << '"' << ",320,240" << endl; break;	
							case 4: storyboard << "Sprite,Background,Centre," << '"' << "SB/LN_Green.png" << '"' << ",320,240" << endl; break;	
							case 5: storyboard << "Sprite,Background,Centre," << '"' << "SB/LN_Red.png" << '"' << ",320,240" << endl; break;	
							case 6: storyboard << "Sprite,Background,Centre," << '"' << "SB/LN_Blue.png" << '"' << ",320,240" << endl; break;	
							case 7: storyboard << "Sprite,Background,Centre," << '"' << "SB/LN_Pink.png" << '"' << ",320,240" << endl; break;	
						}
					}
					offset_Movement_Trail = offset_Movement_Trail + 3;

					if (Position_Y < 240)
					{	
						SPosition_Y = Position_Y + 540;

						if (Position_LR == 0)
						{
							SOffset = offset_Movement_Trail - floor(750 * SV_Value);
							storyboard << " M,0," << offset_Movement_Trail - floor(1500 * SV_Value) << ',' << SOffset << ',' << Position_X - 60 << ',' << SPosition_Y << ',' << Position_X - 75 << ',' << SPosition_Y - 270 << endl;
							SPosition_X = Position_X - 75;
							SPosition_Y = Position_Y + 270;
						//	storyboard << " M,0," << offset - floor(1500 * SV_Value) << ',' << SOffset << ',' << Position_X - 60 << ',' << SPosition_Y + 270 << ',' << SPosition_X << ',' << SPosition_Y << endl;
						
							for (Dummy = 0; Dummy < 10; Dummy++)
							{
								storyboard << " M,0," <<SOffset << ',' << SOffset + floor(73 * SV_Value) << ',' << SPosition_X << ',' << SPosition_Y << ',' << SPosition_X + 3 + Dummy << ',' << SPosition_Y  - 26 << endl;
								SOffset = SOffset + floor(73 * SV_Value);
								SPosition_X = SPosition_X + 3 + Dummy;
								SPosition_Y = SPosition_Y - 27;
							}
							storyboard << " M,0," <<SOffset << ',' << offset << ',' << SPosition_X << ',' << SPosition_Y << ',' << Position_X << ',' << Position_Y << endl;
						}
						else
						{
							SOffset = offset_Movement_Trail - floor(750 * SV_Value);
							storyboard << " M,0," << offset_Movement_Trail - floor(1500 * SV_Value) << ',' << SOffset << ',' << Position_X + 60 << ',' << SPosition_Y << ',' << Position_X + 75 << ',' << SPosition_Y - 270 << endl;
							SPosition_X = Position_X + 75;
							SPosition_Y = Position_Y + 270;
					//		storyboard << " M,0," << offset - floor(1500 * SV_Value) << ',' << SOffset << ',' << Position_X + 60 << ',' << SPosition_Y + 270 << ',' << SPosition_X << ',' << SPosition_Y << endl;
						
							for (Dummy = 0; Dummy < 10; Dummy++)
							{
								storyboard << " M,0," <<SOffset << ',' << SOffset + floor(73 * SV_Value) << ',' << SPosition_X << ',' << SPosition_Y << ',' << SPosition_X - 3 - Dummy << ',' << SPosition_Y  - 26 << endl;
								SOffset = SOffset + floor(73 * SV_Value);
								SPosition_X = SPosition_X - 3 - Dummy;
								SPosition_Y = SPosition_Y - 27;
							}
							storyboard << " M,0," <<SOffset << ',' << offset << ',' << SPosition_X << ',' << SPosition_Y << ',' << Position_X << ',' << Position_Y << endl;
						}
					}
					else
					{
						SPosition_Y = Position_Y - 540;

						if (Position_LR == 0)
						{
							SOffset = offset_Movement_Trail - floor(750 * SV_Value);
							storyboard << " M,0," << offset_Movement_Trail - floor(1500 * SV_Value) << ',' << SOffset << ',' << Position_X - 60 << ',' << SPosition_Y << ',' << Position_X - 75 << ',' << SPosition_Y + 270 << endl;
							SPosition_X = Position_X - 75;
							SPosition_Y = Position_Y - 270;
							//	storyboard << " M,0," << offset - floor(1500 * SV_Value) << ',' << SOffset << ',' << Position_X - 60 << ',' << SPosition_Y - 270 << ',' << SPosition_X  << ',' << SPosition_Y << endl;
						
							for (Dummy = 0; Dummy < 10; Dummy++)
							{
								storyboard << " M,0," <<SOffset << ',' << SOffset + floor(73 * SV_Value) << ',' << SPosition_X << ',' << SPosition_Y << ',' << SPosition_X + 3 + Dummy << ',' << SPosition_Y + 26 << endl;
								SOffset = SOffset + floor(73 * SV_Value);
								SPosition_X = SPosition_X + 3 + Dummy;
								SPosition_Y = SPosition_Y + 27;
							}
							storyboard << " M,0," <<SOffset << ',' << offset << ',' << SPosition_X << ',' << SPosition_Y << ',' << Position_X << ',' << Position_Y << endl;
						}
						else
						{
							SOffset = offset_Movement_Trail - floor(750 * SV_Value);
							storyboard << " M,0," << offset_Movement_Trail - floor(1500 * SV_Value) << ',' << SOffset << ',' << Position_X + 60 << ',' << SPosition_Y << ',' << Position_X + 75 << ',' << SPosition_Y + 270 << endl;
							SPosition_X = Position_X + 75;
							SPosition_Y = Position_Y - 270;
							//	storyboard << " M,0," << offset - floor(1500 * SV_Value) << ',' << SOffset << ',' << Position_X + 60 << ',' << SPosition_Y - 270 << ',' << SPosition_X << ',' << SPosition_Y << endl;
						
							for (Dummy = 0; Dummy < 10; Dummy++)
							{
								storyboard << " M,0," <<SOffset << ',' << SOffset + floor(73 * SV_Value) << ',' << SPosition_X << ',' << SPosition_Y << ',' << SPosition_X - 3 - Dummy << ',' << SPosition_Y + 26 << endl;
								SOffset = SOffset + floor(73 * SV_Value);
								SPosition_X = SPosition_X - 3 - Dummy;
								SPosition_Y = SPosition_Y + 27;
							}
							storyboard << " M,0," <<SOffset << ',' << offset << ',' << SPosition_X << ',' << SPosition_Y << ',' << Position_X << ',' << Position_Y << endl;
						}
					}
					storyboard << " S,0," << offset << ",," << "0.3" << endl;
				}
	// ---------------------- Creation of LN Ending ----------------------
				if (SV_Kiai2[KiaiCounter] == 0)
					{
						switch (ObjectType)
						{
							case 0: storyboard << "Sprite,Foreground,Centre," << '"' << "SB/Triangle_LN.png" << '"' << ",320,240" << endl; break;		
							case 1: storyboard << "Sprite,Foreground,Centre," << '"' << "SB/Circle_LN.png" << '"' << ",320,240" << endl; break;	
							case 2: storyboard << "Sprite,Foreground,Centre," << '"' << "SB/Cross_LN.png" << '"' << ",320,240" << endl; break;	
							case 3: storyboard << "Sprite,Foreground,Centre," << '"' << "SB/Square_LN.png" << '"' << ",320,240" << endl; break;	
							case 4: storyboard << "Sprite,Foreground,Centre," << '"' << "SB/ArrowUp_LN.png" << '"' << ",320,240" << endl; break;	
							case 5: storyboard << "Sprite,Foreground,Centre," << '"' << "SB/ArrowRight_LN.png" << '"' << ",320,240" << endl; break;	
							case 6: storyboard << "Sprite,Foreground,Centre," << '"' << "SB/ArrowDown_LN.png" << '"' << ",320,240" << endl; break;	
							case 7: storyboard << "Sprite,Foreground,Centre," << '"' << "SB/ArrowLeft_LN.png" << '"' << ",320,240" << endl; break;	
						}
					}
					else
					{
						switch (ObjectType)
						{
							case 0: storyboard << "Sprite,Foreground,Centre," << '"' << "SB/Triangle_LN_Gold.png" << '"' << ",320,240" << endl; break;		
							case 1: storyboard << "Sprite,Foreground,Centre," << '"' << "SB/Circle_LN_Gold.png" << '"' << ",320,240" << endl; break;	
							case 2: storyboard << "Sprite,Foreground,Centre," << '"' << "SB/Cross_LN_Gold.png" << '"' << ",320,240" << endl; break;	
							case 3: storyboard << "Sprite,Foreground,Centre," << '"' << "SB/Square_LN_Gold.png" << '"' << ",320,240" << endl; break;	
							case 4: storyboard << "Sprite,Foreground,Centre," << '"' << "SB/ArrowUp_LN_Gold.png" << '"' << ",320,240" << endl; break;	
							case 5: storyboard << "Sprite,Foreground,Centre," << '"' << "SB/ArrowRight_LN_Gold.png" << '"' << ",320,240" << endl; break;	
							case 6: storyboard << "Sprite,Foreground,Centre," << '"' << "SB/ArrowDown_LN_Gold.png" << '"' << ",320,240" << endl; break;	
							case 7: storyboard << "Sprite,Foreground,Centre," << '"' << "SB/ArrowLeft_LN_Gold.png" << '"' << ",320,240" << endl; break;	
						}
					}
					if (Position_Y < 240)
					{	
						SPosition_Y = Position_Y + 540;

						if (Position_LR == 0)
						{
							storyboard << " M,0," << endsyntax - floor(1500 * SV_Value) << ',' << endsyntax - floor(750 * SV_Value) << ',' << Position_X - 60 << ',' << SPosition_Y << ',' << Position_X - 75 << ',' << SPosition_Y - 270 << endl;
							SOffset = endsyntax - floor(750 * SV_Value);
							SPosition_X = Position_X - 75;
							SPosition_Y = Position_Y + 270;
							for (Dummy = 0; Dummy < 10; Dummy++)
							{
								storyboard << " M,0," <<SOffset << ',' << SOffset + floor(73 * SV_Value) << ',' << SPosition_X << ',' << SPosition_Y << ',' << SPosition_X + 3 + Dummy << ',' << SPosition_Y  - 26 << endl;
								SOffset = SOffset + floor(73 * SV_Value);
								SPosition_X = SPosition_X + 3 + Dummy;
								SPosition_Y = SPosition_Y - 27;
							}
							storyboard << " M,0," <<SOffset << ',' << offset << ',' << SPosition_X << ',' << SPosition_Y << ',' << Position_X << ',' << Position_Y << endl;
						}
						else
						{
							storyboard << " M,0," << endsyntax - floor(1500 * SV_Value) << ',' << endsyntax - floor(750 * SV_Value) << ',' << Position_X + 60 << ',' << SPosition_Y << ',' << Position_X + 75 << ',' << SPosition_Y - 270 << endl;
							SOffset = endsyntax - floor(750 * SV_Value);
							SPosition_X = Position_X + 75;
							SPosition_Y = Position_Y + 270;
							for (Dummy = 0; Dummy < 10; Dummy++)
							{
								storyboard << " M,0," <<SOffset << ',' << SOffset + floor(73 * SV_Value) << ',' << SPosition_X << ',' << SPosition_Y << ',' << SPosition_X - 3 - Dummy << ',' << SPosition_Y  - 26 << endl;
								SOffset = SOffset + floor(73 * SV_Value);
								SPosition_X = SPosition_X - 3 - Dummy;
								SPosition_Y = SPosition_Y - 27;
							}
							storyboard << " M,0," <<SOffset << ',' << offset << ',' << SPosition_X << ',' << SPosition_Y << ',' << Position_X << ',' << Position_Y << endl;
						}
					}
					else
					{
						SPosition_Y = Position_Y - 540;

						if (Position_LR == 0)
						{
							storyboard << " M,0," << endsyntax - floor(1500 * SV_Value) << ',' << endsyntax - floor(750 * SV_Value) << ',' << Position_X - 60 << ',' << SPosition_Y << ',' << Position_X - 75 << ',' << SPosition_Y + 270 << endl;
							SOffset = endsyntax - floor(750 * SV_Value);
							SPosition_X = Position_X - 75;
							SPosition_Y = Position_Y - 270;
							for (Dummy = 0; Dummy < 10; Dummy++)
							{
								storyboard << " M,0," <<SOffset << ',' << SOffset + floor(73 * SV_Value) << ',' << SPosition_X << ',' << SPosition_Y << ',' << SPosition_X + 3 + Dummy << ',' << SPosition_Y + 26 << endl;
								SOffset = SOffset + floor(73 * SV_Value);
								SPosition_X = SPosition_X + 3 + Dummy;
								SPosition_Y = SPosition_Y + 27;
							}
							storyboard << " M,0," <<SOffset << ',' << offset << ',' << SPosition_X << ',' << SPosition_Y << ',' << Position_X << ',' << Position_Y << endl;
						}
						else
						{
							storyboard << " M,0," << endsyntax - floor(1500 * SV_Value) << ',' << endsyntax - floor(750 * SV_Value) << ',' << Position_X + 60 << ',' << SPosition_Y << ',' << Position_X + 75 << ',' << SPosition_Y + 270 << endl;
							SOffset = endsyntax - floor(750 * SV_Value);
							SPosition_X = Position_X + 75;
							SPosition_Y = Position_Y - 270;
							for (Dummy = 0; Dummy < 10; Dummy++)
							{
								storyboard << " M,0," <<SOffset << ',' << SOffset + floor(73 * SV_Value) << ',' << SPosition_X << ',' << SPosition_Y << ',' << SPosition_X - 3 - Dummy << ',' << SPosition_Y + 26 << endl;
								SOffset = SOffset + floor(73 * SV_Value);
								SPosition_X = SPosition_X - 3 - Dummy;
								SPosition_Y = SPosition_Y + 27;
							}
							storyboard << " M,0," <<SOffset << ',' << offset << ',' << SPosition_X << ',' << SPosition_Y << ',' << Position_X << ',' << Position_Y << endl;
						}
					}
					storyboard << " S,0," << endsyntax << ",," << "0.525" << endl;

				}
			}
			previousLine = currentLine;
			Pattern_Direction_old = Pattern_Direction;
			last_offset = offset;
			Position_X_old = Position_X;
			ChordTimer--;
			if (type < 6)
			{
				offset_old = offset;
			}
			else
			{
				offset_old = endsyntax;
			}
			
// ---------------- End of Hit Objects creation ------------

		}

		if (currentLine == "[HitObjects]")
		{
			Code_Type = 1;
		}

		Line++;
		Percent = (Line * 100) / Line_Count; 
		cout<<"\rStoryboard Creation: "<<Percent<<"% Finished.";
// ---------------- End of getLine ---------
	}

	storyboard << "//Storyboard Sound Samples" << endl;

	system("cls");
	cout<<"Finished."<<endl;

	/*
	Storyboard Samples:
	
	Sprite Creation:
	Sprite,Foreground,Centre,"SB/Pointer.png",320,240

	Rotation:
	R,0,148308,148422,0,6.283

	Movement:
	M,0,148308,148877,111,114,339,268

	Stable Size:
	S,0,148308,,0.575

	Unstable Size:
	S,0,148308,148763,0.2144655,0.7545201

	Fade Out:
	F,0,18081,18308,1,0
	*/

	Read.close();

	return 0;
}
