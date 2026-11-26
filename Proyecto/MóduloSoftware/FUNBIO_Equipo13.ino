#include <OneWire.h>
#include <DallasTemperature.h>

// FUNBIO - CONTROL DE 3 ELECTROIMANES CON 3 SENSORES DS18B20
const byte IMAN1 = 2;
const byte IMAN2 = 3;
const byte IMAN3 = 4;
const byte SENSOR_PIN = 5;
const byte NUM_SENSORES = 3;
const float LIMITE = 30.0;
const float REARME = 28.0;

OneWire oneWire(SENSOR_PIN);
DallasTemperature sensores(&oneWire);
DeviceAddress direcciones[NUM_SENSORES];
bool bloqueado = false;
bool habilitado = false;
bool sensoresOK = false;

void apagarImanes() {
  digitalWrite(IMAN1, LOW);
  digitalWrite(IMAN2, LOW);
  digitalWrite(IMAN3, LOW);
}

void encenderImanes() {
  digitalWrite(IMAN1, HIGH);
  digitalWrite(IMAN2, HIGH);
  digitalWrite(IMAN3, HIGH);
}

void setup() {
  digitalWrite(IMAN1, LOW);
  digitalWrite(IMAN2, LOW);
  digitalWrite(IMAN3, LOW);
  pinMode(IMAN1, OUTPUT);
  pinMode(IMAN2, OUTPUT);
  pinMode(IMAN3, OUTPUT);
  apagarImanes();
  Serial.begin(9600);
  sensores.begin();
  sensores.setResolution(10);
  Serial.println("==========================");
  Serial.println(" SISTEMA FUNBIO INICIADO");
  Serial.println("==========================");
  byte cantidad = sensores.getDS18Count();
  Serial.print("Sensores encontrados: ");
  Serial.println(cantidad);
  sensoresOK = (cantidad == NUM_SENSORES);
  if (sensoresOK) {
    for (byte i = 0; i < NUM_SENSORES; i++) {
      if (!sensores.getAddress(direcciones[i], i)) {
        sensoresOK = false;
        break;
      }
    }
  }
  if (!sensoresOK) {
    bloqueado = true;
    Serial.println("ERROR: Se requieren 3 sensores");
    Serial.println("SISTEMA BLOQUEADO");
  }
}

void loop() {
  // Si hubo un fallo, mantener apagados
  if (bloqueado) {
    apagarImanes();
    delay(500);
    return;
  }
  // Comprobar cantidad de sensores
  if (sensores.getDS18Count() != NUM_SENSORES) {
    bloqueado = true;
    apagarImanes();
    Serial.println("ERROR: Sensor desconectado");
    Serial.println("SISTEMA BLOQUEADO");
    return;
  }
  sensores.requestTemperatures();
  bool lecturasValidas = true;
  bool sobretemperatura = false;
  bool temperaturaInicialOK = true;
  for (byte i = 0; i < NUM_SENSORES; i++) {
    float temp = sensores.getTempC(direcciones[i]);
    Serial.print("Sensor ");
    Serial.print(i + 1);
    Serial.print(": ");
    bool valida = temp != DEVICE_DISCONNECTED_C && !isnan(temp) &&
                  temp >= -55.0 && temp <= 125.0;
    if (!valida) {
      Serial.print("ERROR");
      lecturasValidas = false;
    } else {
      Serial.print(temp);
      Serial.print(" C");
      if (temp >= LIMITE) sobretemperatura = true;
      if (temp > REARME) temperaturaInicialOK = false;
    }
    Serial.print(" | ");
  }
  if (!lecturasValidas || sobretemperatura) {
    bloqueado = true;
    habilitado = false;
    apagarImanes();
    Serial.println("ELECTROIMANES APAGADOS");
    Serial.println("SISTEMA BLOQUEADO");
    return;
  }
  if (!habilitado && temperaturaInicialOK) {
    habilitado = true;
    Serial.println("TEMPERATURA SEGURA");
  }
  if (habilitado) {
    encenderImanes();
    Serial.println("Electroimanes: ENCENDIDOS");
  } else {
    apagarImanes();
    Serial.println("Esperando enfriamiento...");
  }
  delay(300);
}
