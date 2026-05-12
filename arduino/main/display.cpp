#include "display.h"
#include "config.h"
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