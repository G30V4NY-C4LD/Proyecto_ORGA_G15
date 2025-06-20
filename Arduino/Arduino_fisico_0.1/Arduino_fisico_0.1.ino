#include <LiquidCrystal_I2C.h>
#include <Servo.h>

// LCD I2C 0x20, 20 columnas, 4 filas
LiquidCrystal_I2C lcd(0x27, 20, 4);

// Fotorresistencias
int fotoPins[4] = {2, 3, 4, 5};
int pinPulso = 13;
int pinReset = 12;
int ocupados = 0;
int cuenta = -1; 

// Sensores infrarrojos
const int sensorIR1 = 6;
const int sensorIR2 = 7;

// Servos
Servo servo1, servo2;
int servoPin1 = 10;
int servoPin2 = 11;

//tiempo de 3 segundos
unsigned long tiempo1 = 0;
unsigned long tiempo2 = 0;

bool activo1 = 0;
bool activo2 = 0;

// Estados y tiempos para sensores IR
bool activado1 = false;
bool activado2 = false;

void setup() {
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Grupo 15");
  lcd.setCursor(0, 1);
  lcd.print("Orga");

  for (int i = 0; i < 4; i++) {
    pinMode(fotoPins[i], INPUT);
  }

  pinMode(pinPulso, OUTPUT);
  pinMode(pinReset, OUTPUT);
  digitalWrite(pinPulso, HIGH);
  digitalWrite(pinReset, HIGH);

  pinMode(sensorIR1, INPUT);
  pinMode(sensorIR2, INPUT);

  servo1.attach(servoPin1, 500, 2500);
  servo2.attach(servoPin2, 500, 2500);

  servo1.write(0);
  servo2.write(0);

  Serial.begin(9600);
}

void loop() {

  // Leer sensores de luz para conteo
  int ocupadosAct = 0;
  for (int i = 0; i < 4; i++) {
    if (digitalRead(fotoPins[i]) == LOW) {
      ocupadosAct++;
      //Serial.print(ocupadosAct);
    }
  }

  if (ocupadosAct != cuenta) {
    digitalWrite(pinReset, LOW);
    delay(100);
    digitalWrite(pinReset, HIGH);

    for (int i = 0; i < ocupadosAct; i++) {
      digitalWrite(pinPulso, LOW);
      delay(100);
      digitalWrite(pinPulso, HIGH);
      delay(100);
    }
    cuenta = ocupadosAct;

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Ocupados: ");
    lcd.print(cuenta);
    lcd.setCursor(0, 1);
    lcd.print("Disponibles: ");
    lcd.print(4 - cuenta);
  }

int lectura1 = digitalRead(sensorIR1);

if (lectura1 == HIGH){
  servo1.write(90);
  activo1 = 1;
}else if(activo1){
  delay(3000);servo1.write(0);
  activo1 = 0;
}

int lectura2 = digitalRead(sensorIR2);

if (lectura2 == HIGH){
  servo2.write(90);
  activo2 = 1;
}else if(activo2){
  delay(3000);servo2.write(0);
  activo2 = 0;
}


 /* // --- Control servo1 ---
  int lectura1 = digitalRead(sensorIR1);
  if (lectura1 == HIGH){
    tiempo1 = 3000;
    servo1.write(90);
  }else{
    if (tiempo1 <= 0){
      servo1.write(0);
    }else{
      tiempo1 - 250; //tiene que ser el mismo tiempoque en el delay
    }
  }*/

  /*// --- Control servo2 ---
  int lectura2 = digitalRead(sensorIR2);
  if (lectura2 == HIGH){
    tiempo2 = 3000;
    servo2.write(90);
  }else if(tiempo2 == 0){
      servo2.write(0);
  }else if(tiempo2 < 0){
      tiempo2 = 0;
  }else{
    tiempo2 - 250; //tiene que ser el mismo tiempoque en el delay
  }*/
  delay(250);
}
