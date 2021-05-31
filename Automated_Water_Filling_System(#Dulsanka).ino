#include <LiquidCrystal.h> // includes the LiquidCrystal Library

LiquidCrystal lcd(11, 12, 4, 5, 6, 7); // Creates an LCD object. Parameters: (rs, enable, d4, d5, d6, d7)
const int TtrigPin = 9;
const int TechoPin = 10;
const int BtrigPin = 3;
const int BechoPin = 2;

const int OnTimePin = A2;
const int OffTimePin = A3;

const int RuningledPin = A4;
const int StanbyledPin = A5;

/*tank level for display persentage*/
const int TankBu = 25;//cm
const int TankBl = 100;
const int TankTu = 20;//cm
const int TankTl = 250;

/*tank level for automated process*/
const int Bu = 25;//cm
const int Bl = 40;
const int Tu = 40;
const int Tl = 180;

int emergancy = 0;
int flag = 0;
int modeB = 0;
volatile unsigned long oldtime = 0;

const int motorpowerT = A0;
const int motorpowerB = A1;
int motorstateB = 1;
int motorstateT = 1;

unsigned long runing_time = 10;
unsigned long breaking_time = 9;
unsigned long old_rvalue = 0;
unsigned long old_bvalue = 0;

long duration;
int TdistanceCm, BdistanceCm;

int Ttemp = 0;
int Btemp = 0;

void map_runing_time() {
  runing_time = map(analogRead(OnTimePin), 0, 1023, 0, 9) * 30;
  if (runing_time != old_rvalue) {
    old_rvalue = runing_time;
    if (!emergancy) {
      lcd.setCursor(4, 1);
      lcd.print("   ");
      lcd.setCursor(4, 1);
      lcd.print(runing_time);
    }
  } else {

  }
}

void map_breaking_time() {
  breaking_time = map(analogRead(OffTimePin), 0, 1023, 0, 9) * 5;
  if (breaking_time != old_bvalue ) {
    old_bvalue =  breaking_time;
    if (!emergancy) {
      lcd.setCursor(12, 1);
      lcd.print("   ");
      lcd.setCursor(12, 1);
      lcd.print(breaking_time);
    }
  } else {

  }
}


void gettoptanklevel() {
  duration = 0;
  digitalWrite(TtrigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(TtrigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(TtrigPin, LOW);
  duration = pulseIn(TechoPin, HIGH);
  TdistanceCm = duration * 0.034 / 2;
  //  Serial.print("Top : ");
  //Serial.println(TdistanceCm);
}

void getbottomtanklevel() {
  duration = 0;
  digitalWrite(BtrigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(BtrigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(BtrigPin, LOW);
  duration = pulseIn(BechoPin, HIGH);
  BdistanceCm = duration * 0.034 / 2;
  //Serial.print("bottom : ");
  //Serial.println(BdistanceCm);

}
void motorTon() {
  digitalWrite(motorpowerT, LOW);
  motorstateT = 1;
}
void motorToff() {
  digitalWrite(motorpowerT, HIGH);
  motorstateT = 0;
}

void motorBon() {
  digitalWrite(motorpowerB, LOW);
  motorstateB = 1;
}
void motorBoff() {
  digitalWrite(motorpowerB, HIGH);
  motorstateB = 0;
}
void setup() {
  Serial.begin(9600);
  lcd.begin(16, 2); //Initializes the interface to the LCD screen, and specifies the dimensions (width and height) of the display
  pinMode(TtrigPin, OUTPUT);
  pinMode(TechoPin, INPUT);
  pinMode(BtrigPin, OUTPUT);
  pinMode(BechoPin, INPUT);
  motorToff();
  motorBoff();
  pinMode(motorpowerT, OUTPUT);
  pinMode(motorpowerB, OUTPUT);
  pinMode(StanbyledPin, OUTPUT);
  pinMode(RuningledPin, OUTPUT);

  pinMode(OnTimePin, INPUT);
  pinMode(OffTimePin, INPUT);
  map_runing_time();
  map_breaking_time();

  if (runing_time  > 235 and breaking_time > 35) {
    emergancy = 1;
    runing_time = 10;
    breaking_time = 1;
  } else {
    emergancy = 0;
  }

  if (emergancy) {
    digitalWrite(RuningledPin, HIGH );
    digitalWrite(StanbyledPin, HIGH );
    lcd.clear();

    lcd.setCursor(0, 0);

    lcd.print(" Emergancy Mode ");

    lcd.setCursor(4, 1);
    lcd.print("   ");
    lcd.setCursor(4, 1);
    lcd.print(runing_time);
    lcd.setCursor(12, 1);
    lcd.print("   ");
    lcd.setCursor(12, 1);
    lcd.print(breaking_time);

    delay(500);
  } else {
    lcd.setCursor(0, 0); // Sets the location at which subsequent text written to the LCD will be displayed
    lcd.print("T1: ");
    lcd.setCursor(8, 0); // Sets the location at which subsequent text written to the LCD will be displayed
    lcd.print("T2: ");


  }
  lcd.setCursor(0, 1); // Sets the location at which subsequent text written to the LCD will be displayed
  lcd.print("RT: ");
  lcd.setCursor(8, 1); // Sets the location at which subsequent text written to the LCD will be displayed
  lcd.print("BT: ");


}

void TaskB() {
  if (!emergancy) {
    map_runing_time();
    map_breaking_time();
    if (BdistanceCm <= Bu and modeB == 1) {
      modeB = 0;
      if (motorstateB != 0) {
        motorBoff();
      }
    } else if (BdistanceCm >= Bl and modeB == 0) {
      modeB = 1;
    }
  }



  if (modeB == 1 or emergancy == 1 ) {

    if (oldtime + runing_time * 1000 < millis() and flag == 0) {
      flag = 1;
      oldtime = millis();
      motorBoff();
    } else if (oldtime + breaking_time * 1000 * 60 < millis() and flag == 1) {
      flag = 0;
      oldtime = millis();
      motorBon();
    }
  }

}


void TaskT() {
  if (TdistanceCm <= Tu) {

    if (motorstateT != 0) {
      motorToff();
    }
  } else if (TdistanceCm >= Tl ) {
    if (motorstateT != 1) {
      motorTon();
    }
  }

}


void loop() {




  if (!emergancy) {
    gettoptanklevel();
    int Tpresentage = float((TdistanceCm - TankTu) * 100) / float((TankTl - TankTu));
    Serial.println(TdistanceCm);
    if ( Ttemp != Tpresentage) {
      Ttemp = Tpresentage;
      lcd.setCursor(11, 0);
      lcd.print("     ");
      lcd.setCursor(11, 0);
      lcd.print(Tpresentage);
      lcd.print("%");
    }
    delay(100);

    getbottomtanklevel();
    int Bpresentage = float((BdistanceCm - TankBu) * 100) / float((TankBl - TankBu));

    if ( Btemp != Bpresentage) {
      Btemp = Bpresentage;
      lcd.setCursor(3, 0);
      lcd.print("     ");
      lcd.setCursor(3, 0);
      lcd.print(Bpresentage);
      lcd.print("%");
    }
    delay(100);

    if (modeB == 1) {
      digitalWrite(RuningledPin, HIGH );
      digitalWrite(StanbyledPin, LOW );
    } else {
      digitalWrite(RuningledPin, LOW);
      digitalWrite(StanbyledPin, HIGH);
    }

  } else {
    delay(200);
  }

  //TaskT();
  TaskB();

}
