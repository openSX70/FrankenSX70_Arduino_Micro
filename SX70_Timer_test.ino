int LED1 = 5;
// int LED2 = 13;
int button = 6;

boolean ButtonState1 = false;
boolean ButtonState2 = false;

long buttonTimer = 0;
long longPressTime = 1000;
//int x;
boolean buttonActive = false;
boolean longPressActive = false;

void setup() {

  pinMode(LED1, OUTPUT);
//  pinMode(LED2, OUTPUT);
  pinMode(button, INPUT);
  Serial.begin(9600) ;


}

void loop() {

  if (digitalRead(button) == HIGH) {

    if (buttonActive == false) {

      buttonActive = true;
      buttonTimer = millis();

    }

    if ((millis() - buttonTimer > longPressTime) && (longPressActive == false)) {

      longPressActive = true;
      ButtonState1 = !ButtonState1;
      // digitalWrite(LED1, ButtonState1);
      //long press
        Serial.println("LONG\n");
        
        //delay (500);

                              
                             for( int x = 0; x<80; x++)
                            {
                            digitalWrite(LED1, HIGH);   // sets the LED on
                            delay(150-x);                  // waits a period of time
                            digitalWrite(LED1, LOW);    // sets the LED off
                            delay(100-x);                  // waits a period of time
                              Serial.println(x);
                            }



     

                            digitalWrite(LED1, HIGH);   // sets the LED on
                            delay(2000);                  // waits a period of time
                            digitalWrite(LED1, LOW);  

    }

  } else {

    if (buttonActive == true) {

      if (longPressActive == true) {

        longPressActive = false;

      } else {

        ButtonState2 = !ButtonState2;
        //digitalWrite(LED2, ButtonState2);
        //short press
        Serial.println("SHORT\n");

        digitalWrite(LED1, HIGH);   // sets the LED on
        delay(2000);                  // waits a period of time
        digitalWrite(LED1, LOW);  

      
      //  delay (500);
      }

      buttonActive = false;

    }

  }

}
