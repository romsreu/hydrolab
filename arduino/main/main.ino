
/**************************************************************
 *                     PROYECTO HIDROPONIA
 *------------------------------------------------------------
 * IMPORTANTE:
 *   - En el puerto serie SOLO se debe imprimir el JSON, 
 *     cualquier otra impresión podría corromper la comunicación.
 *   - Si se desea mostrar debug o imprimir variables, 
 *     se debe PAUSAR la comunicación con la RPi mientras tanto.
 *
 *   Esto asegura que la RPi reciba datos consistentes y 
 *   evita errores en la transmisión de datos.
 *
 *------------------------------------------------------------
 * ESTRUCTURA DEL CÓDIGO:
 *   1) TABLA DE PINES:
 *        - Aquí se listan todos los pines usados.
 *        - Sirve como referencia rápida (no modificar salvo
 *          que cambie el hardware).
 *
 *   2) PARÁMETROS DE CONFIGURACIÓN:
 *        - Tiempos, umbrales, consignas, etc.
 *        - SOLO esta sección es la pensada para que el usuario
 *          la modifique fácilmente.
 *
 *   3) LÓGICA DE FUNCIONAMIENTO:
 *        - Acá está el funcionamiento y lógica del sistema.
 *        - ¡NO TOCAR! (a menos que quieras cambiar el 
 *          comportamiento del sistema).
 *
 **************************************************************/

/* Mapa de pines:
   https://hydrolabutn.netlify.app/
   También se encuentra en el documento LAHI4.0.pdf dentro de
   https://github.com/romsreu/hydrolab/tree/main/docs */

// Pinout de referencia:
// https://www.electronicshub.org/wp-content/uploads/2021/01/Arduino-Mega-Pinout.jpg
  


// =====================================================
//           PARÁMETROS DE CONFIGURACIÓN (EDITABLES)
// =====================================================

// ---- Humedad y Temperatura ----
float LHum        = 80.0;   // [%] Límite de humedad relativa (por ahora solo referencia)
float limit_temp  = 22;     // [°C] Temperatura límite

// ---- pH ----
float pH_low      = 6;      // [pH] Valor mínimo aceptable
float pH_high     = 8;      // [pH] Valor máximo aceptable
float pH_value    = 0;      // [pH] Valor medido (se actualiza en ejecución)

// ---- Conductividad Eléctrica (EC) ----
float EC_low      = 0;      // [mS/cm] Valor mínimo aceptable
float EC_high     = 0;      // [mS/cm] Valor máximo aceptable
float EC_value    = 0;      // [mS/cm] Valor medido (se actualiza en ejecución)

// ---- Ventilación ----
float vent_time   = 0;      // [ms] Duración de ventilación (configurable si se usa control activo)


// =====================================================
//           VARIABLES DE TIEMPO Y MUESTREO
// =====================================================

const float MIN_TO_MILLISECONDS = 60000;

// ---- Tiempo general ----
unsigned long currentMillis = 0; // Variable de tiempo general, se actualiza al inicio del loop

// ---- Impresión Serial ----
unsigned long previousMillis_print = 0;
unsigned long interval_print       = 1 * MIN_TO_MILLISECONDS; // [ms] Intervalo entre prints en consola (1 seg)

// ---- pH ----
unsigned long previousMillis_pH    = 0;
unsigned long interval_pH          = 10 * MIN_TO_MILLISECONDS; // [ms] Intervalo de lectura (el primer valor representa los minutos)
int pH_samples                     = 5;            // [#] Cantidad de muestras por lectura

// ---- EC ----
unsigned long previousMillis_EC    = 0;
unsigned long interval_EC          = 0 * MIN_TO_MILLISECONDS;   // [ms] Intervalo de lectura (el primer valor representa los minutos)
int EC_samples                     = 0;           // [#] Cantidad de muestras por lectura

// ---- Temperatura ambiente ----
unsigned long previousMillis_temp  = 0;
unsigned long interval_temp        = 1 * MIN_TO_MILLISECONDS;    // [ms] Intervalo de lectura (el primer valor representa los minutos)
int temp_samples                   = 0;           // [#] Cantidad de muestras por lectura

// ---- Temperatura agua ----
unsigned long previousMillis_Wtemp = 0;
unsigned long interval_Wtemp       = 1 * MIN_TO_MILLISECONDS; // [ms] Intervalo de lectura (el primer valor representa los minutos)

// ---- Luz ambiente (LDR) ----
unsigned long previousMillis_LDR   = 0;
const long interval_LDR            = 15 * MIN_TO_MILLISECONDS; // [ms] Intervalo de lectura (el primer valor representa los minutos)

// ---- LEDs ----
unsigned long previousMillis_LED   = 0;
unsigned long interval_LED_on      = 2 * MIN_TO_MILLISECONDS;   // [ms] Tiempo encendido LED (el primer valor representa los minutos)
unsigned long interval_LED_off     = 1 * MIN_TO_MILLISECONDS;   // [ms] Tiempo apagado LED (el primer valor representa los minutos)

// ---- Bomba principal ----
unsigned long previousMillis_bomba = 0;
unsigned long interval_bomba_on    = 1 * MIN_TO_MILLISECONDS;   // [ms] Tiempo encendida (el primer valor representa los minutos)
unsigned long interval_bomba_off   = 2 * MIN_TO_MILLISECONDS;   // [ms] Tiempo apagada (el primer valor representa los minutos)

// ---- Display LCD ----
unsigned long previousMillis_display = 0;
const unsigned long interval_display = 8000;
int screenIndex = 0;
const int totalScreens = 4;
// =====================================================
// =====================================================
// ================   ¡NO MODIFICAR!   =================
// =====================================================
// =====================================================

// A partir de esta sección se definen las LIBRERÍAS, 
// DECLARACIONES, VARIABLES DE ESTADO y PARÁMETROS 
// INTERNOS del sistema.
// -----------------------------------------------------
// El usuario NO debe editar nada aquí, salvo que desee 
// cambiar la lógica de funcionamiento interna.
// -----------------------------------------------------
// =====================================================




// =====================================================
//                   LIBRERÍAS
// =====================================================
#include <DHT.h>                // Sensor de temperatura y humedad DHT22
#include <OneWire.h>            // Sensor de temperatura DS18B20
#include <DallasTemperature.h>  // Librería para DS18B20
#include <ArduinoJson.h>        // Manejo de datos en formato JSON
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x3F, 20, 4);



// =====================================================
//                 DECLARACIONES DE PINES
// =====================================================

// --- Salidas digitales ---
#define v1   23  // Ventilador 1 (columna izquierda)
#define v2   25  // Ventilador 2 (columna central)
#define v3   27  // Ventilador 3 (columna derecha)
#define v0   29  // Ventilador LED
#define b1   31  // Bombita tanque 1
#define b2   33  // Bombita tanque 2
#define b3   35  // Bombita tanque 3
#define b4   37  // Bombita tanque 4
#define LED1 39  // LED estante arriba
#define LED2 41  // LED estante abajo
#define b0   43  // Bomba principal

// --- Entradas digitales ---
#define c1 45 // Caudalímetro 1
#define c2 47 // Caudalímetro 2
#define c3 49 // Caudalímetro 3
#define c4 51 // Caudalímetro 4  

// --- Sensores ---
#define DHTPIN_ext 2   // DHT22 exterior
#define DHTPIN_int 3   // DHT22 interior
#define Wtemp_pin  4   // DS18B20 (dos conectados al mismo pin)
#define LDRpin     A0  // Sensor LDR (iluminación)
#define pHPin      A1  // Sensor pH (simulado con potenciómetro)
#define ECPin      A2  // Sensor EC (simulado con potenciómetro)


// =====================================================
//                 CONFIGURACIÓN DE SENSORES
// =====================================================

// --- DHT22 ---
#define DHTTYPE DHT22
DHT dht_int(DHTPIN_int, DHTTYPE);   // Sensor interior
DHT dht_ext(DHTPIN_ext, DHTTYPE);   // Sensor exterior
float temp_value_int = 0, temp_value_ext = 0;
float hum_value_int  = 0, hum_value_ext = 0;
float Wtemp0_value   = 0, Wtemp1_value = 0;

// --- LDR ---
int LDRvalue = 0;
int nivelIluminacion = 0;

// --- DS18B20 ---
OneWire oneWire(Wtemp_pin);
DallasTemperature DS18B20(&oneWire);

// --- JSON ---
const size_t JSON_BUFFER_SIZE = JSON_OBJECT_SIZE(50);

// --- pH ---
float offset = 0.00;   // Ajustar en calibración

// --- EC ---
float VREF = 5.0;              // Voltaje de referencia (5V o 3.3V)
float coef = 1.0;              // Factor de calibración (ajustar con solución patrón)
float tempCoef = 0.0185;       // Coef. de compensación por °C
float refTemp = 25.0;          // Temperatura de referencia para compensación



// =====================================================
//                   VARIABLES DE ESTADO
// =====================================================

// --- Bombas ---
bool b0_state = 0, b1_state = 0, b2_state = 0, b3_state = 0, b4_state = 0;

// --- LEDs ---
bool LED_state = 0;  // Ambos estantes se encienden al mismo tiempo

// --- Ventiladores ---
bool v0_state = 0; // Refrigeración LED
bool v1_state = 0; // Columna izquierda
bool v2_state = 0; // Columna central
bool v3_state = 0; // Columna derecha

// --- Caudalímetros ---
const float VOLUMEN_POR_PULSO = 2.25; // ml/pulso
float b1_cant = 0, b2_cant = 0, b3_cant = 0, b4_cant = 0; // Cantidad objetivo de riego, de momento no se calculan, cuando se llaman a las funciones
                                                          //se le pasan valores fijos 100ml,100ml,10ml,10ml respectivamente
float vol1 = 10.7, vol2 = 10.7, vol3 = 10.7, vol4 = 10.7;  // Volumen medido
int pulsos1 = 0, pulsos2 = 0, pulsos3 = 0, pulsos4 = 0;   // Pulsos contados


// =====================================================
//                   PARÁMETROS PID
// =====================================================
double Kp = 14.1176;               // Ganancia proporcional
double Ki = Kp / 120;               // Ganancia integral
double Kd = Kp * 30 - 200;          // Ganancia derivativa

double input = 0, output = 0;
unsigned long lastTime;
double ITerm = 0, lastInput = 0;
double error_actual = 0, integral = 0;
bool fail_PID = 0;


// =====================================================
//              PROTOTIPOS DE FUNCIONES
// =====================================================
void vent_config();
void bombas_config();
void LED_config();
void caudalimetro_config();
void temp_config();
void bombita_on(uint8_t, uint8_t, int&, float&, bool&, float);
void LED(unsigned long, unsigned long);
void temp_control(float, unsigned long);
void Wtemp_read(unsigned long);
void bomba(unsigned long, unsigned long);
void vent(bool);
void pHcontrol(unsigned long);
void ECcontrol(unsigned long);
void display_update();
float pH_read();
float EC_read();
void display_update();

// ===============================================================================================================================================================
// ===============================================================================================================================================================


void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Armario Hidroponico");
  lcd.setCursor(0, 1);
  lcd.print("Iniciando...");
  //Poner todos los vent como salidas y setearlos en LOW
  vent_config();
  //Poner todas las bombas como salidas y setearlas en LOW
  bombas_config();
  //Poner los LED como salida y setearlos en LOW
  LED_config();
  //Poner los caudalimetros como entradas
  caudalimetro_config();
  //Poner los sensores de temp como entradas
  temp_config();

  //Para pH y electroconductividad
  pinMode(A1,INPUT);
  pinMode(A2,INPUT);
  lastTime = millis();
  lastInput = dht_int.readTemperature();
  // Serial.print("kp: ");
  // Serial.print(Kp);
  // Serial.print(" ki: ");
  // Serial.print(Ki);
  // Serial.print(" kd: ");
  // Serial.println(Kd);

}

void loop() {
  currentMillis = millis(); // Actualizar tiempo al inicio de cada iteración
  //Ejemplos de cada función:
  switch (screenIndex) {
    case 0: showScreen1(); break;
    case 1: showScreen2(); break;
    case 2: showScreen3(); break;
    case 3: showScreen4(); break;
  }

  if (currentMillis - previousMillis_display >= interval_display) {
    previousMillis_display = currentMillis;
    screenIndex++;
    if (screenIndex >= totalScreens) screenIndex = 0;
    lcd.clear();
  }
  LED(interval_LED_on, interval_LED_off); //Se le pasa el tiempo que tiene que estar encendido y apagado, puede modificarse.             OK
  temp_control(limit_temp, interval_temp); //Se le pasa cada cuanto tiempo medir, puede modificarse.                                     OK - Sensores y ventiladores. VER PID
  Wtemp_read(interval_Wtemp); //Se le pasa cada cuanto tiempo medir, puede modificarse.                                                  OK                                             
  bomba(interval_bomba_on,interval_bomba_off); //Se le pasa el tiempo que tiene que estar encendido y apagado, puede modificarse.        OK
  //pHcontrol(interval_pH); //Se le pasa cada cuanto tiempo medir, puede modificarse.
  //ECcontrol(interval_EC); //Se le pasa cada cuanto tiempo medir, puede modificarse.
  //b1_on(b1_cant); //Se le pasa cuanto volumen tienen que suministrar. Debería salir de un cálculo por las mediciones de pH.            OK - comba y cauda andan - Falta verificar ecuación
  //b2_on(b2_cant); //Se le pasa cuanto volumen tienen que suministrar. Debería salir de un cálculo por las mediciones de pH.            OK - bomba y cauda andan - Falta verificar ecuación
  //b3_on(b3_cant); //Se le pasa cuanto volumen tienen que suministrar. Debería salir de un cálculo por las mediciones de EC.            OK - bomba y cauda andan - Falta verificar ecuación
  //b4_on(b4_cant); //Se le pasa cuanto volumen tienen que suministrar. Debería salir de un cálculo por las mediciones de EC.            OK - bomba y cauda andan - Falta verificar ecuación
  
  
  unsigned long currentMillis = millis();
  //Función para imprimir parámetros de interes. No superponer con el funcionamiento de la comunicación
  if (currentMillis - previousMillis_print >= interval_print) {
    previousMillis_print = currentMillis;
    //  Serial.print("LED: ");
    //  Serial.print(LED_state);
    //  Serial.print(" Temp_int: ");
    //  Serial.print(temp_value_int);
    //  Serial.print(" Hum_int: ");
    //  Serial.print(hum_value_int);
    //  Serial.print(" Hum_ext: ");
    //  Serial.print(hum_value_ext);
    //  Serial.print(" Temp_ext: ");
    //  Serial.print(temp_value_ext);
    //  Serial.print(" Valor LDR: ");
    //  Serial.print(LDRvalue);
    //  Serial.print(" Vent: ");
    //  Serial.println(v1_state);
  }
  


// Código para imprimir en el puerto Serie:
  if (Serial.available()) {
    char command = Serial.read();
    if (command == 'P') {
      
      // Crear el documento JSON
      StaticJsonDocument<JSON_BUFFER_SIZE> doc;

      // Completar el JSON con sus claves y valores
      
      // Temperaturas y humedades
      doc["temperatura_interior"] = temp_value_int;
      doc["humedad_interior"] = hum_value_int;
      doc["temperatura_exterior"] = temp_value_ext;
      doc["humedad_exterior"] = hum_value_ext;
      doc["temperatura_solucion_estante_superior"] = Wtemp1_value;
      doc["temperatura_solucion_estante_inferior"] = Wtemp0_value;

      // pH y CE (si se usan)
      doc["ph"] = pH_value;
      doc["electroconductividad"] = EC_value;
      doc["intensidad_luz"] = LDRvalue;

      // Límites de control
      doc["temperatura_setpoint"] = limit_temp;
      doc["ph_setpoint_minimo"] = pH_low;
      doc["ph_setpoint_maximo"] = pH_high;
      doc["electroconductividad_setpoint_minimo"] = EC_low;
      doc["electroconductividad_setpoint_maximo"] = EC_high;

      // Intervalos de muestreo/tiempo (ms o s según corresponda)
      doc["bomba_tanque_principal_tiempo_encendido"] = interval_bomba_on;
      doc["bomba_tanque_principal_tiempo_apagado"] = interval_bomba_off;
      doc["led_tiempo_encendido"] = interval_LED_on;
      doc["led_tiempo_apagado"] = interval_LED_off;
      doc["intervalo_control_ph"] = interval_pH;
      doc["intervalo_control_ce"] = interval_EC;
      doc["intervalo_control_temperatura_aire"] = interval_temp;
      doc["intervalo_control_temperatura_solucion"] = interval_Wtemp;
      doc["intervalo_control_luz"] = interval_LDR;

      // Estados (tanques, ventilación, LEDs)
      doc["led_estante_superior_estado"] = LED_state;
      doc["led_estante_iniferior_estado"] = LED_state;
      doc["bomba_tanque_principal_estado"] = b0_state;
      doc["bomba_tanque_1_estado"] = b1_state;
      doc["bomba_tanque_2_estado"] = b2_state;
      doc["bomba_tanque_3_estado"] = b3_state;
      doc["bomba_tanque_4_estado"] = b4_state;
      doc["ventilador_principal_estado"] = v0_state;
      doc["ventilador_1_estado"] = v1_state;
      doc["ventilador_2_estado"] = v2_state;
      doc["ventilador_3_estado"] = v3_state;

      // Fallos
      doc["fallo_pid"] = fail_PID;

      // Crear un objeto String para almacenar el JSON
      String jsonString;
      serializeJson(doc, jsonString);

      // Imprimir el JSON en el puerto Serie
      Serial.println(jsonString);
    }
  }

}


// ===============================================================================================================================================================
// ===============================================================================================================================================================


//Reles
void vent_config(){ //Función para setear los pines como salida y ponerlos en low
  
  pinMode(v0,OUTPUT); //Vent LED
  pinMode(v1,OUTPUT); //Vent 1
  pinMode(v2,OUTPUT); //Vent 2
  pinMode(v3,OUTPUT); //Vent 3
  
  digitalWrite(v0,LOW);
  digitalWrite(v1,LOW); 
  digitalWrite(v2,LOW);
  digitalWrite(v3,LOW);

  v0_state = 0;
  v1_state = 0;
  v2_state = 0;
  v3_state = 0;
}

//Bombas CONFIG
void bombas_config(){ //Función para setear los pines como salida y ponerlos en low
  
  pinMode(b1,OUTPUT); //Bombita 1 - ph A
  pinMode(b2,OUTPUT); //Bombita 2 - pH B
  pinMode(b3,OUTPUT); //Bombita 3 - Nutrientes 1
  pinMode(b4,OUTPUT); //Bombita 4 - Nutrientes 2
  pinMode(b0,OUTPUT); //Bomba ppal
  
  digitalWrite(b1,LOW); //Bombita 1 - ph A
  digitalWrite(b2,LOW); //Bombita 2 - pH B
  digitalWrite(b3,LOW); //Bombita 3 - Nutrientes 1
  digitalWrite(b4,LOW); //Bombita 4 - Nutrientes 2
  digitalWrite(b0,HIGH); //Bomba ppal. HIGH es apagado para el solido

  b1_state = 0;
  b2_state = 0;
  b3_state = 0;
  b4_state = 0;
  b0_state = 0;
}

//Luces LED
void LED_config(){
  pinMode(LED1,OUTPUT); //LED estante 1
  pinMode(LED2,OUTPUT); //LED estante 2

  digitalWrite(LED1,HIGH); //LED estante 1 //HIGH es apagado para el solido
  digitalWrite(LED2,HIGH); //LED estante 2 //HIGH es apagado para el solido
 
  LED_state = 0;
}

//Caudalímetros
void caudalimetro_config(){ //Función para setear los pines como entrada
  
  pinMode(c1,INPUT); //Caudalímetro 1 - ph A
  pinMode(c2,INPUT); //Caudalímetro 2 - pH B
  pinMode(c3,INPUT); //Caudalímetro 3 - Nutrientes 1
  pinMode(c4,INPUT); //Caudalímetro 4 - Nutrientes 2
}

//Sensores de temperatura
void temp_config(){ //Función para setear los pines como entrada y arrancar los sensores
  
  //DHT22
  pinMode(DHTPIN_int, INPUT);
  pinMode(DHTPIN_ext, INPUT);
  dht_int.begin();
  dht_ext.begin();
  
  //DS18B20
  pinMode(Wtemp_pin, INPUT);
  DS18B20.begin();
}


//Encender cada una de las bombitas. Se usan los ml como parámetro. La bomba se enciende solo para esos ml.
void bombita_on(uint8_t pinBomba, uint8_t pinCaudal, int& pulsos, float& vol, bool& estado, float ml) {
  
  while (vol < ml) {
    attachInterrupt(digitalPinToInterrupt(pinCaudal), pulsos++, RISING); // revisar esto despues
    vol = ((pulsos * VOLUMEN_POR_PULSO) / 10.0) - 10.7;
    digitalWrite(pinBomba, HIGH);
    estado = HIGH;
    // Serial.print("Pin bomba: ");
    // Serial.print(pinBomba);
    // Serial.print(" | Vol actual: ");
    // Serial.print(vol);
    // Serial.print(" | Vol objetivo: ");
    // Serial.print(ml);
    // Serial.print(" | Pulsos: ");
    // Serial.println(pulsos);
  }

  digitalWrite(pinBomba, LOW);
  estado = LOW;
  detachInterrupt(digitalPinToInterrupt(pinCaudal));
  pulsos = 0;
  vol = 10.7;
  // Serial.print("Pin bomba: ");
  // Serial.print(pinBomba);
  // Serial.println(" | Bomba apagada, pulsos y volumen reseteados.");
}




//LED: Función para encender los leds de los estantes. Como parámetro se necesita el tiempo de encendido y el de apagado.
void LED(unsigned long LED_tOn, unsigned long LED_tOff) {
  // Obtener el tiempo actual
  unsigned long currentMillis = millis();
  // Serial.print("LED ");
  // Serial.println(LED_state);

  // Verificar si ha pasado el tiempo de encendido o apagado y cambiar el estado del LED
  if (LED_state == LOW && currentMillis - previousMillis_LED >= LED_tOff) {
    digitalWrite(LED1, LOW); //Encendido
    digitalWrite(LED2, LOW); //Encendido
    digitalWrite(v0, HIGH);
    LED_state = HIGH;
    previousMillis_LED = currentMillis; // Reiniciar el contador de tiempo
    // Leer el valor de la LDR para chequear que esten encendidos
    //Conectar LDR: https://robots-argentina.com.ar/img/LDR1.png
    LDRvalue = analogRead(LDRpin);
    nivelIluminacion = LDRvalue/950*100;
    if (LDRvalue < 500){
      //Serial.println("Error: Chequear leds");
    }
  
  } else if (LED_state == HIGH && currentMillis - previousMillis_LED >= LED_tOn) {
    digitalWrite(LED1, HIGH); //Apagado
    digitalWrite(LED2, HIGH); //Apagado
    digitalWrite(v0, LOW);
    LED_state = LOW;
    previousMillis_LED = currentMillis; // Reiniciar el contador de tiempo
    // Leer el valor de la LDR para chequear que esten apagados
    //Conectar LDR: https://robots-argentina.com.ar/img/LDR1.png
    LDRvalue = analogRead(LDRpin);
    nivelIluminacion = LDRvalue/950*100;
    if (LDRvalue > 550){
      //Serial.println("Error: Chequear leds");
    }
  }
}

//Medir temp y humedad

void temp_control(float setPoint, unsigned long temp_time) {
  unsigned long currentMillis_temp = millis();
  // Lectura de sensores
    temp_value_int = dht_int.readTemperature();  // Interior
    hum_value_int  = dht_int.readHumidity();
    temp_value_ext = dht_ext.readTemperature();  // Exterior
    hum_value_ext  = dht_ext.readHumidity();

  if (currentMillis_temp - previousMillis_temp >= temp_time) {
    previousMillis_temp = currentMillis_temp;

    

    // =====================================================
    //    LÓGICA ON/OFF (en lugar de PID)
    // =====================================================
    if (temp_value_ext < setPoint) {  
      // solo tiene sentido comparar si la consigna es > temperatura exterior
      if (temp_value_int > setPoint) {
        temp_samples++;   // demasiada temperatura → sumar conteo
      } else if (temp_value_int < setPoint - 1) {  
        // histéresis de -1 °C para evitar que prenda/apague constantemente
        temp_samples--;
      } else {
        temp_samples = 0; // dentro del rango → resetear
      }

      // Una vez acumuladas suficientes muestras, actuar
      if (abs(temp_samples) >= 3) {
        if (temp_samples > 0) {
          vent(1);  // Enciende ventiladores
        } else {
          vent(0);  // Apaga ventiladores
        }
        temp_samples = 0; // reset
      }
    } else {
      fail_PID = 1; // La temperatura objetivo es inferior a la exterior
    }

    // =====================================================
    //     BLOQUE PID (COMENTADO)
    // =====================================================
    /*
    unsigned long now = millis();
    double timeChange = (double)(now - lastTime)/1000; 
    double error_anterior = error_actual;

    input = temp_value_int; 
    error_actual = setPoint - input;

    integral += error_actual * timeChange;
    ITerm = Ki * integral;

    double dInput = (error_actual - error_anterior) / timeChange;
    output = Kp * error_actual + ITerm + Kd * dInput;

    if (millis() - vent_time > 5000){
      if (output < 0) {
        vent(1); 
        vent_time = millis();
      } else {
        vent(0);
      }
    }

    lastInput = input;
    lastTime = now;
    */
  }
}

//Medir temperatura del agua
void Wtemp_read(unsigned long Wtemp_time) {
  unsigned long currentMillis_Wtemp = millis();
  DS18B20.requestTemperatures();
  Wtemp0_value = DS18B20.getTempCByIndex(0); // Lee la temperatura en grados Celsius del estante inferior
  Wtemp1_value = DS18B20.getTempCByIndex(1); // Lee la temperatura en grados Celsius del estante superior

  if (currentMillis_Wtemp - previousMillis_Wtemp >= Wtemp_time) {
    previousMillis_Wtemp = currentMillis_Wtemp;
    
    
    // Serial.print("Temp0 (estante inferior): ");
    // Serial.println(Wtemp0_value);
    // Serial.print("Temp1 (estante superior): ");
    // Serial.println(Wtemp1_value);
  }
}



//Prender bomba para recircular el fluído. Se maneja por tiempos
void bomba(unsigned long bomba_tOn, unsigned long bomba_tOff) {
  // Obtener el tiempo actual
  unsigned long currentMillis = millis();
  // Serial.print("bomba ppal ");
  // Serial.println(b0_state);

  // Verificar si ha pasado el tiempo de encendido o apagado y cambiar el estado de la bomba.
  if (b0_state == HIGH && currentMillis - previousMillis_bomba >= bomba_tOn) {
    digitalWrite(b0, HIGH); // Apagar la bomba
    b0_state = LOW;
    previousMillis_bomba = currentMillis; // Reiniciar el contador de tiempo
  } else if (b0_state == LOW && currentMillis - previousMillis_bomba >= bomba_tOff) {
    digitalWrite(b0, LOW); // Encender la bomba
    b0_state = HIGH;
    previousMillis_bomba = currentMillis; // Reiniciar el contador de tiempo
  }
}

//Prender ventiladores para bajar la temperatura
//Se encienden y apagan todos juntos, podrían separarse si se necesita.
void vent(bool vent_st){
  // Serial.print("vent1,2,3");
  // Serial.println(v3_state);

  if (vent_st == 1) {
    digitalWrite(v1, HIGH); // Enciende ventiladores.
    digitalWrite(v2, HIGH);
    digitalWrite(v3, HIGH);
    v1_state = 1;
    v2_state = 1;
    v3_state = 1;
  } else if (vent_st == 0) {
    digitalWrite(v1, LOW); // Apaga ventiladores
    digitalWrite(v2, LOW);
    digitalWrite(v3, LOW);
    v1_state = 0;
    v2_state = 0;
    v3_state = 0;
  }
}

//pH lectura
float pH_read(){
  
  int raw = analogRead(pHPin);
  float voltage = raw * (5.0 / 1023.0);
  pH_value = 3.5 * voltage + offset;  // Fórmula de aproximación

return pH_value; // ver de poner decimales
}

//EC lectura
float EC_read(){
  
  int raw = analogRead(ECPin);                   // Lectura cruda
  float voltage = raw * VREF / 1024.0;            // Conversión a voltaje
  float ECraw = voltage * coef;                   // Aplicar factor de calibración
  float TempCoefficient = 1.0 + tempCoef * (temp_value_int - refTemp);

  EC_value = ECraw / TempCoefficient;              // Guardar en variable global
  
return EC_value; // ver de poner decimales
}

//pH control
void pHcontrol(unsigned long pH_time) {
  unsigned long currentMillis_pH = millis();
  if (currentMillis_pH - previousMillis_pH >= pH_time) {
    previousMillis_pH = currentMillis_pH;

    pH_value = pH_read();
  //  Serial.print("pH Value: ");
  //  Serial.print(pH_value);

    if (pH_value > pH_high) {
      pH_samples++;
    } else if (pH_value < pH_low) {
      pH_samples--;
    } else {
      pH_samples = 0; // Reset if pH is within acceptable range
    }
  //  Serial.print(" Samples: ");
  //  Serial.println(pH_samples);
    // Check if 3 consecutive samples indicate the need for pH regulation
    if (abs(pH_samples) >= 3) {
      if (pH_samples > 0) {
        // pH is consistently high, adjust accordingly
        bombita_on(b1, c1, pulsos1, vol1, b1_state, 100.0);
      } else {
        // pH is consistently low, adjust accordingly
        bombita_on(b2, c2, pulsos2, vol2, b2_state, 100.0);
      }
      // Reset pH sample count
      pH_samples = 0;
    }
  }
}

//EC control
void ECcontrol(unsigned long EC_time) {
  unsigned long currentMillis_EC = millis();

  if (currentMillis_EC - previousMillis_EC >= EC_time) {
    previousMillis_EC = currentMillis_EC;

    EC_value = EC_read();
    //Serial.print("EC Value: ");
    //Serial.print(EC_value);

    if (EC_value > EC_high) {
      EC_samples++;
    } else if (EC_value < EC_low) {
      EC_samples--;
    } else {
      EC_samples = 0; // Reset if pH is within acceptable range
    }
    //Serial.print(" Samples: ");
    //Serial.println(EC_samples);
    // Check if 3 consecutive samples indicate the need for pH regulation
    if (abs(EC_samples) >= 3) {
      if (EC_samples > 0) {
        // pH is consistently high, adjust accordingly
        bombita_on(b3, c3, pulsos3, vol3, b3_state, 10.0);
      } else {
        // pH is consistently low, adjust accordingly
        bombita_on(b4, c4, pulsos4, vol4, b4_state, 10.0);
      }
      // Reset pH sample count
      EC_samples = 0;
    }
  }
}

void showScreen1() {
  String tempStr;

  auto printRight = [&](const char* label, float val, int row) {
    tempStr = String(val, 1) + " C";
    if (tempStr.length() > 19) tempStr = tempStr.substring(0, 19);
    lcd.setCursor(0, row);
    lcd.print(label);
    lcd.setCursor(19 - tempStr.length() + 1, row);
    lcd.print(tempStr);
  };

  printRight("Temp Interior:", temp_value_int, 0);
  printRight("Temp Exterior:", temp_value_ext, 1);
  printRight("Solucion Sup: ", Wtemp1_value,   2);
  printRight("Solucion Inf: ", Wtemp0_value,   3);
}

void showScreen2() {
  String valStr;

  lcd.setCursor(0, 0);
  lcd.print("     -- pH --       ");

  valStr = "Valor: " + String(pH_value, 2);
  lcd.setCursor(0, 1);
  lcd.print(valStr);

  valStr = "Min:   " + String(pH_low, 2);
  lcd.setCursor(0, 2);
  lcd.print(valStr);

  valStr = "Max:   " + String(pH_high, 2);
  lcd.setCursor(0, 3);
  lcd.print(valStr);
}

void showScreen3() {
  String valStr;

  lcd.setCursor(0, 0);
  lcd.print("     -- EC --       ");

  valStr = "Valor: " + String(EC_value, 2) + " mS/cm";
  lcd.setCursor(0, 1);
  lcd.print(valStr);

  valStr = "Min:   " + String(EC_low, 2);
  lcd.setCursor(0, 2);
  lcd.print(valStr);

  valStr = "Max:   " + String(EC_high, 2);
  lcd.setCursor(0, 3);
  lcd.print(valStr);
}

void showScreen4() {
  String valStr;

  lcd.setCursor(0, 0);
  lcd.print("  -- Intervalos --  ");

  valStr = "Bomba: " + String(interval_bomba_on / 60000) + "/" + String(interval_bomba_off / 60000) + " min";
  lcd.setCursor(0, 1);
  lcd.print(valStr);

  valStr = "LED:   " + String(interval_LED_on / 60000) + "/" + String(interval_LED_off / 60000) + " min";
  lcd.setCursor(0, 2);
  lcd.print(valStr);

  valStr = "Temp:  " + String(interval_temp / 60000) + " min";
  lcd.setCursor(0, 3);
  lcd.print(valStr);
}