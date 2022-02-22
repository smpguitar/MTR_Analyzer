
#include <EncoderButton.h>   //https://github.com/Stutchbury/EncoderButton/blob/main/README.md
#include <LiquidCrystal_I2C.h>
#include <si5351.h>
#include "Wire.h"


//instantiation
LiquidCrystal_I2C lcd(0x27, 16, 2);
EncoderButton button(3, 2, 4);
Si5351 si5351;

//unchanging variables
const int band []{20, 30, 40, 80};
const unsigned long increment [] = {50, 100, 500};
const String mode []{"Automatic", "Manual"};
const String var []{"Mode", "Band", "Step"};

//Variables for control
int index;  //value of encoder
bool clickSubmit = false;  //when encoder clicked it sets true

//array sizes
int varSize = sizeof var / (sizeof var[0] + 1);
int modeSize = sizeof mode/ (sizeof mode[0] + 1);
int bandSize = sizeof band/ (sizeof band[0] + 1);
int indexSize = sizeof increment / (sizeof increment[0]+1) ;  //number of elements in increment

//user selected parameter variables
bool modeSelected;  //if true automatic if false manual.
int bandSelected;  // bandSelected by user
long FreqStep;  // FreqStep elected by user
int varSelected;  //UI element selected by user

int corr = -4350000; // this is the correction factor for the Si5351, use calibration sketch to find value.

volatile unsigned long Freq = 10000000ULL; // current Frequency

//SWR parameters needed:
  int FWD; // create forward sensor reading.  read the input on analog pin 0:
  int REV; //create reverse reading. read the input on analong pin 1
  long FwdVoltage;
  long RevVoltage;
  float swrValue;  //float SWR Value


void setup() {
  // put your setup code here, to run once:
Serial.begin(9600); 
button.setEncoderHandler(onTurn); 
button.setClickHandler(onClick);
  pinMode(FWD, INPUT);
  pinMode(REV, INPUT);

  analogReference(DEFAULT); //set reference to internal 5V source

  
// Initialize the display
  lcd.init();                      // initialize the lcd 
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
  // put your main code here, to run repeatedly:
button.update();
//setFreqStep();

Serial.println(index);
lcdSetParam();
//si5351.set_freq(Freq, SI5351_CLK0);

}

//Functions
int onTurn(EncoderButton& eb){
if (eb.position() >  indexSize) eb.resetPosition();
if (eb.position() < -indexSize) eb.resetPosition();
index = eb.position();
index = abs(index);
Serial.println(index);
return index;
  } 

void onClick(EncoderButton& eb){
  clickSubmit = true;
 }

int onResetIndex(){
index = 0;
return index;
}

bool onResetClickSubmit(){
  clickSubmit=false;
  }

void onReset(){
  onResetClickSubmit();
  onResetIndex();
  }  

long setFreqStep(){
  if(clickSubmit = false){
  Serial.println(increment[index]);
  } else {
    FreqStep= increment[index];
    return FreqStep;
    }
    onReset();
  }

void swr_calc(){
  FWD = analogRead(A1); // create forward sensor reading.  read the input on analog pin 0:
  FwdVoltage = FWD * (5.0 / 1023.0);    // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
  REV = analogRead(A2);  //create reverse reading. read the input on analong pin 1
  RevVoltage = REV * (1.0 / 1023.0);  // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
  swrValue = (FwdVoltage + RevVoltage) / (FwdVoltage - RevVoltage);  //swrValue = (forwardVoltage + reverseVoltage) / (forwardVoltage - reverseVoltage);
}

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

void draw_lcd(void)
  {
    char temp_str[21];
    
    sprintf_seperated(temp_str, Freq);
    lcd.setCursor(0, 0);
    lcd.print(temp_str);

    lcd.setCursor(0, 1);
    sprintf(temp_str, "%3u", increment[index]);
    lcd.print("Stp ");
    lcd.print(temp_str);
    lcd.print("SWR");
    lcd.print(swrValue);
  }

  void lcdSetParam(){
    if (index == 0){
    lcdBlink();
    lcd.setCursor(0, 1);
    lcd.print(var[1]);
    lcd.setCursor(12, 1);
    lcd.print(var [2]);   
      }

    if (index == 1){
    lcd.setCursor(6, 0); 
    lcd.print(var[0]);
    lcdBlink();
    lcd.setCursor(12, 1);
    lcd.print(var [2]);
      }  

    if (index == 2){
    lcd.setCursor(6, 0); 
    lcd.print(var[0]);
    lcd.setCursor(0, 1);
    lcd.print(var[1]);
    lcdBlink();
    
    Serial.println(index);
      }  
    }

 void lcdBlink(){
    if (index == 0){
    lcd.setCursor(6, 0); 
    lcd.print(var[0]);
    delay (500);
    lcd.setCursor(6, 0);
    lcd.print("    ");
    delay (500); 
      } else {
    lcd.setCursor(6, 0); 
    lcd.print(var[0]);
        }

    if (index == 1){
    lcd.setCursor(0, 1); 
    lcd.print(var[1]);
    delay (500);
    lcd.setCursor(0, 1);
    lcd.print("    "); 
    delay (500); 
      } else {
    lcd.setCursor(0, 1); 
    lcd.print(var[1]);
        }

    if (index == 2){
    lcd.setCursor(12, 1); 
    lcd.print(var[2]);
    delay (500);
    lcd.setCursor(12, 1);
    lcd.print("    "); 
    delay (500);    
      }  else {
    lcd.setCursor(12, 1); 
    lcd.print(var[2]);  
        }
      
      }

  
