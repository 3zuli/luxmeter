// BH1750 + 5510 LCD Luxmeter

#include <Wire.h>
#include <BH1750.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>


// Hardware SPI (faster, but must use certain hardware pins):
// pin 13 on Arduino (SCK) - LCD serial clock (SCLK) 
// pin 11 on an Arduino (MOSI) - LCD data in (DIN)
// pin 5 - LCD Data/Command select (D/C)
// pin 4 - LCD chip select (CS)
// pin 3 - LCD reset (RST)
Adafruit_PCD8544 display = Adafruit_PCD8544(5, 4, 3);
// Note with hardware SPI MISO and SS pins aren't used but will still be read
// and written to during SPI transfer.  Be careful sharing these pins!

/*Connection of BH1750:
 VCC-5v
 GND-GND
 SCL-SCL(analog pin 5)
 SDA-SDA(analog pin 4)
 ADD-NC or GND
*/
BH1750 lightMeter;

//84*48
const uint8_t BUFF_LENGTH = 83;
uint8_t buffIndex = 0;
uint16_t graphBuff[BUFF_LENGTH]={0};
uint16_t graphMin=65535, graphMax=0;

int numScreens = 2;
volatile int screenMode = 0;
void changeScreen(){
    Serial.print("Change screen from ");
    Serial.print(screenMode);
    screenMode = ++screenMode%numScreens;
    Serial.print(" to ");
    Serial.println(screenMode);
}

void testContrast(){
    int c = 0;
    display.setTextSize(2);
    display.setTextColor(BLACK);
    while(true){
        display.clearDisplay();
        display.setContrast(c);
        display.setCursor(0,16);
        display.print(c);
        display.display();
        Serial.print("contrast: ");
        Serial.println(c);
        c++;
        if(c==128) c=0;
        delay(100);
    }
}

void setup() {
  screenMode = 0;
  pinMode(9,OUTPUT);
  
  Serial.begin(57600);
  lightMeter.begin(BH1750_CONTINUOUS_HIGH_RES_MODE_2);

  display.begin();
  display.setContrast(50);

  // Contrast value of 50 works fine for me. If your LCD is badly readable,
  // or doesn't show anything at all, uncomment the following line to go into 
  // Contrast Test mode and see, which value works best for you
  // Then change display.setContrast(50) to that value
  //testContrast();
  
  pinMode(2,INPUT_PULLUP);
  attachInterrupt(0,changeScreen,FALLING);
}

void loop() {
  // Get lux reading from HB1750
  uint16_t lux = lightMeter.readLightLevel();
  Serial.print("Light: ");
  Serial.print(lux);
  Serial.println(" lx");
  
  // Turn on/off LCD backlight based on current light level
  if(lux>4000){
      digitalWrite(9, HIGH);
  }
  else{
      if(lux>3000){
          analogWrite(9, (lux-1000)/4);
      }
      else  
          digitalWrite(9, LOW); 
  }
    
  // Update graph buffer
  buffIndex = (buffIndex+1)%BUFF_LENGTH;
  graphBuff[buffIndex] = lux;
  // Update graph minimum and maximum values
  graphMax=graphBuff[0]; //0;
  graphMin=graphBuff[0]; //65535;
  for(int x=0; x<BUFF_LENGTH;x++){
      if(graphBuff[x]>graphMax) graphMax=graphBuff[x];
      if(graphBuff[x]<graphMin) graphMin=graphBuff[x]; 
  }
  
  // Update LCD
  display.clearDisplay();
  switch(screenMode){
      case 0:
          display.setTextSize(2);
          display.setTextColor(BLACK);
          display.setCursor(0,16);
          display.print(lux);
          display.setCursor(45,33);
          display.println("lux");
          display.display();
      break;
      case 1:
          for(int x=1;x<83;x++){
              /*if(buffIndex-x-1<2) 
                  display.drawPixel(x, 38-int(map(graphBuff[x-1], graphMin,graphMax, 0,38)), WHITE);
              else*/
              //display.drawPixel(x, 38-int(map(graphBuff[x-1], graphMin,graphMax, 0,38)), BLACK);
              //display.drawPixel(x, 38-int(map(graphBuff[(buffIndex+x)%BUFF_LENGTH ], graphMin,graphMax, 0,38)), BLACK);
        
              /*display.drawLine(x, 38-int(map(graphBuff[x-1], graphMin,graphMax, 0,38)),
                               x+1, 38-int(map(graphBuff[x], graphMin,graphMax, 0,38)), BLACK);*/
              if(x==1){
                  display.drawLine(x, 38-int(map(graphBuff[(buffIndex+x)%BUFF_LENGTH], graphMin,graphMax, 0,38)),
                                   x+1, 38-int(map(graphBuff[(buffIndex+x)%BUFF_LENGTH], graphMin,graphMax, 0,38)), BLACK);
              }
              else {
                  display.drawLine(x, 38-int(map(graphBuff[(buffIndex+x-1)%BUFF_LENGTH], graphMin,graphMax, 0,38)),
                                   x+1, 38-int(map(graphBuff[(buffIndex+x)%BUFF_LENGTH], graphMin,graphMax, 0,38)), BLACK);
              }
        
          }  
          // Refresh screen
          display.setTextSize(1);
          display.setTextColor(BLACK);
          display.setCursor(0,40);
          display.print(lux);
          display.println(" lux");
          display.display();
      break;      
  }
  delay(250);
}
