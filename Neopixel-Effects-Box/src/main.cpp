#ifdef __AVR__
#include <avr/power.h>
#endif
#include <Arduino.h>
#define FASTLED_ALLOW_INTERRUPTS 0
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
#include <SPI.h>
#include "arduinoFFT.h"
#include <FastLED.h>
//https://github.com/FastLED/FastLED/wiki/Pixel-reference


//strip define
#define PIN RX //Neopixel data pin num
#define PixNum 295 //Count of pixels
#define MaxBright 150 // 1 - 255 in order to optimize power consuption you can reduc max possible brightness

//Define Audion analog reader pin
#define AudioPin A0

//Rotary encoder define
#define RotaryCLK D4 //clk
#define RotaryDT D1 //DT
#define RotaryBTN D3 //MS

//Display define
#define TFT_CS     D8
#define TFT_RST    D0  // you can also connect this to the Arduino reset
// in which case, set this #define pin to 0!
#define TFT_DC     D2
// Option 1 (recommended): must use the hardware SPI pins
// (for UNO thats sclk = 13 and sid = 11) and pin 10 must be
// an output. This is much faster - also required if you want
// to use the microSD card (see the image drawing example)


void RotaryChange();
void RotaryPush();

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);


int MaxBrightness = MaxBright; //* define on start
int MaxStripLength = PixNum; //* define on start
int Change = 0; //flag for interrupt trigger /0 No change /-1 Rotary down / +1 rotary up / 2 button

CRGBArray<PixNum> np;

int Zero = 0; //do not remove this


//----------------------------------------MENU

class Param
{
private:
	int max;
	int min;
	int step;
public:
	int val;
	String name;
	void SetVals(String name, int val, int min, int max, int step) {
		this->name = name;
		this->val = val;
		this->min = min;
		this->max = max;
		this->step = step;
	}
	void StepChange(int type) {
		//type - true - increment, false vice versa
		if (type == 1) {
			if (val + step <= max)
				val += step;
		}
		else if (type == -1) {
			if (val - step >= min)
				val -= step;
		}
	};
};

struct Effect
{
public:
	String name;
	int ParametersNum;
	Param Parameters[8]; //**4. Extend maximal number of paramaters if needed (if want to have definable more parameters as is already set)
};

#define NumberOfEffects 4 //**1. increment value if you want to add new effect 
struct MenuStruct {
	int EffectsNum = NumberOfEffects;
	Effect Effects[NumberOfEffects];
};

//actual menu position object. 
struct MenuPos {
	int CurrentEffect=2;
	int CurrentParameter=1;
	int Cursor = 0; //0-Effect, 1-Parameter, 2-InsideParameter
};
//In this object are stored all values about actual effect, actual parameter, menu position...
MenuPos MenuPosition;
MenuStruct Menu;



int SetMenu()
{
	//**2. add new effect name - new index value of your effect - x - will be used when calling values from Menu.Effects[x]
	Serial.begin(115200);
	Menu.Effects[0].name = "Snakes";
	Menu.Effects[1].name = "Sparkles";
	Menu.Effects[2].name = "Bouncing Snakes";
	Menu.Effects[3].name = "Spectrum Visual";
	Menu.EffectsNum = NumberOfEffects;
	

	//Definning
	Menu.Effects[0].Parameters[0].SetVals("<< UP", 1, 1, 1, 1);
	Menu.Effects[0].Parameters[1].SetVals("Speed", 50, 0, 1000, 5); //Name, Default value, Minimal value, Maximal value, Step value
	Menu.Effects[0].Parameters[2].SetVals("Brightness", 60, 1, MaxBrightness, 3);
	Menu.Effects[0].Parameters[3].SetVals("Strip Length", MaxStripLength, 1, MaxStripLength, 1);
	Menu.Effects[0].Parameters[4].SetVals("Snake Length", 10, 1, MaxStripLength - 1, 1);
	Menu.Effects[0].Parameters[5].SetVals("Snake Gap", 50, 1, MaxStripLength - 1, 1);
	Menu.Effects[0].ParametersNum = 5;

	Menu.Effects[1].Parameters[0].SetVals("<< UP", 1, 1, 1, 1);
	Menu.Effects[1].Parameters[1].SetVals("Speed", 50, 0, 1000, 5); //Name, Default value, Minimal value, Maximal value, Step value
	Menu.Effects[1].Parameters[2].SetVals("Brightness", 50, 1, MaxBrightness, 3);
	Menu.Effects[1].Parameters[3].SetVals("Min Bright", Menu.Effects[1].Parameters[2].val/2, 1, MaxBrightness-1, 3);
	Menu.Effects[1].Parameters[4].SetVals("Strip Length", MaxStripLength - 1, 1, MaxStripLength, 1);
	Menu.Effects[1].Parameters[5].SetVals("Color Num", 3, 1, 11, 1);
	Menu.Effects[1].Parameters[6].SetVals("Change time", 800, 1, 30000, 20);
	Menu.Effects[1].Parameters[7].SetVals("Sparkles Num", MaxStripLength / 2, 1, MaxStripLength - 1, 5);
	Menu.Effects[1].ParametersNum = 7; //-1

	Menu.Effects[2].Parameters[0].SetVals("<< UP",1,1,1,1); 
	Menu.Effects[2].Parameters[1].SetVals("Speed", 50, 0, 1000, 5); //Name, Default value, Minimal value, Maximal value, Step value
	Menu.Effects[2].Parameters[2].SetVals("Brightness", 70, 1, MaxBrightness, 3);
	Menu.Effects[2].Parameters[3].SetVals("Strip Length", MaxStripLength, 1, MaxStripLength, 1);
	Menu.Effects[2].Parameters[4].SetVals("Snake Length", 10, 1, MaxStripLength - 1, 1);
	Menu.Effects[2].Parameters[5].SetVals("Snake Count", 3, 1, 9, 1);
	Menu.Effects[2].ParametersNum = 5;

	Menu.Effects[3].Parameters[0].SetVals("<< UP", 1, 1, 1, 1);
	Menu.Effects[3].Parameters[1].SetVals("Color Speed", 3, 0, 100, 1); 
	Menu.Effects[3].Parameters[2].SetVals("Brightness", 70, 1, MaxBrightness, 3); 
	Menu.Effects[3].Parameters[3].SetVals("Strip Length", MaxStripLength, 1, MaxStripLength, 1); 
	Menu.Effects[3].Parameters[4].SetVals("Color Mode", 2, 1, 4, 1);  //how colors are distributed
	Menu.Effects[3].Parameters[5].SetVals("Peak Reducer", 100, 1, 1000, 5);  //good to adjust volume sensitivity
	Menu.Effects[3].Parameters[6].SetVals("Fade Speed", 60, 1, 1000, 5); //how fast band visualisation light fades 
	Menu.Effects[3].ParametersNum = 6;
	
	//**3. Like all effects in this function, define all parameters - values which want to be able change durring the run. 
	//**3. Define also ParametersNum
	//**3. 0 index is always "<< UP" - it is possibility to go on effect selection in menu
	//**3. actual value of parameter is as public in Menu.Effects[x].Parameters[y].val
	//**3.  x-index of your effect, y - array ID of your parameter

}

//in this function is processed every rotary push/change + updated display
//as argument use Change variable -1 rotary down, 1 rotary up, 2 push
//also consider MenuPosition value
void PrintDisplay() {
	uint16 GreyColor = 0xC618;
	Serial.print("Running Change ");
	Serial.println(Change);
	
	//on button push
	if (Change == 2) {
		switch (MenuPosition.Cursor)
		{
		case 2: {
			MenuPosition.Cursor = 1;
			break;
		}
		case 1: {
			if (MenuPosition.CurrentParameter == 0) //If cursor is on "<< UP"
				MenuPosition.Cursor = 0;
			else
				MenuPosition.Cursor = 2;
			break;
		}
		case 0: {
			MenuPosition.Cursor = 1;
			break;
		}
		default:
			Serial.println("Problem with Cursor");
			Serial.println(MenuPosition.Cursor);
			break;
		}
		
	}

	if (Change == 1 || Change == -1) {
		if (MenuPosition.Cursor == 2) {
				if (MenuPosition.CurrentParameter != -1)
					Menu.Effects[MenuPosition.CurrentEffect].Parameters[MenuPosition.CurrentParameter].StepChange(Change); //decrement
		}

		if (MenuPosition.Cursor == 1) {
			int ProposedChange = MenuPosition.CurrentParameter + Change;
			Serial.println(ProposedChange);
			if ((ProposedChange <= Menu.Effects[MenuPosition.CurrentEffect].ParametersNum) && (ProposedChange >= 0)) {
				MenuPosition.CurrentParameter = ProposedChange;
			}
		}
			if (MenuPosition.Cursor == 0) {
				int ProposedChange = MenuPosition.CurrentEffect + Change;
				if ((ProposedChange < Menu.EffectsNum) && (ProposedChange >= 0)) {
					MenuPosition.CurrentEffect = ProposedChange;
			}
		}
	}

	tft.setCursor(0, 10);
	tft.setTextSize(2);
	tft.setTextColor(GreyColor);

	//If button is pushed, change everything
	int ChangeDisplayLevel = 0;
	if (Change != 2) { ChangeDisplayLevel = MenuPosition.Cursor; };

	switch (ChangeDisplayLevel)
	{
	case 0: {
		if (MenuPosition.Cursor == 0)
			tft.setTextColor(ST7735_YELLOW);
		else tft.setTextColor(GreyColor);
		tft.setCursor(0, 10);
		tft.fillRect(0, 1, 160, 29, ST7735_BLACK);
		tft.println(Menu.Effects[MenuPosition.CurrentEffect].name);
	}
	case 1: {
		tft.setCursor(0, 36);
		tft.fillRect(0, 35, 160, 52, ST7735_BLACK);
		tft.setTextColor(GreyColor);
		if (MenuPosition.CurrentParameter > 0)
			tft.println(Menu.Effects[MenuPosition.CurrentEffect].Parameters[MenuPosition.CurrentParameter-1].name);
		else
			tft.println(" ");
		if (MenuPosition.Cursor == 1) {
			tft.setTextColor(ST7735_YELLOW);
			tft.fillTriangle(152, 61, 157, 56, 157, 66, ST7735_BLUE);}
		else tft.setTextColor(GreyColor);
		tft.println(Menu.Effects[MenuPosition.CurrentEffect].Parameters[MenuPosition.CurrentParameter].name);
		tft.setTextColor(GreyColor);
		if (MenuPosition.CurrentParameter < Menu.Effects[MenuPosition.CurrentEffect].ParametersNum)
			tft.println(Menu.Effects[MenuPosition.CurrentEffect].Parameters[MenuPosition.CurrentParameter+1].name);

	}
	case 2: {
		if (MenuPosition.Cursor == 2)
			tft.setTextColor(ST7735_YELLOW);
		else tft.setTextColor(GreyColor);
		tft.setCursor(0, 94);
		tft.fillRect(0, 93, 160, 35, ST7735_BLACK);
		tft.setTextSize(4);
		tft.println(Menu.Effects[MenuPosition.CurrentEffect].Parameters[MenuPosition.CurrentParameter].val);
	}
	default:
		break;
	}

	Change = 0;
}


//--------------------------------Snakes Effect



float CurrentSnakeColor[3];
int *SnakeValues = new int[MaxStripLength]; //calculated values for snake to avoid same calculations in every loop, used in 2 snake effects

void Snakes_SetColor();
void SnakesSet() { //**5. Define setup function for effect same like here
	FastLED.clear();
	Snakes_SetColor();
	SnakeValues[0] = 0;
}


void DefineSnakeValues(int Length, int Brightness, int* Snake); 
void SnakesLoop() { //**5. define loop function like here
	
int counter = 0;
int SnakeCounter = 1;
int GapCounter = 0;

	while (MenuPosition.CurrentEffect == 0) { //**6. code inside loop function must be wrapped by while loop watching current effect
		memmove(&np[1], &np[0], (Menu.Effects[0].Parameters[3].val - 1) * sizeof(CRGB)); //same as before but faster

		//Checking current length and Brightness, if a same are used in SnakeValues
		if ((SnakeValues[0] != Menu.Effects[0].Parameters[4].val) || (SnakeValues[1] != Menu.Effects[0].Parameters[2].val)) {
			DefineSnakeValues(Menu.Effects[0].Parameters[4].val, Menu.Effects[0].Parameters[2].val, SnakeValues);
		}


		if (GapCounter >= Menu.Effects[0].Parameters[5].val) {
			GapCounter = 0;
			SnakeCounter = 1;
			np[0].setRGB(0, 0, 0);
		}
		else if (GapCounter >= 1) {
			np[0].setRGB(0, 0, 0);
			GapCounter++;
		};

		if (SnakeCounter == 1) {
			Snakes_SetColor();
			np[0].setRGB(CurrentSnakeColor[0]*SnakeValues[1], CurrentSnakeColor[1]*SnakeValues[1], CurrentSnakeColor[2]*SnakeValues[1]);
			SnakeCounter++;
		}
		else if ((SnakeCounter <= Menu.Effects[0].Parameters[4].val) && (SnakeCounter > 1)) {
			np[0].setRGB(CurrentSnakeColor[0] * SnakeValues[SnakeCounter], CurrentSnakeColor[1] * SnakeValues[SnakeCounter], CurrentSnakeColor[2] * SnakeValues[SnakeCounter]);
			SnakeCounter++;
		}
		else if (SnakeCounter > Menu.Effects[0].Parameters[4].val) {
			np[0].setRGB(0, 0, 0);
			SnakeCounter = 0;
			GapCounter = 1;
		};

		FastLED.show();
		if (Change != 0) { //**7. Add this condition before end of while loop, to trigger rotary encoder change flag
			PrintDisplay();	}//**7.
		
		delay(Menu.Effects[0].Parameters[1].val);
	}
}


void Snakes_SetColor() {
	CurrentSnakeColor[0] = random(1,11)*0.1; CurrentSnakeColor[1] = random(1, 11)*0.1; CurrentSnakeColor[2] = random(1, 11)*0.1;
}

double CalculateFadeModifier(int NumOfPixels);
void DefineSnakeValues(int Length, int Brightness, int* Snake) {
	double FadeModifier = CalculateFadeModifier(Length);
	double CurrentValue = Brightness;
	Snake[0] = Length;
	Snake[1] = Brightness;
	for (int i = 2; i <= Length; i++) {
		CurrentValue = CurrentValue * FadeModifier;
		int val = int(CurrentValue);
		Snake[i] = val;
	}


}


//magic function to calculate constant number. It is for cumulative multiply value from maximal brightness value_
//of snake until end of length of snake based on its length
//
double CalculateFadeModifier(int NumOfPixels) {
	if (NumOfPixels < 46) {
		//Magic formula with bulgarian constants 
		return 0.1222*log(NumOfPixels) + 0.4868;
	}
	//increasing is now linear instead of logarithmic with molvanian constant
	else if ((NumOfPixels <101) && (NumOfPixels >= 46)) {
		return 0.95 + ((NumOfPixels - 45)*0.000545454);
	}
	else if ((NumOfPixels < 201) && (NumOfPixels >= 101)) {
		return 0.98 + ((NumOfPixels - 100)*0.0001);
	}
	else {
		return 0.99;
	}

}


//---------------------------------Snakes 2



class BouncingSnake {
public:
	bool Direction = true; //True - from 0 to End
	float Position = 0;
	float Speed = 1;
	int &length = Menu.Effects[2].Parameters[4].val;
	float Color[3]; // modifier of current brightness -> x*Brightness

	void Inc() {
		if (Direction)
			Position += Speed;
		else
			Position -= Speed;
		if ((int(Position) >= Menu.Effects[2].Parameters[3].val) || (int(Position) <= 0)) {
			Direction = !Direction;
		}
	};
};



BouncingSnake s[9];

void SetBouncingSnakes() {
	FastLED.clear();
	SnakeValues[0] = 0;


	s[0].Position = 50; s[0].Speed = 0.8; 
	s[0].Color[0] = 1; s[0].Color[1] = 0.5; s[0].Color[2] = 0;
	s[1].Position = 20; s[1].Speed = 1.5; 
	s[1].Color[0] = 0; s[1].Color[1] = 1; s[1].Color[2] = 0;
	s[2].Position = 1; s[2].Speed = 1;
	s[2].Color[0] = 0; s[2].Color[1] = 0; s[2].Color[2] = 1;
	s[3].Position = 10; s[3].Speed = 0.7;
	s[3].Color[0] = 0; s[3].Color[1] = 1; s[3].Color[2] = 1;
	s[4].Position = 30; s[4].Speed = 1.2; 
	s[4].Color[0] = 1; s[4].Color[1] = 1; s[4].Color[2] = 0;
	s[5].Position = 20; s[5].Speed = 1.1; 
	s[5].Color[0] = 1; s[5].Color[1] = 0; s[5].Color[2] = 1;
	s[6].Position = 10; s[6].Speed = 0.9; 
	s[6].Color[0] = 0.5; s[6].Color[1] = 1; s[6].Color[2] = 0.5;
	s[7].Position = 5; s[7].Speed = 0.95; 
	s[7].Color[0] = 0.8; s[7].Color[1] = 0.5; s[7].Color[2] = 0.2;
	s[8].Position = 15; s[8].Speed = 1.35; 
	s[8].Color[0] = 0.5; s[8].Color[1] = 0.9; s[8].Color[2] = 0.2;
}



void BouncingSnakesLoop() {

	while (MenuPosition.CurrentEffect == 2) {
		FastLED.clearData();
		//Checking current length and Brightness, if a same are used in SnakeValues (detecting changes in settings)
		if ((SnakeValues[0] != Menu.Effects[2].Parameters[4].val) || (SnakeValues[1] != Menu.Effects[2].Parameters[2].val)) {
			DefineSnakeValues(Menu.Effects[2].Parameters[4].val, Menu.Effects[2].Parameters[2].val, SnakeValues);
		}

		for (int j = 0; j < Menu.Effects[2].Parameters[5].val; j++) {
			s[j].Inc();
			int pos = int(s[j].Position);
			if (s[j].Direction) {
				for (int i = 1; i < Menu.Effects[2].Parameters[4].val + 1; i++) {
					if ((pos - i) < 0) {
						np[pos + i].setRGB(np[pos + i][0] + s[j].Color[0]*SnakeValues[i], np[pos + i][1] + s[j].Color[1] * SnakeValues[i], np[pos + i][2] + s[j].Color[2] * SnakeValues[i]);
					}
					else {
						np[pos - i].setRGB(np[pos - i][0] + s[j].Color[0] * SnakeValues[i], np[pos - i][1] + s[j].Color[1] * SnakeValues[i], np[pos - i][2] + s[j].Color[2] * SnakeValues[i]);
					}
				}
			}
			else {
				for (int i = 1; i < Menu.Effects[2].Parameters[4].val + 1; i++) {
					if ((pos + i) > Menu.Effects[2].Parameters[3].val)
					{
						np[pos - i].setRGB(np[pos - i][0] + s[j].Color[0] * SnakeValues[i], np[pos - i][1] + s[j].Color[1] * SnakeValues[i], np[pos - i][2] + s[j].Color[2] * SnakeValues[i]);
					}
					else
					{
						np[pos + i].setRGB(np[pos + i][0] + s[j].Color[0] * SnakeValues[i], np[pos + i][1] + s[j].Color[1] * SnakeValues[i], np[pos + i][2] + s[j].Color[2] * SnakeValues[i]);
					}
				}
			}
		}
		FastLED.show();
		if (Change != 0) {
			PrintDisplay();
		}
		delay(Menu.Effects[2].Parameters[1].val);
	}

}



//-----------------------------------Sparkles


#define FadeByFactor_Sparkles 1 //(x/255*100)
#define LoopToCheckCount 2


CRGB SetPixel(int Index) {
	CRGB pixel;
	int Num = random8(Menu.Effects[1].Parameters[3].val, Menu.Effects[1].Parameters[2].val);
	switch (Index) {
		//case 1: {pixel.setRGB(random8(MinBright, MaxBright), 0, 0); break; }
	case 1: {pixel.setRGB(Num, Num / 2, Num / 3); break; }
	case 2: {pixel.setRGB(Num / 2, Num, Num / 3); break; }
	case 3: {pixel.setRGB(Num / 3, Num / 2, Num); break; }
	case 4: {pixel.setRGB(Num, Num / 2, Num / 3); break; }
	case 5: {pixel.setRGB(Num, Num / 3, Num); break; }
	case 6: {pixel.setRGB(Num / 3, Num, Num); break; }
	case 7: {pixel.setRGB(Num / 2, Num, 0); break; }
	case 8: {pixel.setRGB(Num / 2, 0, Num); break; }
	case 9: {pixel.setRGB(0, Num / 2, Num); break; }
	case 10: {pixel.setRGB(Num, Num / 2, 0); break; }//
	case 11: {pixel.setRGB(Num, 0, Num / 2); break; }
	case 12: {pixel.setRGB(0, Num, Num / 2); break; }
	default: pixel.setRGB(30, 30, 30);
	}
	return pixel;
}

int GetRandBlackPix(CRGB np[]);
int GetRandColoredPix(CRGB np[]);
int GetColorPixelNum(CRGB np[]);
void SparklesSet() {

	Serial.println("SparklesSet");
	FastLED.clear();
	for (int i = 0; i < Menu.Effects[1].Parameters[7].val; i++) {
		np[GetRandBlackPix(np)] = SetPixel(1);
	}
}


void SparklesLoop() {
	int CurrentColorIndex = 1;
	bool CreationAllowed = true;
	int ChangeColorCounter = 0;
	int c = 0;

	while (MenuPosition.CurrentEffect == 1){
	np(0, PixNum - 1).fadeToBlackBy(FadeByFactor_Sparkles); //bulk function apply

	//checking pixels function
	if (c++ == LoopToCheckCount) {
		int MissingPixels = Menu.Effects[1].Parameters[7].val - GetColorPixelNum(np);
		if (MissingPixels > 0) {
			abs(MissingPixels);
			for (int j = 0; j < MissingPixels; j++)
				np[GetRandBlackPix(np)] = SetPixel(CurrentColorIndex);
		}

		c = 0;
	};

	if (ChangeColorCounter++ >= Menu.Effects[1].Parameters[6].val) {
		if (CurrentColorIndex >= Menu.Effects[1].Parameters[5].val)
			CurrentColorIndex = 1;
		else
		{
			CurrentColorIndex++;
		}
		ChangeColorCounter = 0;
	};
	FastLED.show();
	if (Change != 0) {
		PrintDisplay();
	}
	delay(Menu.Effects[1].Parameters[1].val);
	}

}

int GetRandBlackPix(CRGB np[]) {
	while (1) {
		int Pix = random(Menu.Effects[1].Parameters[4].val);
		if (!(np[Pix])) {
			return Pix;
		}
	}
};

int GetRandColoredPix(CRGB np[]) {
	while (1) {
		int Pix = random(Menu.Effects[1].Parameters[7].val);
		if (np[Pix]) {
			return Pix;
		}
	}
};

int GetColorPixelNum(CRGB np[]) {
	int c = 0;
	for (int i = 0; i < Menu.Effects[1].Parameters[4].val; i++) {
		if (np[i]) {
			c++;
		}
	}
	return c;
}

//---------------------------------SpectrumVisualisator


#define Num_Bands 7
#define SpeedDivider 10.0

class BandData {
public:
	int LowerFreq; //on 256 samle rate, 1 represents 25Hz
	int UpperFreq; //on 256 samle rate, 1 represents 25Hz
	int HSVColor;
	double ValueMultiplier; //Some bands have all time lower values, for better I increased their value
	int ActualValue;
	void BandDataSet(int LowerFreq, int UpperFreq, int HSVColor, double ValueMultiplier) {
		this->LowerFreq = LowerFreq;
		this->UpperFreq = UpperFreq;
		this->HSVColor = HSVColor;
		this->ValueMultiplier = ValueMultiplier;
		ActualValue = 0;
	};
	void SetVal(int val) {

		ActualValue = (ActualValue < int(val*ValueMultiplier)) ? int(val*ValueMultiplier) : ActualValue;
	}

};
BandData Bands[Num_Bands];



// https://github.com/kosme/arduinoFFT, in IDE, Sketch, Include Library, Manage Library, then search for FFT
arduinoFFT FFT = arduinoFFT();



#define SAMPLES 256             
#define SAMPLING_FREQUENCY 5000 
#define amplitude 50
unsigned int sampling_period_us;
unsigned long microseconds;
byte peak[] = { 0,0,0,0,0,0,0 };
double vReal[SAMPLES];
double vImag[SAMPLES];
unsigned long newTime, oldTime;

void SpectrumVisualizerSetup() {
	
	FastLED.clear();
	FastLED.show();

	sampling_period_us = round(1000000 * (1.0 / SAMPLING_FREQUENCY));

	//I set frequency band values just to get visualy nice results
	Bands[0].BandDataSet(2, 2, 0, 1); 
	Bands[1].BandDataSet(3, 4, 30, 1);
	Bands[2].BandDataSet(5, 7, 60, 1.4);
	Bands[3].BandDataSet(7, 9, 90, 1.6);
	Bands[4].BandDataSet(10, 13, 120, 1.8);
	Bands[5].BandDataSet(14, 18, 150, 2);
	Bands[6].BandDataSet(19, 128, 180, 2.2);

	FastLED.setBrightness(Menu.Effects[3].Parameters[2].val);
}



void SpectrumVisualizerLoop() {
	int ColumnsSplit;
	double Spectrum_ColorChangeCounter = 0;
	int Spectrum_ColorChangeCounter_int;

	while (MenuPosition.CurrentEffect == 3) {

		FastLED.setBrightness(Menu.Effects[3].Parameters[2].val);


		Spectrum_ColorChangeCounter += double(Menu.Effects[3].Parameters[1].val / SpeedDivider);
		Spectrum_ColorChangeCounter_int = int(Spectrum_ColorChangeCounter);
		//---computing vales
		for (int i = 0; i < SAMPLES; i++) {
			newTime = micros() - oldTime;
			oldTime = newTime;
			vReal[i] = analogRead(AudioPin); // A conversion takes about 1mS on an ESP8266
			vImag[i] = 0;
			while (micros() < (newTime + sampling_period_us)) { /* do nothing to wait */ }
		}
		FFT.Windowing(vReal, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
		FFT.Compute(vReal, vImag, SAMPLES, FFT_FORWARD);
		FFT.ComplexToMagnitude(vReal, vImag, SAMPLES);

		for (int i = 0; i < Num_Bands; i++) { Bands[i].ActualValue = 0; };

		for (int i = 2; i < SAMPLES; i++) { // Don't use sample 0 and only first SAMPLES/2 are usable. Each array eleement represents a frequency and its value the amplitude.
			if (vReal[i] > 200) { // Add a crude noise filter, 4 x amplitude or more

								  //add values in correct frequency range to correct frequency Band
				for (int j = 0; j < Num_Bands; j++) {
					if (i >= Bands[j].LowerFreq && i <= Bands[j].UpperFreq) {
						Bands[j].SetVal((int)vReal[i]);
						//break;
					}
				}

				//if (i <= 2)             displayBand(0, (int)vReal[i] / amplitude); //
				//if (i >2 && i <= 4)  displayBand(1, (int)vReal[i] / amplitude); // 
				//if (i >4 && i <= 8)  displayBand(2, (int)vReal[i] / amplitude); // 
				//if (i >8 && i <= 16)  displayBand(3, (int)vReal[i] / amplitude); // 
				//if (i >16 && i <= 32)  displayBand(4, (int)vReal[i] / amplitude); // 1000Hz
				//if (i >32 && i <= 64) displayBand(5, (int)vReal[i] / amplitude); // 2000Hz
				//if (i >64 && i <= 128) displayBand(6, (int)vReal[i] / amplitude); // 4000Hz
				////if (i >64 && i <= 127) displayBand(6, (int)vReal[i] / amplitude); // 8000Hz
				//Serial.println("");

				//Serial.println(i);
			}


		}
		ColumnsSplit = Menu.Effects[3].Parameters[3].val / Num_Bands;
		int BandCenter, Vall, BandTopValue, BandBottomValue;
		if (Menu.Effects[3].Parameters[4].val == 1)
		{
			for (int j = 0; j < Num_Bands; j++) {
				BandCenter = ((j)*ColumnsSplit) + (ColumnsSplit / 2);

				Bands[j].ActualValue = int(Bands[j].ActualValue / Menu.Effects[3].Parameters[5].val);
				Vall = (Bands[j].ActualValue < (ColumnsSplit / 2 + 1)) ? Bands[j].ActualValue : (ColumnsSplit / 2 + 1);
				BandTopValue = (BandCenter + Vall) >= Menu.Effects[3].Parameters[3].val ? Menu.Effects[3].Parameters[3].val - 1 : BandCenter + Vall;
				BandBottomValue = (BandCenter - Vall) <= 0 ? 0 : BandCenter - Vall;

				np(BandCenter, BandTopValue).fill_gradient(CHSV(Bands[j].HSVColor, 255, 255), CHSV(Bands[j].HSVColor, 255, 255), SHORTEST_HUES);
				np(BandCenter, BandBottomValue).fill_gradient(CHSV(Bands[j].HSVColor, 255, 255), CHSV(Bands[j].HSVColor, 255, 255), SHORTEST_HUES);


			}
		}



		if (Menu.Effects[3].Parameters[4].val == 2)
		{
			for (int j = 0; j < Num_Bands; j++) {
				BandCenter = ((j)*ColumnsSplit) + (ColumnsSplit / 2);
				Bands[j].ActualValue = int(Bands[j].ActualValue / Menu.Effects[3].Parameters[5].val);
				Vall = (Bands[j].ActualValue < (ColumnsSplit / 2 + 1)) ? Bands[j].ActualValue : (ColumnsSplit / 2 + 1);
				BandTopValue = (BandCenter + Vall) >= Menu.Effects[3].Parameters[3].val ? Menu.Effects[3].Parameters[3].val - 1 : BandCenter + Vall;
				BandBottomValue = (BandCenter - Vall) <= 0 ? 0 : BandCenter - Vall;

				np(BandCenter, BandTopValue).fill_gradient(CHSV(BandCenter + Spectrum_ColorChangeCounter_int, 255, 255), CHSV(BandTopValue + Spectrum_ColorChangeCounter_int, 255, 255), SHORTEST_HUES);
				np(BandCenter, BandBottomValue).fill_gradient(CHSV(BandCenter + Spectrum_ColorChangeCounter_int, 255, 255), CHSV(BandBottomValue + Spectrum_ColorChangeCounter_int, 255, 255), SHORTEST_HUES);
			}
		}

		if (Menu.Effects[3].Parameters[4].val == 3)
		{
			for (int j = 0; j < Num_Bands; j++) {
				BandCenter = ((j)*ColumnsSplit) + (ColumnsSplit / 2);
				Bands[j].ActualValue = int(Bands[j].ActualValue / Menu.Effects[3].Parameters[5].val);
				Vall = (Bands[j].ActualValue < (ColumnsSplit / 2 + 1)) ? Bands[j].ActualValue : (ColumnsSplit / 2 + 1);
				BandTopValue = (BandCenter + Vall) >= Menu.Effects[3].Parameters[3].val ? Menu.Effects[3].Parameters[3].val - 1 : BandCenter + Vall;
				BandBottomValue = (BandCenter - Vall) <= 0 ? 0 : BandCenter - Vall;

				np(BandCenter, BandTopValue).fill_gradient(CHSV(BandCenter + Spectrum_ColorChangeCounter_int, 255, 255), CHSV(BandTopValue + Spectrum_ColorChangeCounter_int, 255, 255), SHORTEST_HUES);
				np(BandCenter, BandBottomValue).fill_gradient(CHSV(BandCenter + Spectrum_ColorChangeCounter_int, 255, 255), CHSV(BandBottomValue + Spectrum_ColorChangeCounter_int, 255, 255), SHORTEST_HUES);
			}
		}

		if (Menu.Effects[3].Parameters[4].val == 4)
		{
			for (int j = 0; j < Num_Bands; j++) {
				BandCenter = ((j)*ColumnsSplit) + (ColumnsSplit / 2);
				Bands[j].ActualValue = int(Bands[j].ActualValue / Menu.Effects[3].Parameters[5].val);
				Vall = (Bands[j].ActualValue < (ColumnsSplit / 2 + 1)) ? Bands[j].ActualValue : (ColumnsSplit / 2 + 1);
				BandTopValue = (BandCenter + Vall) >= Menu.Effects[3].Parameters[3].val ? Menu.Effects[3].Parameters[3].val - 1 : BandCenter + Vall;
				BandBottomValue = (BandCenter - Vall) <= 0 ? 0 : BandCenter - Vall;

				np(BandCenter, BandTopValue).fill_gradient(CHSV(Spectrum_ColorChangeCounter_int, 255, 255), CHSV(Spectrum_ColorChangeCounter_int, 255, 255), SHORTEST_HUES);
				np(BandCenter, BandBottomValue).fill_gradient(CHSV(Spectrum_ColorChangeCounter_int, 255, 255), CHSV(Spectrum_ColorChangeCounter_int, 255, 255), SHORTEST_HUES);
			}
		}


		
		FastLED.show();
		np.fadeToBlackBy(Menu.Effects[3].Parameters[6].val);
		
		Spectrum_ColorChangeCounter = (Spectrum_ColorChangeCounter >= 255) ? 0 : Spectrum_ColorChangeCounter + Menu.Effects[3].Parameters[1].val / SpeedDivider;
		if (Change != 0) {
			PrintDisplay();
		}
		delay(10); //Must be atleast 10 otherwise you will get reboots

	}
	
}



//---------------------------------Main loop

void setup() {
	SetMenu();
	pinMode(RotaryCLK, INPUT_PULLUP);
	pinMode(RotaryDT, INPUT_PULLUP);
	pinMode(RotaryBTN, INPUT_PULLUP);
	attachInterrupt(digitalPinToInterrupt(RotaryCLK), RotaryChange, FALLING);
	attachInterrupt(digitalPinToInterrupt(RotaryBTN), RotaryPush, FALLING);
	FastLED.addLeds<NEOPIXEL, PIN>(np, MaxStripLength);
	FastLED.setBrightness(MaxBrightness);
	Serial.println("Leds Created");
	tft.initR(INITR_BLACKTAB);   // initialize a ST7735S chip, black tab
	tft.fillScreen(ST7735_BLACK);
	tft.setRotation(1);
	
	tft.setTextWrap(false);
	tft.fillScreen(ST7735_BLACK);
	tft.fillRect(0, 30, 160, 4, ST7735_RED);
	tft.fillRect(0, 88, 160, 4, ST7735_RED);
		
	PrintDisplay();
}

 
void loop() {
	//**8. Add new case for your effect array number

	switch (MenuPosition.CurrentEffect)
	{
	case 0: {
		SnakesSet();
		SnakesLoop(); break;}
	case 1:{
		SparklesSet();
		SparklesLoop(); break; }
	case 2:{
		SetBouncingSnakes();
		BouncingSnakesLoop(); break; }
	case 3: {
		SpectrumVisualizerSetup();
		SpectrumVisualizerLoop(); break; }
	//case x:	//**9. Define new case for your effect. Thats all
		   //SetNewEffectFunction();
		   //NewEffectLoop(); in this function use: while (MenuPosition.CurrentEffect == x){ your code}
		   // Add also to loop trigger for interrup flag : 	if (Change != 0) {PrintDisplay();} - see how is implemented in other effects
	default:
		break;
	}
	
}

void RotaryChange()
{
	bool StateA, StateB;

	StateB = digitalRead(RotaryDT);
	StateA = digitalRead(RotaryCLK);
	if ((StateA == 0) && (StateB == 1))
		Change = 1; //rotary up

	else if
		((StateA == 0) && (StateB == 0))
		Change = -1; //rotary down

	else {
		return;
	}

	Serial.print("Rottary changed ");
	Serial.println(Change);
}

void RotaryPush() {
	Change = 2;
	Serial.println("Button");
}
