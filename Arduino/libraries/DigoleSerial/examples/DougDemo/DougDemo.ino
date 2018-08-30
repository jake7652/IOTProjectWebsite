/*
 *This demo code will show you all functions for
 *Digole Graphic LCD adapter
 */
#define _Digole_Serial_UART_  //To tell compiler compile the special communication only, 
//all available are:_Digole_Serial_UART_, _Digole_Serial_I2C_ and _Digole_Serial_SPI_
#include <DigoleSerial.h>
//--------UART setup
#if defined(_Digole_Serial_UART_)
DigoleSerialDisp mydisp(&Serial, 9600); //UART:Arduino UNO: Pin 1(TX)on arduino to RX on module
#endif
//--------I2C setup
#if defined(_Digole_Serial_I2C_)
#include <Wire.h>
DigoleSerialDisp mydisp(&Wire,'\x27');  //I2C:Arduino UNO: SDA (data line) is on analog input pin 4, and SCL (clock line) is on analog input pin 5 on UNO and Duemilanove
#endif
//--------SPI setup
#if defined(_Digole_Serial_SPI_)
DigoleSerialDisp mydisp(8,9,10);  //SPI:Pin 8: data, 9:clock, 10: SS, you can assign 255 to SS, and hard ground SS pin on module
#endif

const unsigned char welcomeimage[] PROGMEM = {
    0, 0, 0, 0, 0, 127
    , 0, 8, 1, 2, 0, 127
    , 0, 8, 0, 148, 0, 127
    , 0, 16, 0, 89, 112, 127
    , 3, 144, 0, 16, 144, 127
    , 28, 145, 192, 16, 144, 127
    , 1, 30, 128, 80, 144, 127
    , 9, 49, 3, 208, 144, 127
    , 5, 72, 0, 80, 144, 127
    , 2, 72, 0, 150, 144, 127
    , 3, 8, 0, 152, 208, 127
    , 5, 24, 0, 64, 160, 127
    , 4, 148, 0, 64, 128, 127
    , 8, 36, 0, 128, 128, 127
    , 16, 34, 3, 240, 128, 127
    , 32, 65, 0, 14, 0, 127
    , 0, 129, 128, 1, 252, 127
    , 3, 0, 64, 0, 0, 127
    , 0, 0, 0, 0, 0, 127
    , 0, 0, 0, 0, 0, 127
    , 0, 0, 0, 0, 0, 127
};
int ptr;
const char a[] = "disp char array";
const char b = 'Q';
int c = 3456;
String d = "I'm a string";
float pi = 3.1415926535;
double lg10;
const unsigned char fonts[] = {6, 10, 18, 51, 120, 123};
const char *fontdir[] = {"0\xb0", "90\xb0", "180\xb0", "270\xb0"};
void resetpos1(void) //for demo use, reset display position and clean the demo line
{
    mydisp.setPrintPos(0, 0, _TEXT_);
    delay(3000); //delay 2 seconds
    mydisp.println("                "); //display space, use to clear the demo line
    mydisp.setPrintPos(0, 0, _TEXT_);
}
void resetpos(void) //for demo use, reset display position and clean the demo line
{
    mydisp.setPrintPos(0, 1, _TEXT_);
    delay(3000); //delay 2 seconds
    mydisp.println("                "); //display space, use to clear the demo line
    mydisp.setPrintPos(0, 1, _TEXT_);
}

void setup() {
    mydisp.begin();
    /*----------for text LCD adapter and graphic LCD adapter ------------*/
    mydisp.clearScreen(); //CLear screen
    mydisp.displayConfig(1);    //set config display ON, 0=off
    delay(5000);

    for (uint8_t j = 0; j < 4; j++) //making "Hello" string moving
    {
        for (uint8_t i = 0; i < 10; i++) //move string to right
        {
            mydisp.setPrintPos(i, 1, _TEXT_);
            mydisp.print(" Hello ");
            delay(100); //delay 100ms
        }
        for (uint8_t i = 0; i < 10; i++) //move string to left
        {
            mydisp.setPrintPos(9 - i, 1, _TEXT_);
            mydisp.print(" Hello ");
            delay(100);
        }
    }
    mydisp.print("Enjoy it!");
    mydisp.enableCursor(); //enable cursor
    /*---------for Graphic LCD adapter only-------*/
    mydisp.disableCursor(); //enable cursor
    resetpos();
    mydisp.drawStr(0, 1, "Negative/flash");
    mydisp.setMode('~');
    for (uint8_t i = 0; i < 10; i++) {
        mydisp.drawBox(0, 13, 110, 13);
        delay(500);
    }
    resetpos();
    mydisp.clearScreen();
    mydisp.drawStr(0, 0, "Draw image in 4 dir");
    //display image in 4 different directions, use setRot90();setRot180();setRot270();undoRotation();setRotation(dir);
    for (uint8_t i = 0; i < 4; i++) {
        mydisp.setRotation(i);
        mydisp.drawBitmap(12, 12, 41, 21, welcomeimage);
    }
    mydisp.undoRotation();

    //test differnt font
    resetpos1();
    mydisp.clearScreen();
    mydisp.drawStr(0, 0, "default font");
    mydisp.setFont(fonts[0]);
    mydisp.nextTextLine();
    mydisp.print("Font 6");
    mydisp.setFont(fonts[1]);
    mydisp.nextTextLine();
    mydisp.print("Font 10");
    mydisp.setFont(fonts[2]);
    mydisp.nextTextLine();
    mydisp.print("Font 18");
    mydisp.setFont(fonts[3]);
    mydisp.nextTextLine();
    mydisp.print("Font 51");
    resetpos1();
    mydisp.clearScreen();
    mydisp.setFont(fonts[4]);
    mydisp.setPrintPos(0, 0, _TEXT_);
    mydisp.print("Font 120");
    mydisp.setFont(fonts[5]); //font 123 only have chars of space and 1 to 9
    mydisp.nextTextLine();
    mydisp.print("123");
    resetpos1();
    mydisp.clearScreen();
    //User font using number: 200,201,202,203,only available after upload to adapter
    mydisp.setFont(10);
    mydisp.drawStr(0, 0, "User font using:200,201,202,203,available after upload to adapter,16Kb space.");
    //Control backlight on/off
    resetpos1();
    mydisp.clearScreen();
    mydisp.drawStr(0, 0, "Turn backlight off after 2 seconds the on again");
    delay(2000);
    mydisp.backLightOff();
    delay(2000);
    mydisp.backLightOn();
    //move area on screen
    resetpos1();
    mydisp.clearScreen();
    //this section show how to use enhanced TEXT position functions:
    //setTextPosBack(),setTextPosOffset() and setTextPosAbs() to make some thing fun
    resetpos1();
    mydisp.clearScreen();
    mydisp.setMode('|');
    mydisp.print("Bold D");
    mydisp.setFont(120);
    mydisp.nextTextLine();
    mydisp.print('D');
    mydisp.setTextPosBack(); //set text position back
    delay(2000);
    mydisp.setTextPosOffset(2, 0); //move text position to x+2
    mydisp.print('D'); //display D again, under or mode, it will bold 'D'
    //use setTextPosAbs() to make a Animation letter
    resetpos1();
    mydisp.clearScreen();
    mydisp.print("make > animation");
    mydisp.setFont(51);
    mydisp.setMode('C');
    for (uint8_t i = 0; i < 110; i++) {
        mydisp.setTextPosAbs(i, 45);
        mydisp.print('>');
        delay(30);
    }
    //show font in different direction
    resetpos1();
    mydisp.clearScreen();
    mydisp.setFont(fonts[1]);
    mydisp.print("show font in different direction");
    resetpos1();
    mydisp.clearScreen();
    for (uint8_t i = 0; i < 4; i++) {
        mydisp.setRotation(i);
        mydisp.setFont(fonts[2]);
        mydisp.setPrintPos(0, 0);
        for (uint8_t j = 0; j < 3; j++) {
            mydisp.setFont(fonts[2 - j]);
            if (j != 0)
                mydisp.nextTextLine();
            mydisp.print(fontdir[i]);
        }
    }

}
void loop() {
}
