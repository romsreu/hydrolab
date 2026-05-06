/********************************************************************
 ***    Laboratorio Autocontenido Hidropónico de Industria 4.0    ***
 ********************************************************************

 ********************************************************************
 ***********************   ACTUADORES   *****************************
 *  Implementación  de  las  funciones  de  control  de  actuadores  *
 *  del  sistema.  Incluye  configuración  y  manejo  de  ventila-  *
 *  dores,  bombas  peristálticas,  bomba  principal,  LEDs  de    *
 *  iluminación  y  caudalímetros.                                  *
 ********************************************************************/

#include "actuadores.h"

extern const float VOLUMEN_POR_PULSO;

void vent_config() {
  pinMode(v0, OUTPUT);
  pinMode(v1, OUTPUT);
  pinMode(v2, OUTPUT);
  pinMode(v3, OUTPUT);

  digitalWrite(v0, LOW);
  digitalWrite(v1, LOW);
  digitalWrite(v2, LOW);
  digitalWrite(v3, LOW);

  v0_state = 0;
  v1_state = 0;
  v2_state = 0;
  v3_state = 0;
}

void bombas_config() {
  pinMode(b1, OUTPUT); // Bombita 1 - pH A
  pinMode(b2, OUTPUT); // Bombita 2 - pH B
  pinMode(b3, OUTPUT); // Bombita 3 - Nutrientes 1
  pinMode(b4, OUTPUT); // Bombita 4 - Nutrientes 2
  pinMode(b0, OUTPUT); // Bomba principal

  digitalWrite(b1, LOW);
  digitalWrite(b2, LOW);
  digitalWrite(b3, LOW);
  digitalWrite(b4, LOW);
  digitalWrite(b0, HIGH); // HIGH es apagado para el relé sólido

  b1_state = 0;
  b2_state = 0;
  b3_state = 0;
  b4_state = 0;
  b0_state = 0;
}

void LED_config() {
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);

  digitalWrite(LED1, HIGH); // HIGH es apagado para el relé sólido
  digitalWrite(LED2, HIGH);

  LED_state = 0;
}

void caudalimetro_config() {
  pinMode(c1, INPUT);
  pinMode(c2, INPUT);
  pinMode(c3, INPUT);
  pinMode(c4, INPUT);
}

void bombita_on(uint8_t pinBomba, uint8_t pinCaudal, int& pulsos, float& vol, bool& estado, float ml) {
  while (vol < ml) {
    attachInterrupt(digitalPinToInterrupt(pinCaudal), pulsos++, RISING);
    vol = ((pulsos * VOLUMEN_POR_PULSO) / 10.0) - 10.7;
    digitalWrite(pinBomba, HIGH);
    estado = HIGH;
  }

  digitalWrite(pinBomba, LOW);
  estado = LOW;
  detachInterrupt(digitalPinToInterrupt(pinCaudal));
  pulsos = 0;
  vol = 10.7;
}

void LED(unsigned long LED_tOn, unsigned long LED_tOff) {
  unsigned long currentMillis = millis();

  if (LED_state == LOW && currentMillis - previousMillis_LED >= LED_tOff) {
    digitalWrite(LED1, LOW);
    digitalWrite(LED2, LOW);
    digitalWrite(v0, HIGH);
    LED_state = HIGH;
    previousMillis_LED = currentMillis;
    LDRvalue = analogRead(LDRpin);
    nivelIluminacion = LDRvalue / 950 * 100;

  } else if (LED_state == HIGH && currentMillis - previousMillis_LED >= LED_tOn) {
    digitalWrite(LED1, HIGH);
    digitalWrite(LED2, HIGH);
    digitalWrite(v0, LOW);
    LED_state = LOW;
    previousMillis_LED = currentMillis;
    LDRvalue = analogRead(LDRpin);
    nivelIluminacion = LDRvalue / 950 * 100;
  }
}

void bomba(unsigned long bomba_tOn, unsigned long bomba_tOff) {
  unsigned long currentMillis = millis();

  if (b0_state == HIGH && currentMillis - previousMillis_bomba >= bomba_tOn) {
    digitalWrite(b0, HIGH); // Apagar la bomba
    b0_state = LOW;
    previousMillis_bomba = currentMillis;
  } else if (b0_state == LOW && currentMillis - previousMillis_bomba >= bomba_tOff) {
    digitalWrite(b0, LOW); // Encender la bomba
    b0_state = HIGH;
    previousMillis_bomba = currentMillis;
  }
}

void vent(bool vent_st) {
  if (vent_st == 1) {
    digitalWrite(v1, HIGH);
    digitalWrite(v2, HIGH);
    digitalWrite(v3, HIGH);
    v1_state = 1;
    v2_state = 1;
    v3_state = 1;
  } else if (vent_st == 0) {
    digitalWrite(v1, LOW);
    digitalWrite(v2, LOW);
    digitalWrite(v3, LOW);
    v1_state = 0;
    v2_state = 0;
    v3_state = 0;
  }
}