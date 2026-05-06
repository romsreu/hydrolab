/********************************************************************
 ***    Laboratorio Autocontenido Hidropónico de Industria 4.0    ***
 ********************************************************************

 ************************   IMPORTANTE   ****************************
 *  El  puerto  serie  está  reservado  exclusivamente  para   la   *
 *  transmisión  de  datos  en  formato  JSON.  Modificar  la  ló-  *
 *  gica  del  sistema  únicamente  si  se  tiene  pleno  conoci-   *
 *  miento  de  lo  que  se  está  realizando,  ya  que  el  có-    *
 *  digo  controla  hardware  real  y  un  error  puede  ocasionar  *
 *  daños  físicos  irreversibles.  Para  pruebas,  utilizar  los   *
 *  sketches  unitarios  por  módulo  disponibles  en  el  reposi-  *
 *  torio.  Ante  cualquier  duda,  consultar  la  documentación    *
 *  antes  de  realizar  algún  cambio.                             *
 ****** https://github.com/romsreu/hydrolab/tree/main/docs **********
 ********************************************************************

 ********************************************************************
 ***********************   MAPA DE PINES   **************************
 ** https://github.com/romsreu/hydrolab/blob/main/docs/LAHI4.0.pdf **
 ***************** https://hydrolabutn.netlify.app/ *****************
 ********************************************************************

 ********************************************************************
 *********************   PINOUT ATMEGA2560   ************************
 ****** https://lastminuteengineers.com/arduino-mega-pinout/ ********
 *******************************************************************/

/********************************************************************
 ************************   PARÁMETROS   ****************************
 *******************************************************************/

/******************   HUMEDAD Y TEMPERATURA   **********************/

float LHum        = 80.0;   // [%] Límite de humedad relativa
float limit_temp  = 22;     // [°C] Temperatura límite

/***************************   pH   ********************************/

float pH_low      = 6;      // [pH] Valor mínimo aceptable
float pH_high     = 8;      // [pH] Valor máximo aceptable
float pH_value    = 0;      // [pH] Valor medido

/***************   CONDUCTIVIDAD ELÉCTRICA (EC)   ******************/

float EC_low      = 0;      // [mS/cm] Valor mínimo aceptable
float EC_high     = 0;      // [mS/cm] Valor máximo aceptable
float EC_value    = 0;      // [mS/cm] Valor medido

/***********************   VENTILACIÓN   ****************************/

float vent_time   = 0;      // [ms] Duración de ventilación


/********************************************************************
 **************   VARIABLES DE TIEMPO Y MUESTREO   ******************
 *******************************************************************/

const float MIN_TO_MILLISECONDS = 60000;

/**********************   TIEMPO GENERAL   **************************/

unsigned long currentMillis = 0; // Variable de tiempo general

/*********************   IMPRESIÓN SERIAL   *************************/

unsigned long previousMillis_print = 0;
unsigned long interval_print       = 1 * MIN_TO_MILLISECONDS; 

/***************************   pH   ********************************/

unsigned long previousMillis_pH    = 0;
unsigned long interval_pH          = 10 * MIN_TO_MILLISECONDS;   
int pH_samples                     = 5;                         

/***************************   EC   ********************************/

unsigned long previousMillis_EC    = 0;
unsigned long interval_EC          = 0 * MIN_TO_MILLISECONDS;    
int EC_samples                     = 0;                        

/*******************   TEMPERATURA AMBIENTE   ***********************/

unsigned long previousMillis_temp  = 0;
unsigned long interval_temp        = 1 * MIN_TO_MILLISECONDS;   
int temp_samples                   = 0;                          

/*********************   TEMPERATURA AGUA   *************************/

unsigned long previousMillis_Wtemp = 0;
unsigned long interval_Wtemp       = 1 * MIN_TO_MILLISECONDS;  

/********************   LUZ AMBIENTE (LDR)   ************************/

unsigned long previousMillis_LDR   = 0;
const long interval_LDR            = 15 * MIN_TO_MILLISECONDS; 

/**************************   LEDs   ********************************/

unsigned long previousMillis_LED   = 0;
unsigned long interval_LED_on      = 2 * MIN_TO_MILLISECONDS;   
unsigned long interval_LED_off     = 1 * MIN_TO_MILLISECONDS;   

/**********************   BOMBA PRINCIPAL   *************************/

unsigned long previousMillis_bomba = 0;
unsigned long interval_bomba_on    = 1 * MIN_TO_MILLISECONDS;   
unsigned long interval_bomba_off   = 2 * MIN_TO_MILLISECONDS;   

/************************   DISPLAY LCD   ***************************/

unsigned long previousMillis_display = 0;
const unsigned long interval_display = 8000;
int screenIndex = 0;
const int totalScreens = 4;

/********************************************************************
 **********************   ¡NO MODIFICAR!   **************************
 *  A  partir  de  esta  sección  se  definen  las  librerías,  de- *
 *  claraciones,  variables  de  estado  y  parámetros  internos    *
 *  del  sistema.  No  editar  salvo  que  se  desee  modificar  la *
 *  lógica  de  funcionamiento  interna.                            *
 ********************************************************************/

/********************************************************************
 ************************   LIBRERÍAS   *****************************
 *******************************************************************/

#include <DHT.h>                // Sensor de temperatura y humedad DHT22
#include <OneWire.h>            // Sensor de temperatura DS18B20
#include <DallasTemperature.h>  // Librería para DS18B20
#include <ArduinoJson.h>        // Manejo de datos en formato JSON
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x3F, 20, 4);

#include "display.h"
#include "sensores.h"
#include "actuadores.h"
#include "json_serial.h"

/********************************************************************
 *********************  DECLARACIÓN PINES   *************************
 *******************************************************************/

/********************   SALIDAS DIGITALES   ************************/

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

/********************   ENTRADAS DIGITALES   ***********************/

#define c1 45 // Caudalímetro 1
#define c2 47 // Caudalímetro 2
#define c3 49 // Caudalímetro 3
#define c4 51 // Caudalímetro 4  

/************************   SENSORES   ******************************/

#define DHTPIN_ext 2   // DHT22 exterior
#define DHTPIN_int 3   // DHT22 interior
#define Wtemp_pin  4   // DS18B20 (dos conectados al mismo pin)
#define LDRpin     A0  // Sensor LDR (iluminación)
#define pHPin      A1  // Sensor pH (simulado con potenciómetro)
#define ECPin      A2  // Sensor EC (simulado con potenciómetro)


/********************************************************************
 ******************  CONFIGURACIÓN SENSORES   ***********************
 *******************************************************************/

/*************************   DHT22   *******************************/

#define DHTTYPE DHT22
DHT dht_int(DHTPIN_int, DHTTYPE);   // Sensor interior
DHT dht_ext(DHTPIN_ext, DHTTYPE);   // Sensor exterior
float temp_value_int = 0, temp_value_ext = 0;
float hum_value_int  = 0, hum_value_ext = 0;
float Wtemp0_value   = 0, Wtemp1_value = 0;

/**************************   LDR   ********************************/

int LDRvalue = 0;
int nivelIluminacion = 0;

/************************   DS18B20   ******************************/

OneWire oneWire(Wtemp_pin);
DallasTemperature DS18B20(&oneWire);

/**************************   JSON   *******************************/

const size_t JSON_BUFFER_SIZE = JSON_OBJECT_SIZE(50);

/***************************   pH   ********************************/

float offset = 0.00;   // Ajustar en calibración

/***************************   EC   ********************************/

float VREF = 5.0;              // Voltaje de referencia (5V o 3.3V)
float coef = 1.0;              // Factor de calibración (ajustar con solución patrón)
float tempCoef = 0.0185;       // Coef. de compensación por °C
float refTemp = 25.0;          // Temperatura de referencia para compensación


/********************************************************************
 ********************  VARIABLES DE ESTADO   ************************
 *******************************************************************/

/*************************   BOMBAS   ******************************/

bool b0_state = 0, b1_state = 0, b2_state = 0, b3_state = 0, b4_state = 0;

/**************************   LEDs   *******************************/

bool LED_state = 0;  // Ambos estantes se encienden al mismo tiempo

/**********************   VENTILADORES   ***************************/
bool v0_state = 0, v1_state = 0, v2_state = 0, v3_state = 0;
//v0 Refrigeracion LED, v1 Columna Izquierda, v2 Columna Central, v3 Columna Derecha

/**********************   CAUDALÍMETROS   ***************************/
const float VOLUMEN_POR_PULSO = 2.25; // ml/pulso
float b1_cant = 0, b2_cant = 0, b3_cant = 0, b4_cant = 0; 
/* Cantidad objetivo de riego, de momento no se calculan, cuando se llaman a las
 funcionesse le pasan valores fijos 100ml,100ml,10ml,10ml respectivamente*/
float vol1 = 10.7, vol2 = 10.7, vol3 = 10.7, vol4 = 10.7;  // Volumen medido
int pulsos1 = 0, pulsos2 = 0, pulsos3 = 0, pulsos4 = 0;   // Pulsos contados


/********************************************************************
 ***********************  PARÁMETROS PID   **************************
 *******************************************************************/

double Kp = 14.1176;                // Ganancia proporcional
double Ki = Kp / 120;               // Ganancia integral
double Kd = Kp * 30 - 200;          // Ganancia derivativa

double input = 0, output = 0;
unsigned long lastTime;
double ITerm = 0, lastInput = 0;
double error_actual = 0, integral = 0;
bool fail_PID = 0;

/********************************************************************
 ****************************  SETUP   ******************************
 *******************************************************************/

void setup() {
  Serial.begin(9600);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Armario Hidroponico");
  lcd.setCursor(0, 1);
  lcd.print("Iniciando...");

  vent_config();
  bombas_config();
  LED_config();
  caudalimetro_config();
  temp_config();

  pinMode(A1,INPUT);
  pinMode(A2,INPUT);
  lastTime = millis();
  lastInput = dht_int.readTemperature();
}

void loop() {
  currentMillis = millis();
  
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

  LED(interval_LED_on, interval_LED_off);
  temp_control(limit_temp, interval_temp);
  Wtemp_read(interval_Wtemp);
  bomba(interval_bomba_on, interval_bomba_off);
  //pHcontrol(interval_pH);
  //ECcontrol(interval_EC);
  //b1_on(b1_cant);
  //b2_on(b2_cant);
  //b3_on(b3_cant);
  //b4_on(b4_cant);

  if (currentMillis - previousMillis_print >= interval_print) {
    previousMillis_print = currentMillis;
  }

  if (Serial.available()) {
    char command = Serial.read();
    if (command == 'P') enviar_json();
  }
}
