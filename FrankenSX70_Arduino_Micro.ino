//FrankenSX70_Arduino_Micro
// HELLO SX70!!!!!
//First code on GitHub
//Remember I am not a programmer, this is probably a mess. Use at your own risk.
//You can do whatever you want for personal use but NOT for commercial purposes.
//As for the electronics are really simple but will try to make an schematic when I find the time
//This will work with "normal" arduinos (UNO etc...)
//Remember: you have been warned!

//ADDED timer function: shortPress and longPress control the timer function: shortPress = just take a picture
//                                                                       but longPress = 10 seconds delay (or whatever you define)

// FIRST I ASSIGN THE Sx SWITCHES OF THE SX70 TO PINS ON THE ARDUINO
// THESE ARE INPUTS
// SX70-ARDUINO LOGICAL EQUIVALENTS
// S1: LOW = CLOSED
// S3: LOW = CLOSED
// S5: LOW = CLOSED
// S8: HIGH = CLOSED
// S9: HIGH = CLOSED
const int S1 = 2; //Red button SHUTTER RELEASE
//S1: LOW = CLOSED
const int S3 = 3;
//S3: LOW = CLOSED
const int S5 = 4;
//S5: LOW = CLOSED
const int S8 = 5;
//S8: HIGH = CLOSED
const int S9 = 6;
//S9: HIGH = CLOSED

// const int FFA = 7;
//this for Flash Firing
const int Selector = 8;
//this is an external switch for Double Exposure/Normal operation
// const int Sol2 = 9;

// const int S2 =12;
//this CLOSED when there is a FLASHBAR inserted
// NOW I ASSIGN THE OUTPUTS
// SOLENOID #1 IS THE SHUTTER
// IF OFF THEN THE SHUTTER IS OPEN (SLR FUNCTION)
// THEN "PowerUp" IS "ALL" POWER TO MOVE TO "CLOSED" POSSITION
// WITH THE "ALL POWER" WE CONSUME TOO MUCH, SO WE GO TO "PowerDown"
// PowerDown WE USE A RESISTOR TO LIMIT POWER
const int solenoidPowerUp = 11;           // 6V High Power
const int solenoidPowerDown = 10;          //low-power solenoid mode

// MOTOR JUST TURNS ON OR OFF THE MOTOR
// DEPENDS ON S3 AND S5 STATES PRIMARILY
const int Motor = 13;

// THESE ARE THE INPUTS USED TO "READ" THE SHUTTER SPEED SELECTOR
const int switchPin1 = A0; // the number of the switch’s pin
const int switchPin2 = A1; // the number of the switch’s pin
const int switchPin3 = A2; // the number of the switch’s pin
const int switchPin4 = A3; // the number of the switch’s pin

// RED LED USED FOR DEBUGGING PURPOSES
const int REDled = A4;
// GREEN LED USED FOR DEBUGGING PURPOSES
const int GREENled = A5;

bool takePicture = false;

//selectorPOSITION is the position of the selector, we assign speeds with ShutterSpeed
int ActualSlot;
//These are the actual speed in ms

//int ShutterSpeed[] = {0, 1000, 500, 250, 125, 66, 33, 16, 8, 0};
//inversed wheel
int ShutterSpeed[] = {0, 8, 16, 33, 66, 125, 250, 500, 1000, 0};


int shots = 0;
byte brightness = 160;


         // Lets define what is considered a longPress and a shortPress
         // shortPress is when you want to take a "regular" picture
         // it needs to be timed for debounce purposes, that is, you need a "solid" press to take a picture
  
  const int shortPress = 150;
  
         //longPress is when you want to "something else", in my case delay the taking of the picture for x (10) seconds.
         //since 1000ms = 1 seconds, this is just a bit more than 1 second.

  const int  longPress = 1200;


void setup() {
  // this code only runs once:


  //Solenoid #1 PINS are OUTPUT
  pinMode(solenoidPowerUp, OUTPUT);
  pinMode(solenoidPowerDown, OUTPUT);

  //SX70 switches are INPUTS
  //I like to think that the SX70 has mechanical electronics :-)
  pinMode(S1, INPUT);
  pinMode(S3, INPUT);
  pinMode(S5, INPUT);
  pinMode(S8, INPUT);
  pinMode(S9, INPUT);
  // pinMode(FFA, OUTPUT);
  // pinMode(S2, INPUT);
  pinMode(Selector, INPUT);
  // pinMode (Sol2, OUTPUT);

  //LED are OUTPUTs
  pinMode(REDled, OUTPUT);
  pinMode(GREENled, OUTPUT);

  //Motor is OUTPUT
  pinMode(Motor, OUTPUT);

  //I make sure that the Motor is OFF
  digitalWrite(Motor, LOW);

  //I also want the Shutter to be open for SLR-TTL fuctionality
  digitalWrite(solenoidPowerUp, LOW);
  digitalWrite(solenoidPowerDown, LOW);

  //And then, LEDs initially are OFF
  analogWrite(REDled, 0);
  analogWrite(GREENled, 0);

  //The pins in the Shutter Speed selector are INPUTs
  pinMode(switchPin1, INPUT);
  pinMode(switchPin2, INPUT);
  pinMode(switchPin3, INPUT);
  pinMode(switchPin4, INPUT);
    // TURN OFF LEDs
  analogWrite(REDled, 0);
  analogWrite(GREENled, 0);

  //This is just in case the mirror is (for whatever reason) halfway when I power up I want to run this JUST ONCE
  while (digitalRead(S5) != LOW || digitalRead(S3) != LOW)  // while S5 or S3 are different than CLOSED (mirror at an angle)
  {
  motorON ();
  motorOFF();
  }

  }


void loop() {

  
  //WHAT TO DO WHEN POWER-UP:
  //  S8     S9
  //closed  open  --> EJECT DARKSLIDE (DEFAULT)
  // open  closed --> FILM REACH 0 (NO FLASH)
  // open   open  --> NORMAL OPERATION 10 TO 1
 
    // STATE 1: EJECT DARKSLIDE:
  if (digitalRead(S8) == HIGH && digitalRead(S9) == LOW)
    //EJECT DARK SLIDE
  {
    //analogWrite (REDled, 60);
    //analogWrite (GREENled, 0);
    darkslideEJECT();
  }

  //STATE 2: PACK IS EMPTY--> NO WASTE OF FLASH
  if (digitalRead(S8) == LOW && digitalRead(S9) == HIGH)
    // FOR THE MOMENT I JUST TURN ON THE GREEN LED
  {
    //analogWrite (GREENled, 60);
    //analogWrite (REDled, 0);

}

  //STATE 3: NORMAL OPERATION
  if (digitalRead(S8) == LOW && digitalRead(S9) == LOW)
  {
    //TURN ON GREEN LED and RED DIMM
    //analogWrite (GREENled, 160);
    //analogWrite (REDled, 160);
   
// ///////////////////////////////////PICTURE TAKING OPERATION//////////////////////////////////////////////////
    /* FOUR CASES:
     *  CASE 1 NORMAL OPERATION: FULL CYCLE
     *  SELECTOR = NORMAL (LOW)
     *  SHOTS = 0
     *  S1 = LOW (RED BUTTON PRESSED)
     *  
     *  CASE 2 DOUBLE EXPOSURE FIRST SHOT: MIRROR DOWN AND FIRST PICTURE (CLICK: SHUTTER OPERATION REMAINING CLOSED)
     *  SELECTOR = DOUBLE (HIGH)
     *  SHOTS = 0
     *  S1 = LOW (RED BUTTON PRESSED)
     *  
     *  CASE 3 DOUBLE EXPOSURE ULTERIOR SHOTS: NO MOTOR OPERATION JUST PICTURE (CLICK: SHUTTER OPERATION REMAINING CLOSED)
     *  SELECTOR = DOUBLE (HIGH)
     *  SHOTS >= 1
     *  S1 = LOW (RED BUTTON PRESSED)
     *  CASE 4 PICTURE EXPULSION AFTER DOUBLE EXPOSURE: MIRROR DOWN AND SHUTTER OPENING (NO PICTURE TAKEN)
     *  
     *  SELECTOR = NORMAL (LOW)
     *  SHOTS >= 1
     */

            int pressTime = REDbutton(S1);
  
            if ((pressTime > shortPress) && (pressTime < longPress)) {
  
              Serial.println("---------------------------");
              Serial.print ("SHORT:");
              Serial.println (pressTime);
              Serial.println("---------------------------");

            takePicture = true;
              
            }  // END OF if ((pressTime > shortPress) && (pressTime < longPress)) {
            
            else if (pressTime > longPress) {
              
              Serial.println("---------------------------");
              Serial.print ("LONG: ");
              Serial.println (pressTime);
              Serial.println("---------------------------");
              
            timerDelay(); 
            takePicture = true;

            }   // END Of else if (pressTime > longPress) {

            
           

    
    if (takePicture == true && digitalRead(Selector) ==  HIGH && shots == 0)          //NORMAL OPERATION
     {
          analogWrite (GREENled, 0);  //selector NORMAL just take a picture and EJECT

          shutterCLOSE ();  // I CLOSE THE SHUTTER
                
        //delay (20);     //I wait? Is this really necessary???
        //Motor Starts: MIRROR COMES UP!!!
      
        mirrorUP();

        while (digitalRead(S3) != HIGH)            //waiting for S3 to OPEN
          ;
                
        delay (40);                               //S3 is now open start Y-delay (40ms)

        Click ();                                // NOW I am going to take the picture: SMILE!
        analogWrite (REDled, brightness);    //shots taken

        delay (200);                             //AGAIN is this delay necessary?
        
        mirrorDOWN ();                          //Motor starts, let bring the mirror DOWN

        delay (200);                             //AGAIN is this delay necessary?

        shutterOPEN();
       

     }  //end of if (digitalRead(S1) == LOW && digitalRead(Selector) ==  HIGH && shots == 0)          //NORMAL OPERATION
                  
     // CASE 2 DOUBLE EXPOSURE FIRST SHOT: MIRROR DOWN AND FIRST PICTURE (CLICK: SHUTTER OPERATION REMAINING CLOSED)           
     if (digitalRead(S1) == LOW && digitalRead(Selector) ==  LOW && shots == 0)    //DOUBLE EXPOSURE and RED BUTTON PRESSED (S1) FIRST SHOT!!!!
      {
         analogWrite (GREENled, 255);

          shutterCLOSE ();  // I CLOSE THE SHUTTER

                                          // delay (20);     //I wait? Is this really necessary???
                                            //Motor Starts: MIRROR COMES UP!!!
          mirrorUP();

          while (digitalRead(S3) != HIGH)            //waiting for S3 to OPEN
            ;
          //S3 is now open start Y-delay (40ms)
          delay (40);

          Click ();                 // NOW I am going to take the picture: SMILE!
          analogWrite (REDled, brightness);
          delay (1000);            //debounce will go here
          shots++;


          } // end of if if (digitalRead(S1) == LOW && digitalRead(Selector) ==  LOW && shots == 0)    //DOUBLE EXPOSURE and RED BUTTON PRESSED (S1) FIRST SHOT!!!!

         // CASE 3 DOUBLE EXPOSURE ULTERIOR SHOTS: NO MOTOR OPERATION JUST PICTURE (CLICK: SHUTTER OPERATION REMAINING CLOSED)  
         if (digitalRead(S1) == LOW && digitalRead(Selector) ==  LOW && shots >= 1)    //DOUBLE EXPOSURE and RED BUTTON PRESSED (S1) ULTERIOR SHOTS!!!!
          {
          Click();
          
          delay (1000); //debounce will go here
          
          shots++;

          brightness = brightness + 40;

          analogWrite (REDled, brightness);
          
          } // end of  if (digitalRead(S1) == LOW && digitalRead(Selector) ==  LOW && shots >= 1)    //DOUBLE EXPOSURE and RED BUTTON PRESSED (S1) ULTERIOR SHOTS!!!!

          // CASE 4 PICTURE EXPULSION AFTER DOUBLE EXPOSURE: MIRROR DOWN AND SHUTTER OPENING (NO PICTURE TAKEN)
         if (digitalRead(S1) == HIGH && digitalRead(Selector) ==  HIGH && shots >= 1)  //Was in DOUBLE EXPOSURE MODE but NOW is back to NORMAL
          {
              analogWrite (GREENled, 0);  //selector LOW
              analogWrite (REDled, brightness);    //shots taken
              shots = 0;
 
            mirrorDOWN ();

            delay (200);                             //AGAIN is this delay necessary?

            shutterOPEN();    

           }   // end of if (digitalRead(S1) == HIGH && digitalRead(Selector) ==  LOW && shots > 1)  //Was in DOUBLE EXPOSURE MODE but NOW is back to NORMAL
   

    }// end of if (digitalRead(S8) == LOW && digitalRead(S9) == LOW)

} //END OF loop      


// -------------------------------------------------------------------------------------------------------------------------------------------------

//                                HERE GO MY FUNCIONS

// -------------------------------------------------------------------------------------------------------------------------------------------------

void timerDelay() {
  // this is a test function to do the progressing blinking of the LED using my blink function
  // it last exactly 10 seconds (2500x4) and I could not accomplish this with the delay() 
  // everytime the led (in pin 5) blinks faster 1000, 700, 400, and 100.
unsigned long startTimer = millis();
Serial.println ("Start TIMER countdown: ");
Serial.println ("START 700,2500,5 = SLOW BlINK");
blink (700,2500,REDled);
;
Serial.println ("START 500,2500,5 = MEDIUM-SLOW BlINK");
blink (500,2500,REDled);
;
Serial.println ("START 200,2500,5 = MEDIUM-FAST BlINK");
blink (200,2500,REDled);
;
Serial.println ("START 100,2500,5 = FAST BlINK");
blink (100,2500,REDled);
;
Serial.println ("END");

unsigned long endTimer = millis();

unsigned long time = ((endTimer-startTimer)/1000);
;
Serial.println ("===========================");
Serial.print ("TIME:   ");
Serial.print(time);
Serial.println ("  seconds  ");
Serial.println ("===========================");


            takePicture = false;
            
} // END of timerDelay


int blink (int interval, int timer, int ledPin)
// blink (blink interval=blinking speed, timer=duration blinking, ledPin=pin of LED)
// blink is a standalone function
{


int ledState = LOW;             // ledState used to set the LED
pinMode(ledPin, OUTPUT);
  
unsigned long previousMillis = 0;        // will store last time LED was updated

unsigned long currentMillisTimer = millis();
  
  while (millis() < (currentMillisTimer + timer)) 
  
  {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;

    // if the LED is off turn it on and vice-versa:
    if (ledState == LOW) {
      ledState = HIGH;
    } else {
      ledState = LOW;
    }

    // set the LED with the ledState of the variable:
    digitalWrite(ledPin, ledState);
  }
  }
} //   END OF blink FUNCTION


int REDbutton(int button) {
    
    // REDbutton (button pin)
    // REDbutton is a standalone function
    // by Joaquín de Prada
  
// REDbutton variables  
static boolean buttonActive = false;
static unsigned long buttonTimer = 0;
static unsigned long STARTbuttonTimer = 0 ;
// END of REDbutton variables
    
    pinMode(button, INPUT);
 
    
int result= 0 ;

if (digitalRead(button) == LOW) {
            if (buttonActive == false) {
           Serial.println("button pressed");
            STARTbuttonTimer = millis(); 
            buttonActive = true ;
                                        }
     
     } else  {


      if (buttonActive == true) {
            buttonTimer = millis();
            Serial.println("button just released");
            Serial.print("pressed time inside function:  ");
            result = (buttonTimer - STARTbuttonTimer);
            Serial.println(result);
            buttonActive = false ;
     }
     }
                      return result;

     } // END OF REDbutton function
     
void motorON()
{
  digitalWrite(Motor, HIGH);
}

void motorOFF()
{
  digitalWrite(Motor, LOW);
}

void shutterCLOSE()
{
  digitalWrite(solenoidPowerUp, HIGH);              // Close with all the power
  // delay (5);                                        //Wait for the SOL#1 in BC position
  digitalWrite(solenoidPowerDown, HIGH);
  digitalWrite(solenoidPowerUp, LOW);               //switch to LOWPOWER and turn off POWERUP
}

void shutterOPEN()
{
  digitalWrite(solenoidPowerUp, LOW);              // Shutter OPEN
  digitalWrite(solenoidPowerDown, LOW);
}
void mirrorDOWN()
{
  motorON ();

  while (digitalRead(S5) != LOW)  //While S5 open keep the motor running and lower the mirror
    ;
  motorOFF ();
}
void mirrorUP()
{
  motorON ();

  while (digitalRead(S5) != HIGH)            //waiting for S5 to OPEN do NOTHING
    ;
  //S5 OPENS
  //S1 MAY BE OPEN NOW (DON'T KNOW HOW TO DO THIS YET)
  // Motor Brake
  motorOFF ();

}

void darkslideEJECT()
{
  mirrorUP();
  mirrorDOWN();
}

int TimeSlot() {

  int v;
  int switchState1 = 0;
  int switchState2 = 0;
  int switchState3 = 0;
  int switchState4 = 0;

  byte selector = B0000; // Variable for printing value over serial debug

  switchState1 = digitalRead(switchPin1);   // So NOW I find out the switch possition to determine the time the user wants
  switchState2 = digitalRead(switchPin2);
  switchState3 = digitalRead(switchPin3);
  switchState4 = digitalRead(switchPin4);

  if (switchState1 == HIGH) {
    bitSet(selector, 0); //sets bit 0 (rightmost bit -000(1)) to 1
  }
  else {
    bitClear(selector, 0); //sets bit 0 (rightmost bit -000(0)) to 0
  }
  if (switchState2 == HIGH) {
    bitSet(selector, 1); //sets bit 1 (second rightmost bit -00(1)0) to 1
  }
  else {
    bitClear(selector, 1); //sets bit 1 (second rightmost bit -00(0)0) to 0
  }
  if (switchState3 == HIGH) {
    bitSet(selector, 2); //sets bit 2 (second leftmost bit -0(1)00) to 1
  }
  else {
    bitClear(selector, 2); //sets bit 2 (second leftmost bit -0(0)00) to 0
  }
  if (switchState4 == HIGH) {
    bitSet(selector, 3); //sets bit 3 (leftmost bit -(1)000) to 1
  }
  else {
    bitClear(selector, 3); //sets bit 3 (leftmost bit -(0)000) to 1
  }


  v = (selector);                                   // I want the selection to be TimeSlot

  return v;
}

void Click()
{
  shutterOPEN ();  //SOLENOID OFF MAKES THE SHUTTER TO OPEN!

  ActualSlot = TimeSlot();

  if (ActualSlot == 0)                                        //THIS IS THE B "BULB" OPTION WHILE S1 (Red SHUTTER button is pressed) the shutter is OPEN
  { ;
    while (digitalRead(S1) == LOW);
    shutterCLOSE ();
    return;
  }
  // TimeSlot = 9

  if (ActualSlot == 9)
  { ;
    while (digitalRead(S1) == HIGH );                        //THIS IS THE T "TIME" OPTION PRESS ONCE S1 (Red SHUTTER button) the shutter is OPEN and AGAIN to close
    shutterCLOSE ();
    return;
  }
  delay (ShutterSpeed[ActualSlot]);                        // NOW COMES THE DELAY THAT IS THE PHOTO!

  //CLICK!

  shutterCLOSE ();                                         //close the shutter

}
