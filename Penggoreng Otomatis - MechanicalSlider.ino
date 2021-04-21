/*Penggoreng Otomatis

  ##Modul yang digunakan

  - Motor Stepper
  - Load Cell XH711
  - Limit Switch
  - Joy Stick
  - Arduino Uno
  
  ##Pin yang Digunakan
  - 0   = no-connect
  - 1   = no-connect
  - 2   = no-connect
  - 3   = no-connect
  - 4   = no-connect
  - 5   = Step motor X   
  - 6   = Step motor Rotary
  - 7   = Step motor y
  - 8   = Direction Motor X
  - 9   = Direction Mootr Rotary
  - 10  = Direction Motor Y
  - 11  = X Limit Switch
  - 12  = Y Limit Switch
  - 13  = Mode Opsi Auto / Manual
  
  - A0  = OUT Load Cell 
  - A1  = CLK Load Cell
  - A2  = X Value Joystick
  - A3  = y Value Joystick
  - A4  = no-connect
  - A5  = no-connect
*/
  
// ====================================                   Library                  ====================================

  #include <SoftwareSerial.h>
  #include "HX711.h"
  #include "AccelStepper.h" 
  
// ====================================         Inisialisasi Pin Arduino           ====================================

  #define EnaR          2
  #define EnaY          3
  
  #define Peniris       4
  
  #define MotorX        5
  #define MotorY        7
  #define MotorR        6

  #define dirX          8
  #define dirY          10
  #define dirR          9  

  #define endX          11
  #define endY          12

  #define opsi          13  

  #define DOUT          A0
  #define CLK           A1

  #define Xin           A2
  #define Yin           A3

// ====================================                  Variabel                   ====================================

  int beban;
  int menit = 0;
  int detik = 0;
  int GRAM;
  int xVal; 
  int yVal;

// ====================================              Variabel Timer                ====================================

  int timergoreng = 2;//
  int timerpeniris = 1; // hitungan berupa menit , timerpeniris akan dikali 60 detik    
  
  unsigned long timerMilis=0;
  unsigned long Milisterkini=0;
  
// ==================================== Variabel Factor Pengkali XH711 / Load Cell ====================================

  int bebanbahan = 15;
  
  float   calibration_factor = 440;

// ====================================               Kondisi Input               ====================================
  boolean otomatis = false;
  boolean internaltimer = false;

// ====================================             Kondisi Input Tombol          ====================================
    
  boolean currentState1 = LOW;
  boolean lastState1 = LOW;

  boolean currentState2 = LOW;
  boolean lastState2 = LOW;
  
  boolean currentState3 = LOW;
  boolean lastState3 = LOW;

// ====================================         Variabel Motor Stepper           ====================================
  
  int move_finished=1;  // Used to check if move is completed
  int max_x = 3050;
  int move1_x = 1500;
  int max_y = 2300;
  int x= 0;
  int y= 0;
  int r= 0;
  int i;
  long initial_homing=-1;  // Used to Home Stepper at startup
  long move_x = 0;
  long move_y = 0;
  
// ====================================          Setup Fungsi Library           ====================================
  
  HX711 scale   (DOUT, CLK);

  AccelStepper stepperR(1, 6, 9);    
  AccelStepper stepperX(1, 5, 8);   
  AccelStepper stepperY(1, 7, 10);  

    
  
  
// ====================================                  Setup                  ====================================

void setup(){

  Serial.begin(9600);
  
  pinMode(EnaR    , OUTPUT);
  pinMode(EnaY    , OUTPUT);
  
  pinMode(Peniris , OUTPUT);

  pinMode(endX    , INPUT);
  pinMode(endY    , INPUT);
  pinMode(opsi    , INPUT);
  
  pinMode(Xin     , INPUT);
  pinMode(Yin     , INPUT);
  
  stepperR.setPinsInverted(false,false,true);
  stepperR.setEnablePin(EnaR);

  scale.set_scale ();
  scale.tare      ();
  scale.set_scale (calibration_factor);

  delay(10);
  
  Serial.println("Memposisikan All Motor Posisi Standby");

  stepperX.setMaxSpeed(1000.0);      // Set Max Speed of Stepper (Slower to get better accuracy)
  stepperX.setAcceleration(1000.0);  // Set Acceleration of Stepper
  
  stepperY.setMaxSpeed(1000.0);      // Set Max Speed of Stepper (Slower to get better accuracy)
  stepperY.setAcceleration(1000.0);  // Set Acceleration of Stepper
  
  stepperX.setCurrentPosition(0);  // Set Zero X
  stepperY.setCurrentPosition(0);  // Set Zero Y
  
  /*y = -1;
  while (y != 0) { // Jika Limit Swtch X belum tertekan   
    if (digitalRead(endY) == HIGH){
      y = 0;
    }*/
  while (digitalRead(endY) != HIGH){  
    for( i = 25 ; i>0 ; i-- ){
    stepperY.moveTo(initial_homing);  // Set the position to move to
    initial_homing--;  // Decrease by 1 for next move if needed
    stepperY.run();  // Start moving the stepper
    Serial.println("Homing Motor Y");
    }
    delay(5);
    }

  Serial.println("Motor Y Zero Position");
  
  /*x = -1;
  while (x != 0) {
    if (digitalRead(endX) == HIGH){
      //x = 0;
    }*/
  while (digitalRead(endX) != HIGH){
    for( i = 25 ; i>0 ; i-- ){  
    stepperX.moveTo(initial_homing);  // Set the position to move to
    initial_homing--;  // Decrease by 1 for next move if needed
    stepperX.run();  // Start moving the stepper
    Serial.println("Homing Motor X");
    }
    delay(5);
  }
  
  Serial.println("Motor X Zero Position");
  
  delay(10);
  
  stepperX.setCurrentPosition(0);  // Set Zero X
  stepperY.setCurrentPosition(0);  // Set Zero Y
  stepperR.setCurrentPosition(0);  // Set Zero R

  Serial.println("Semua Motor Pada Posisi Standby");
  
  stepperR.disableOutputs();
}

// ====================================               Program Utama             ====================================

void loop(){

// ====================================       Membaca Kondisi Switch Mode       ====================================   
  if (internaltimer){
    Milisterkini = (millis()/1000);   
    if ((Milisterkini - timerMilis) > (timerpeniris*60)){
        Serial.println("Mematikan Peniris");
        digitalWrite(Peniris, LOW);
        internaltimer = false;    
        }
    }
    
  currentState3 = digitalRead(opsi);
  if (currentState3 == HIGH && lastState3 == LOW){
      Serial.println("Ganti Mode");                   // Jika ON maka akan ke mode Otomatis 
      otomatis = true;
      }
      
      
 lastState3 = currentState3;
 
 while (analogRead(Yin) >= 600 ) {
    if ( move_y < max_y){
      stepperY.setMaxSpeed    (1000);
      stepperY.setAcceleration(1000);
      stepperY.moveTo(move_y);
      stepperY.run();
      move_y++;
      delay(5);
      }
    }

 while (analogRead(Yin) <= 400 ) {
    if ( move_y > 0){
      stepperY.setMaxSpeed    (1000);
      stepperY.setAcceleration(1000);
      stepperY.moveTo(move_y);
      stepperY.run();
      move_y--;
      delay(5);
      }
    }

 while (analogRead(Xin) >= 600 ) {
    if ( move_x < max_x){
      stepperX.setMaxSpeed    (1000);
      stepperX.setAcceleration(1000);
      stepperX.moveTo(move_x);
      stepperX.run();
      move_x++;
      delay(5);
      }
    }

 while (analogRead(Xin) <= 400 ) {
    if ( move_x > 0){
      stepperX.setMaxSpeed    (500);
      stepperX.setAcceleration(500);
      stepperX.moveTo(move_x);
      stepperX.run();
      move_x--;
      delay(5);
      }
    }
// ====================================              Mode Otomatis              ====================================

if (otomatis){

  //---------------------------------------- Speed Tiap Motor ----------------------------------------
  
  stepperX.setMaxSpeed(1000.0);      // Set Max Speed of Stepper (Slower to get better accuracy)
  stepperX.setAcceleration(1000.0);  // Set Acceleration of Stepper
  
  stepperY.setMaxSpeed(1000.0);      // Set Max Speed of Stepper (Slower to get better accuracy)
  stepperY.setAcceleration(1000.0);  // Set Acceleration of Stepper

  stepperR.setMaxSpeed(300.0);      // Set Max Speed of Stepper (Slower to get better accuracy)
  stepperR.setAcceleration(300.0);  // Set Acceleration of Stepper

  //---------------------------------------- Homing ke titik 0 / Switch tertekan ----------------------------------------
  y = -1;
  while (y != 0) { // Jika Limit Swtch X belum tertekan   
    if (digitalRead(endY) == HIGH){
      y = 0;
    }
    for( i = 25 ; i>0 ; i-- ){
      stepperY.moveTo(initial_homing);  // Set the position to move to
      initial_homing--;  // Decrease by 1 for next move if needed
      stepperY.run();  // Start moving the stepper
      Serial.println("Homing Motor Y");
    }
    delay(5);
    }

  Serial.println("Motor Y Zero Position");
  
  x = -1;
  while (x != 0) {
    if (digitalRead(endX) == HIGH){
      x = 0;
    }
    for( i = 25 ; i>0 ; i-- ){
      stepperX.moveTo(initial_homing);  // Set the position to move to
      initial_homing--;  // Decrease by 1 for next move if needed
      stepperX.run();  // Start moving the stepper
      Serial.println("Homing Motor X");
    }
    delay(5);
  }
  
  Serial.println("Motor X Zero Position");  
  Serial.println("Posisi Standby Menunggu Loyang Penuh");
  
  delay(10);
  
  stepperR.disableOutputs();
  //---------------------------------------- Menunggu Beban Bahan Terpenuhi  ----------------------------------------

  beban = -1;
  while (beban != 0){
  // Mmebaca Gram  
  GRAM = scale.get_units(), 4;
  Serial.print("Beban Bahan : ");
  Serial.print(GRAM);
  Serial.println("GRAM");
  
  if (GRAM > bebanbahan){ // Hitungan Gram
      beban = 0; // Jika Beban melebihi maka kondisi terpenuhi
  }
  currentState3 = digitalRead(opsi);
  //Timer akan membaca apabila Timer peniris belum selesai saat sudah pada posisi standby
  if (internaltimer){
      Milisterkini = (millis()/1000);   
      if ((Milisterkini - timerMilis) > (timerpeniris*60)){
         Serial.println("Mematikan Peniris");
         digitalWrite(Peniris, LOW);
         internaltimer = false;    
         }
      }
  //Apabila Mode Dirubah saat menggu loyang penuh
  if (currentState3 == LOW){
      Serial.println("Ganti Mode");
      otomatis = false;               // Apabila Saat Menunngu Bahan Switch di Off kan, akan keluar ke mode Manual / Standby
      return;
      }
  delay(100);
  }
  
  lastState3 = currentState3;
    
  delay(10);    

  stepperR.enableOutputs();
      
  stepperX.setCurrentPosition(0);  // Set Zero X
  stepperY.setCurrentPosition(0);  // Set Zero Y
  stepperR.setCurrentPosition(0);  // Set Zero Y
  
  //---------------------------------------- BEBAN PENUH MEMULAI MENGGORENG  ----------------------------------------

  stepperX.runToNewPosition(1500); // Motor Kearah penggoreng 

  delay (750);

  stepperR.runToNewPosition(-250); // Menggendorkan Loyang 

  delay (750);

  stepperY.setMaxSpeed(600.0);      // Set Max Speed of Stepper (Slower to get better accuracy)
  stepperY.setAcceleration(600.0);  // Set Acceleration of Stepper
  stepperY.runToNewPosition(2650);  // Menurunkan Loyang //2300

  delay(750);

  //---------------------------------------- Timer Penggoreng  ----------------------------------------

    Serial.println("Memulai Menggoreng");
    menit = timergoreng;
    for ( menit ; menit >= 0; menit-- ){
      for ( detik ; detik >= 0; detik-- ){
        Serial.print("Sisa Waktu Menggoreng : ");
        Serial.print(menit);
        Serial.print(" menit ");
        Serial.print(detik);
        Serial.println(" detik.");
 
          if (internaltimer){
            Milisterkini = (millis()/1000);   
            if ((Milisterkini - timerMilis) > (timerpeniris*60)){
               Serial.println("Mematikan Peniris");
               digitalWrite(Peniris, LOW);
               internaltimer = false;    
               }
            }
            
        delay(1000); // 1000 Delay 1 Detik
        
      }
    detik = 59;  
    }
    
  //---------------------------------------- Timer Penggoreng  ----------------------------------------

  stepperY.runToNewPosition(0);//loyang keposisi semula
  Serial.println("Motor Y Zero Position");

  delay(750);

  stepperR.runToNewPosition(0);//loyang keposisi semula
  delay(500); 
  stepperX.runToNewPosition(3000); // Motor Kearah peniris 

  delay(750);

  stepperR.runToNewPosition(-100);//menggendorkan loyang
  delay(500); 
  stepperY.runToNewPosition(600); // Motor Y menurukan loyang
  delay(500);
  stepperR.runToNewPosition(325);//Menarik Loyang
  delay(500); 
  stepperY.runToNewPosition(950); // Motor Y menurukan loyang
  delay(1500);
  stepperR.runToNewPosition(300);//menggendorkan loyang
  delay(500);
  stepperR.runToNewPosition(200);//menggendorkan loyang
  delay(500);
  stepperY.runToNewPosition(300); // Motor Y menurukan loyang
  delay(500);
  stepperR.runToNewPosition(100);//menggendorkan loyang
  delay(500);
  stepperR.runToNewPosition(-100);//menggendorkan loyang
  delay(500);
  stepperY.runToNewPosition(0); // Motor Y menurukan loyang
  delay(500);
  stepperR.runToNewPosition(0);//menggendorkan loyang

  delay(500);
  
  timerMilis = (millis()/1000);   //Menggunakan Internal Timer Sebagai pewaktu Peniris sambil Mesin ke posisi Standby
  digitalWrite(Peniris , HIGH);
  internaltimer = true;
    
  x = -1;
  while (x != 0) {
    if (digitalRead(endX) == HIGH){
      x = 0;
    }
    for( i = 25 ; i>0 ; i-- ){
      stepperX.moveTo(initial_homing);  // Set the position to move to
      initial_homing--;  // Decrease by 1 for next move if needed
      stepperX.run();  // Start moving the stepper
      Serial.println("Homing Motor X");
      delay(5);
      }
    if (internaltimer){
      Milisterkini = (millis()/1000);   
      if ((Milisterkini - timerMilis) > (timerpeniris*60)){
         Serial.println("Mematikan Peniris");
         digitalWrite(Peniris, LOW);
         internaltimer = false;    
         }
      }
    }
    
  delay(500);      
  //---------------------------------------- Mode Otomatis selesai, kembali keawal ----------------------------------------
  }
}

// ====================================                Selesai                  ====================================
