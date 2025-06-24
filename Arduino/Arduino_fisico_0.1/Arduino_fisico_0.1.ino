#include <LiquidCrystal_I2C.h>
#include <Servo.h>

LiquidCrystal_I2C lcd(0x27, 20, 4); // 0x27 físico, 0x20 simulador

// Pines
int fotoPins[4] = {14, 15, 16, 17};
int pinPulso = 13;
int pinReset = 12;
int pinCuatro = 11;
int ascPins[3] = {7, 6, 5};
int descPins[3] = {4, 3, 2};

const int sensorIR1 = 22;
const int sensorIR2 = 23;
int panicPin = 18;
int buzzPin = 19;

int servoPin1 = 10;
int servoPin2 = 9;

Servo servo1, servo2;

int cuenta = -1;

// Tiempo para sirena
unsigned long lastToneTime = 0;
bool tonoAlto = false;

// Control no bloqueante servos
unsigned long tiempoServo1 = 0;
bool activo1 = false;

unsigned long tiempoServo2 = 0;
bool activo2 = false;

// MODOS DEL SISTEMA
enum ModoParqueo {
  NORMAL,
  PANICO,
  MANTENIMIENTO,
  NOCTURNO,
  EVACUACION
};

ModoParqueo modoActual = NORMAL;
bool modoYaSeleccionado = false;

void setup() {
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Grupo 15");
  lcd.setCursor(0, 1);
  lcd.print("Orga");

  for (int i = 0; i < 4; i++) pinMode(fotoPins[i], INPUT);
  for (int i = 0; i < 3; i++) {
    pinMode(ascPins[i], INPUT);
    pinMode(descPins[i], INPUT);
  }

  pinMode(pinPulso, OUTPUT);
  pinMode(pinReset, OUTPUT);
  pinMode(pinCuatro, OUTPUT);
  digitalWrite(pinPulso, HIGH);
  digitalWrite(pinReset, HIGH);
  digitalWrite(pinCuatro, LOW);

  pinMode(sensorIR1, INPUT);
  pinMode(sensorIR2, INPUT);
  pinMode(panicPin, INPUT);
  pinMode(buzzPin, OUTPUT);
  digitalWrite(buzzPin, LOW);

  servo1.attach(servoPin1, 500, 2500);
  servo2.attach(servoPin2, 500, 2500);
  servo1.write(0);
  servo2.write(0);

  Serial.begin(9600);
}

void loop() {
  // Establecer modo normal por defecto si aún no se ha seleccionado otro
  if (!modoYaSeleccionado) {
    modoActual = NORMAL;
  }

  if (digitalRead(panicPin) == HIGH && modoActual == NORMAL) {
    Serial.println("ModoPanicoActivado");
    //solo para el simulador
    modoActual = PANICO;
    modoYaSeleccionado = true;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("  MODO PANICO ");
    lcd.setCursor(0, 1);
    lcd.print("Ambas cerradas");
  }

  // Leer comandos Serial
  if (Serial.available()) {
    String comando = Serial.readStringUntil('\n');
    comando.trim();
    
//Activar modos a traves de el serial

//NORMAL
    if (comando == "ModoNormalActivado") {
      modoActual = NORMAL;
      modoYaSeleccionado = true;
      cuenta = -1;

//PANICO
    } else if (comando == "ModoPanicoActivado" && modoActual == NORMAL) {
      modoActual = PANICO;
      modoYaSeleccionado = true;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("  MODO PANICO ");
      lcd.setCursor(0, 1);
      lcd.print("Ambas cerradas");
    
//MANTENIMIENTO
    } else if (comando == "ModoMantenimientoActivado" && modoActual == NORMAL) {
      modoActual = MANTENIMIENTO;
      modoYaSeleccionado = true;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(" MANTENIMIENTO ");
      lcd.setCursor(0, 1);
      lcd.print("Solo salida ON ");

//NOCTURNO
    } else if (comando == "ModoNocturnoActivado" && modoActual == NORMAL) {
      modoActual = NOCTURNO;
      modoYaSeleccionado = true;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("   MODO NOCHE   ");
      lcd.setCursor(0, 1);
      lcd.print("Ambas cerradas");

//EVACUACION
    } else if (comando == "ModoEvacuacionActivado" && modoActual == NORMAL) {
      modoActual = EVACUACION;
      modoYaSeleccionado = true;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(" EVACUACION!! ");
      lcd.setCursor(0, 1);
      lcd.print("Ambas abiertas");
    } else {
      Serial.println("ERROR MODO =C");
    }
  }

  // --- COMPORTAMIENTO POR MODO ---
  if (modoActual == PANICO) {
    servo1.write(0);
    servo2.write(0);

    unsigned long tiempoActual = millis();
    if (tiempoActual - lastToneTime >= 300) {
      lastToneTime = tiempoActual;
      tone(buzzPin, tonoAlto ? 1000 : 1500);
      tonoAlto = !tonoAlto;
    }

    return;
  }

  if (modoActual == MANTENIMIENTO) {
    servo1.write(0); // entrada cerrada
    noTone(buzzPin);

    // salida funcional
    bool lectura2 = digitalRead(sensorIR2);
    if (lectura2 == LOW) {
      servo2.write(90);
      activo2 = true;
      tiempoServo2 = millis();
    } else if (activo2 && millis() - tiempoServo2 >= 3000) {
      servo2.write(0);
      activo2 = false;
    }

    return;
  }

  if (modoActual == NOCTURNO) {
    servo1.write(0);
    servo2.write(0);
    noTone(buzzPin);

    return;
  }

  if (modoActual == EVACUACION) {
    servo1.write(90);
    servo2.write(90);

    unsigned long tiempoActual = millis();
    if (tiempoActual - lastToneTime >= 300) {
      lastToneTime = tiempoActual;
      tone(buzzPin, tonoAlto ? 1200 : 800);
      tonoAlto = !tonoAlto;
    }

    return;
  }

  // MODO NORMAL
  noTone(buzzPin);

  // Contador de ocupados
  int ocupadosAct = 0;
  for (int i = 0; i < 4; i++) {
    if (digitalRead(fotoPins[i]) == LOW) {
      ocupadosAct++;
    }
  }

  if (ocupadosAct != cuenta) {
    digitalWrite(pinReset, LOW);
    delay(100);
    digitalWrite(pinCuatro, LOW);
    digitalWrite(pinReset, HIGH);
    delay(250);
    digitalWrite(pinCuatro, HIGH);

    for (int i = 0; i < ocupadosAct; i++) {
      digitalWrite(pinPulso, LOW);
      delay(100);
      digitalWrite(pinPulso, HIGH);
      delay(100);
    }

    cuenta = ocupadosAct;
    int valorAsc = (digitalRead(ascPins[2])) * 4 + (digitalRead(ascPins[1])) * 2 + (digitalRead(ascPins[0])) * 1;
    int valorDesc = (digitalRead(descPins[2])) * 4 + (digitalRead(descPins[1])) * 2 + (digitalRead(descPins[0])) * 1;

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Ocupados: ");
    lcd.print(valorAsc);
    lcd.setCursor(0, 1);
    lcd.print("Disponibles: ");
    lcd.print(valorDesc);

    Serial.println("Ocupados: " + String(valorAsc) + ", Libres: " + String(valorDesc));
  }

  // --- Servo 1 (entrada) ---
  bool lectura1 = digitalRead(sensorIR1);
  if (lectura1 == LOW) {
    servo1.write(90);
    activo1 = true;
    tiempoServo1 = millis();
  } else if (activo1 && (millis() - tiempoServo1 >= 3000)) {
    servo1.write(0);
    activo1 = false;
  }

  // --- Servo 2 (salida) ---
  bool lectura2 = digitalRead(sensorIR2);
  if (lectura2 == LOW) {
    servo2.write(90);
    activo2 = true;
    tiempoServo2 = millis();
  } else if (activo2 && (millis() - tiempoServo2 >= 3000)) {
    servo2.write(0);
    activo2 = false;
  }

  delay(250);
}
