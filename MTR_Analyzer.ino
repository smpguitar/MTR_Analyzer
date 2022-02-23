
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
String param; //parameter being adjusted

//array sizes
int varSize = sizeof var / (sizeof var[0] + 1);
int modeSize = sizeof mode/ (sizeof mode[0] + 1);
int bandSize = sizeof band/ (sizeof band[0] + 1);
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
lcdMenu();
//si5351.set_freq(Freq, SI5351_CLK0);

}

//Functions
//input device functions
int onTurn(EncoderButton& eb){
 index = index + abs(eb.increment()); 
 index = index;
 Serial.println(index);
  } 

void onClick(EncoderButton& eb){
  if (button.clickCount() == 1 && index == 2 && state == "menu"){
    param = "step";
    Serial.println(param);
    lcd.clear();
    setFreqStep();
    } else if (button.clickCount() == 1 && state == "param" && param == "step"){
      FreqStep = increment[index];
      }
 }


// other Functions
void setFreqStep(){
  state = "param";
  lcd.setCursor(0,0);
  lcd.print("Frequency Select");  
  lcd.print(increment[index]); 
  }

void lcdMenu(){
  state = "menu";
  if (index >  indexSize) index = 0;
  if (index < -indexSize) index = 0;
  lcd.setCursor(0,0);
  lcd.print("Select Parameter");
  lcd.setCursor(6,1);
  lcd.print(var[index]);
  Serial.println(index);
  }  
    

 

  
