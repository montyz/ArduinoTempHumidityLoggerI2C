#include <Bridge.h>
#include <Temboo.h>
#include <Process.h>
#include "TembooAccount.h" // contains Temboo account information, as described below

// BME280 MOD-1022 weather multi-sensor Arduino demo
// Written originally by Embedded Adventures

#include <BME280_MOD-1022.h>

#include <Wire.h>

#define BUF_LEN 45
Process date;                 // process used to get the date

void printCompensatedMeasurements(void) {
  char buffer[BUF_LEN];
  int i;
  for (i=0; i<BUF_LEN; i++) {
    buffer[i] = '\0';
  }
  
  if (!date.running()) {
    date.begin("date");
    date.addParameter("+%Y-%m-%d %H:%M:%S");
    date.run();
  }
  
  String timeString = "";
  //if there's a result from the date process, parse it:
  while (date.available() > 0) {
    timeString = date.readString();
  }
  
  double tempMostAccurate, humidityMostAccurate, pressureMostAccurate;

  tempMostAccurate     = BME280.getTemperatureMostAccurate() * 9.0 / 5.0 + 32.0;
  humidityMostAccurate = BME280.getHumidityMostAccurate();
  pressureMostAccurate = BME280.getPressureMostAccurate();

  i = 0;
  timeString.toCharArray(buffer, BUF_LEN, 0);
  i += 19;
  buffer[i++]=',';
  buffer[i++] = ' ';
  dtostrf(tempMostAccurate, 7, 2, buffer+i);
  i+=7;
  buffer[i++] = ',';
  buffer[i++] = ' ';
  dtostrf(humidityMostAccurate, 7, 2, buffer+i);
  
      TembooChoreo AppendRowChoreo;

    // Invoke the Temboo client
    AppendRowChoreo.begin();

    // Set Temboo account credentials
    AppendRowChoreo.setAccountName(TEMBOO_ACCOUNT);
    AppendRowChoreo.setAppKeyName(TEMBOO_APP_KEY_NAME);
    AppendRowChoreo.setAppKey(TEMBOO_APP_KEY);

    // Set profile to use for execution
    AppendRowChoreo.setProfile("TempHumidity");

    // Set Choreo inputs
    AppendRowChoreo.addInput("SpreadsheetTitle", "Temp-Humidity");
    AppendRowChoreo.addInput("RowData", buffer);

    // Identify the Choreo to run
    AppendRowChoreo.setChoreo("/Library/Google/Spreadsheets/AppendRow");
    
    // Run the Choreo; when results are available, print them to serial
    AppendRowChoreo.run();
    
    while(AppendRowChoreo.available()) {
      char c = AppendRowChoreo.read();
      //Serial.print(c);
    }
    AppendRowChoreo.close();

}


// setup wire and serial

void setup()
{
  Wire.begin();
  /*Serial.begin(115200);
  while(!Serial);
  */
  
  Bridge.begin();

  // wait 2 min
  const long interval = 120000;
  unsigned long currentMillis = millis();
  unsigned long start = currentMillis;
  while (currentMillis - start <= interval) {
    currentMillis = millis();
  }

  // need to read the NVM compensation parameters
  BME280.readCompensationParams();
  
  // We'll switch into normal mode for regular automatic samples
  BME280.writeStandbyTime(tsb_0p5ms);        // tsb = 0.5ms
  BME280.writeFilterCoefficient(fc_16);      // IIR Filter coefficient 16
  BME280.writeOversamplingPressure(os16x);    // pressure x16
  BME280.writeOversamplingTemperature(os2x);  // temperature x2
  BME280.writeOversamplingHumidity(os1x);     // humidity x1
  
  BME280.writeMode(smNormal);
}

// main loop

void loop()
{
  while (BME280.isMeasuring());
  
  // read out the data - must do this before calling the getxxxxx routines
  BME280.readMeasurements();
  printCompensatedMeasurements();
  
  // wait 2 min
  const long interval = 1200000;
  unsigned long currentMillis = millis();
  unsigned long start = currentMillis;
  while (currentMillis - start <= interval) {
    currentMillis = millis();
  }
}

