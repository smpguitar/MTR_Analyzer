
#include <EncoderButton.h>   //https://github.com/Stutchbury/EncoderButton/blob/main/README.md
#include <LiquidCrystal_I2C.h>
#include <si5351.h>
#include "Wire.h"


//instantiation
LiquidCrystal_I2C lcd(0x27, 16, 2);
EncoderButton button(3, 2, 4);
Si5351 si5351;

//unchanging variables
const int band [] = {20, 30, 40, 80};
const unsigned long increment [] = {50, 100, 500};
const String mode [] = {"Automatic", "Manual"};
const String var [] = {"Mode", "Band", "Step"};

//Variables for state
int index = 0;  //value of encoder
bool clickSubmit = false;  //when encoder clicked it sets true
String param = "menu"; //parameter being adjusted

//array sizes
int varSize = sizeof var / (sizeof var[0] + 1);
int modeSize = sizeof mode/ (sizeof mode[0] + 1);
int bandSize = (sizeof band - 1)/ (sizeof band[0]);
int indexSize = sizeof increment / (sizeof increment[0]+1) ;  //number of elements in array

//user selected parameter variables
bool modeSelected = true;  //if true automatic if false manual.
int bandSelected = 20;  // bandSelected by user
long FreqStep = 50;  // FreqStep elected by user
String state = "menu";  //UI element selected by user

int corr = -4350000; // this is the correction factor for the Si5351, use calibration sketch to find value.

volatile unsigned long Freq = 10000000ULL; // current Frequency

//SWR parameters needed:
  int FWD; // create forward sensor reading.  read the input on analog pin 0:
  int REV; //create reverse reading. read the input on analong pin 1
  long FwdVoltage;
  long RevVoltage;
  float swrValue;  //float SWR Value


void setup() {
Serial.begin(9600); 
button.setEncoderHandler(onTurn); 
button.setClickHandler(onClick);

  pinMode(FWD, INPUT);
  pinMode(REV, INPUT);

  analogReference(DEFAULT); //set reference to internal 5V source

  
// Initialize the display
  lcd.init();                    
  lcd.backlight();  
  lcd.begin(16, 2);  
  lcd.setCursor(3,0);   // Column , Row
  lcd.print("KF5RY MTR");
  lcd.setCursor(3, 1);  // Column , Row
  lcd.print("Analyzer");
  
  delay(2000);
  lcd.clear();
  si5351.init(SI5351_CRYSTAL_LOAD_8PF, 0, corr);
  si5351.set_freq(Freq, SI5351_CLK0);
}

void loop() {
button.update();

if (state == "menu" && param == "menu"){
lcdMenu();
} else if (state == "param" && param == "step"){
  setFreqStep();
  }else if (state == "param" && param == "band"){
    setMeters();
    }else if (state == "param" && param == "mode"){
    setMode();
    }
//si5351.set_freq(Freq, SI5351_CLK0);

}

//Functions
//input device functions
int onTurn(EncoderButton& eb){
 lcd.clear();
 index = index + abs(eb.increment()); 
 index = index;
  } 

void onClick(EncoderButton& eb){
  if (button.clickCount() == 1 && index == 2 && state == "menu"){
    param = "step";
    lcd.clear();
    setFreqStep();  
    } else if (button.clickCount() == 1 && state == "param" && param == "step"){
      FreqStep = increment[index];
      state = "menu";
      param = "menu";
      Serial.println(FreqStep);
      } else if (button.clickCount() == 1 && index == 1 && state == "menu"){
    param = "band";
    lcd.clear();
    setMeters(); 
      }
    else if (button.clickCount() == 1 && state == "param" && param == "band"){
      bandSelected = band[index];
      state = "menu";
      param = "menu";
     Serial.println(bandSelected);
 } else if (button.clickCount() == 1 && index == 0 && state == "menu"){
    param = "mode";
    lcd.clear();
    setMode(); 
      }
    else if (button.clickCount() == 1 && state == "param" && param == "mode"){
      modeSelected = mode[index];
      state = "menu";
      param = "menu";
      lcd.clear();
     Serial.println(modeSelected);
 }
}


// other Functions
void setFreqStep(){
  state = "param";
  if (index > indexSize) index = 0;
  lcd.setCursor(0,0);
  lcd.print("Frequency Select");  
  lcd.setCursor(7,1);
  lcd.print(increment[index]); 
  Serial.println(state);
  }

void setMeters(){
  state = "param";
  if (index > bandSize) index = 0;
  lcd.setCursor(2,0);
  lcd.print("Band Select");  
  lcd.setCursor(7,1);
  lcd.print(band[index]); 
  Serial.println(bandSize);
  }

  void setMode(){
  state = "param";
  if (index > modeSize) index = 0;
  lcd.setCursor(2,0);
  lcd.print("Mode Select");  
  lcd.setCursor(7,1);
  lcd.print(mode[index]); 
  Serial.println(modeSize);
  }

void lcdMenu(){
  if (index >  varSize) index = 0;
  lcd.setCursor(0,0);
  lcd.print("Select Parameter");
  lcd.setCursor(6,1);
  lcd.print(var[index]);
  Serial.println(index);
  }  
    

 

  
