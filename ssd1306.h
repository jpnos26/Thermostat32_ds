#include <U8x8lib.h>

U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(/* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16);

String ftoa(float number, uint8_t precision, uint8_t size) {
  // Based on mem,  16.07.2008
  // http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num = 1207226548/6#6

  // prints val with number of decimal places determine by precision
  // precision is a number from 0 to 6 indicating the desired decimial places
  // example: printDouble(3.1415, 2); // prints 3.14 (two decimal places)

  // Added rounding, size and overflow #
  // ftoa(343.1453, 2, 10) -> "    343.15"
  // ftoa(343.1453, 4,  7) -> "#      "
  // avenue33, April 10th, 2010

  String s = "";

  // Negative
  if (number < 0.0)  {
    s = "-";
    number = -number;
  }

  double rounding = 0.5;
  for (uint8_t i = 0; i < precision; ++i)    rounding /= 10.0;

  number += rounding;
  s += String(uint16_t(number));  // prints the integer part

  if(precision > 0) {
    s += ".";                // prints the decimal point
    uint32_t frac;
    uint32_t mult = 1;
    uint8_t padding = precision -1;
    while(precision--)     mult *= 10;

    frac = (number - uint16_t(number)) * mult;

    uint32_t frac1 = frac;
    while(frac1 /= 10)    padding--;
    while(padding--)      s += "0";

    s += String(frac,DEC) ;  // prints the fractional part
  }

  if (size>0)                // checks size
    if (s.length()>size)        return("#");
    else while(s.length()<size) s = " "+s;

  return s;
}

void clear_display(){
  u8x8.clear();
}
void init_display(){
  u8x8.begin();
  u8x8.setFont(u8x8_font_8x13B_1x2_f);
  u8x8.clear();
  u8x8.drawString(0,3,"Starting ..");
  
}
void write_display(){
  u8x8.setFont(u8x8_font_pxplusibmcgathin_f); 
  char invdate[11];
  time_t test = now()+946684800UL;
  sprintf(invdate,"%02u/%02u/%04u",day(test),month(test),year(test));
  u8x8.drawString( 0,0, invdate );
  sprintf(invdate,"%02u:%02u",hour(test),minute(test));
  u8x8.drawString( 11,0, invdate );
  u8x8.setFont(u8x8_font_px437wyse700a_2x2_f );
  
  char currentsend[8];
  String ssend =String(Temperatura,1)+char(176);
  ssend.toCharArray(currentsend, 8);
  u8x8.drawString( 0,2, currentsend );
  u8x8.setFont(u8x8_font_pcsenior_f);
  if (stato =="ON"){
    u8x8.inverse();
  }
  if (checkEnable == 0){
    ssend = String(setPoint[0],1)+char(176);
  }
  else{
    ssend = String(setT,1)+char(176);
  }
  ssend.toCharArray(currentsend, 6);
  u8x8.drawString( 11,3,currentsend);
  u8x8.noInverse();
  u8x8.setFont(u8x8_font_pressstart2p_f);
  ssend = String(Umidita,1)+"%  ";
  ssend.toCharArray(currentsend,8);
  u8x8.drawString( 1,5, currentsend);
 
  ssend = String(Pressione,0)+"mb";
  ssend.toCharArray(currentsend,8);
  u8x8.drawString( 9,5, currentsend);
  u8x8.drawString(0,7,descProg[checkEnable]);
  if (checkEnable == 0){
    u8x8.drawString(8,7,"        ");
  }
  else if(checkEnable == 3){
    u8x8.drawString(8,7,"        ");  
  }
  else{
    u8x8.drawString(8,7,descPoint[checkTemperature]);
  }
  
}
