
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
String param = "menu"; //parameter being adjusted
String state = "menu";  //UI element selected by user
bool longPress = false; 

//array sizes
int varSize = sizeof var / (sizeof var[0] + 1);
int modeSize = sizeof mode/ (sizeof mode[0] + 1);
int bandSize = (sizeof band - 1)/ (sizeof band[0]);
int indexSize = sizeof increment / (sizeof increment[0]+1) ;  //number of elements in array

//user selected parameter variables
String modeSelected = "Manual";  //Automatic and Manual selected by user
int bandSelected = 20;  // bandSelected by user
long FreqStep = 500;  // FreqStep elected by user
int Freqknob = 0;

int corr = -4350000; // this is the correction factor for the Si5351, use calibration sketch to find value.

volatile unsigned long Freq = 10000000UL; // current Frequency

//SWR parameters needed:
  int FWD; // create forward sensor reading.  read the input on analog pin 0:
  int REV; //create reverse reading. read the input on analong pin 1
  float FwdVoltage;
  float RevVoltage;
  float swrValue;  //float SWR Value
  String swrValuePrint;
  char tempBuffer [7];
 
  

//Automatic SWR Variables
long FreqStart;
long FreqStop;
volatile unsigned long minSwrFreq;
volatile float minSwr = 10.0;
String minSwrPrint;

void setup() {
Serial.begin(9600); 
button.setEncoderHandler(onTurn); 
button.setClickHandler(onClick);
button.setLongPressHandler(onLongPress, true);

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
  si5351.set_freq(Freq * 100UL , SI5351_CLK1);
}

void loop() {
//always running  
button.update(); // button listener
si5351.set_freq(Freq * 100UL, SI5351_CLK1);

//Display Logic
if (state == "menu" && param == "menu"){
lcdMenu();
} else if (state == "param" && param == "step"){
  setFreqStep();
  }else if (state == "param" && param == "band"){
    setMeters();
    }else if (state == "param" && param == "mode"){
    setMode();
    }  

}

//Functions
//SWR Calc
void swrCalc(){
  FWD = analogRead(A1); // create forward sensor reading.  read the input on analog pin 0:
  FwdVoltage = FWD * (5.0 / 1023.0);    // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
  REV = analogRead(A0);  //create reverse reading. read the input on analong pin 1
  RevVoltage = REV * (5.0 / 1023.0);  // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
  swrValue = (FwdVoltage + RevVoltage) / (FwdVoltage - RevVoltage);  //swrValue = (forwardVoltage + reverseVoltage) / (forwardVoltage - reverseVoltage);
  swrValuePrint = dtostrf(swrValue, 4, 2, tempBuffer);
  Serial.println(RevVoltage);
}


//input device functions
int onTurn(EncoderButton& eb){
 if(state == "execute") {
  Freqknob = eb.increment();
  changeFreq();
  onExecute(); //refresh LCD
 }
 if(state != "execute") {
 lcd.clear(); // rewrite display
 index = index + abs(eb.increment()); 
 index = index;
 }
 
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

void onLongPress(EncoderButton& eb){
if (eb.longPressCount() > 0 && state == "menu" && param == "menu" && modeSelected == "Manual"){
    state = "execute";
    param = "execute";
    if (bandSelected == 80){  //80M
    Freq = 3180000UL;
  } else if (bandSelected == 40){
    Freq = 7030000UL;
  } else if (bandSelected == 30){
    Freq = 10118000UL;
  } else if (bandSelected == 20){
    Freq = 14060000UL;
  } 
    /*else if (bandSelected == autoInf){
     case 5:  //full sweep
    FreqStart = 3000000UL; 
    FreqStop = 30000000UL;
  }*/
      lcd.clear();
      index = 0;
      onExecute();
      Serial.println(state);
    } else if (eb.longPressCount() > 0 && state == "menu" && param == "menu" && modeSelected == "Automatic"){
    state = "execute";
    param = "execute";
    Serial.println(modeSelected);
    if (bandSelected == 80){  //80M
    FreqStart = 2000000UL;
    FreqStop = 4000000UL;
  } else if (bandSelected == 40){
    FreqStart = 6000000UL;
    FreqStop = 8000000UL;
  } else if (bandSelected == 30){
    FreqStart = 9000000UL;
    FreqStop = 11000000UL;
  } else if (bandSelected == 20){
    FreqStart = 13000000UL;
    FreqStop = 15000000UL;
  } 
      lcd.clear();
      index = 0;
      Freq = FreqStart;
      sweep();
    } else if (eb.longPressCount() > 0 && state == "execute"){
      state = "menu";
      param = "menu";
      lcd.clear();
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
  }  

void sweep(){
  while (Freq < FreqStop) {
  swrCalc(); 
  Freq = Freq + (FreqStep *100UL); //increment freq
  onExecute();
   if (swrValue < minSwr){
    minSwr = swrValue;
    minSwrPrint = dtostrf(minSwr, 4, 2, tempBuffer);
    minSwrFreq = Freq; 
    } 
 }
 sweepResults();
  }


void changeFreq(){
  Freq = Freq + (Freqknob * (FreqStep * 100UL));
  Freqknob = 0;
  Serial.println(Freq);
  }
    
// State Execute Display Logic
 void sprintf_seperated(char *str, unsigned long num)
{
  // We will print out the frequency as a fixed length string and pad if less than 100s of MHz
  char temp_str[6];
  int zero_pad = 0;
  
  // MHz
  if(num / 1000000UL > 0)
  {
    sprintf(str, "%3lu", num / 1000000UL);
    zero_pad = 1;
  }
  else
  {
    strcat(str, "   ");
  }
  num %= 1000000UL;
  
  // kHz
  if(zero_pad == 1)
  {
    sprintf(temp_str, ",%03lu", num / 1000UL);
    strcat(str, temp_str);
  }
  else if(num / 1000UL > 0)
  {
    sprintf(temp_str, ",%3lu", num / 1000UL);
    strcat(str, temp_str);
    zero_pad = 1;
  }
  else
  {
    strcat(str, "   ");
  }
  num %= 1000UL;
  
  // Hz
  if(zero_pad == 1)
  {
    sprintf(temp_str, ",%03lu", num);
    strcat(str, temp_str);
  }
  else
  {
    sprintf(temp_str, ",%3lu", num);
    strcat(str, temp_str);
  }
  
  strcat(str, " MHz");
}

void onExecute() {
    char temp_str[21];
    swrCalc();
    sprintf_seperated(temp_str, Freq);
    lcd.setCursor(0, 0);
    lcd.print(temp_str);
    
    lcd.setCursor(0, 1);
    lcd.print("Stp ");
    lcd.print(FreqStep);
    lcd.print(" ");
    lcd.print("SWR");
    lcd.print(swrValuePrint);
  }

void sweepResults(){
    char temp_str[21];
    sprintf_seperated(temp_str, minSwrFreq);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Min SWR Freq");
    lcd.setCursor(0,1);
    lcd.print(temp_str);
    
    delay(2000);
    lcd.clear();
    lcd.setCursor(4, 0);
    lcd.print("Min SWR");
    lcd.setCursor(5, 1);
    lcd.print(minSwrPrint);
    delay(2000);
}

  
