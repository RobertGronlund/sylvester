#include <Arduino.h>
#include <ESP32Servo.h>
#include <bsec.h>

bool serial_enable = false;

// Helper functions declarations
void setServo(int pos);
void servoTest(void);
void checkIaqSensorStatus(void);
void errLeds(void);

// Servo setup 0
Servo servo;  // create servo object to control a servo
int servoPin = A1;
int mosfetPin = A0;
int pos_good = 180;
int pos_bad = 0;
int pos_neutral = 90;

// Sensor setup 0
Bsec iaqSensor;
int bme_vcc = 32;
int bme_gnd = 14;

// Variables
String output = "";
int good_air = 0;
int bad_air = 0;
int iaq_threshold = 100;
int trigger_change = 20;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  if (serial_enable) Serial.begin(9600);

  // Servo Setup 1 
  pinMode(mosfetPin, OUTPUT);
  ESP32PWM::allocateTimer(0);
  servo.setPeriodHertz(50);    // standard 50 hz servo
  // Servo tested min/max of 500us and 3000us for 0 to 180 sweep
  servo.attach(servoPin, 500, 3000);

  // Sensor setup 1
  pinMode(bme_gnd, OUTPUT);
  digitalWrite(bme_gnd, LOW);
  pinMode(bme_vcc, OUTPUT);
  digitalWrite(bme_vcc, HIGH);

  iaqSensor.begin(BME68X_I2C_ADDR_HIGH, Wire);
  checkIaqSensorStatus();

  bsec_virtual_sensor_t sensorList[13] = {
    BSEC_OUTPUT_IAQ,
    BSEC_OUTPUT_STATIC_IAQ,
    BSEC_OUTPUT_CO2_EQUIVALENT,
    BSEC_OUTPUT_BREATH_VOC_EQUIVALENT,
    BSEC_OUTPUT_RAW_TEMPERATURE,
    BSEC_OUTPUT_RAW_PRESSURE,
    BSEC_OUTPUT_RAW_HUMIDITY,
    BSEC_OUTPUT_RAW_GAS,
    BSEC_OUTPUT_STABILIZATION_STATUS,
    BSEC_OUTPUT_RUN_IN_STATUS,
    BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_TEMPERATURE,
    BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_HUMIDITY,
    BSEC_OUTPUT_GAS_PERCENTAGE
  };

  iaqSensor.updateSubscription(sensorList, 13, BSEC_SAMPLE_RATE_LP);
  checkIaqSensorStatus();
  // Print the header
  output = "Timestamp [ms],IAQ,IAQ accuracy,Temperature [C],Pressure [hPa],Humidity [%],Gas [Ohm],";
  if (serial_enable) Serial.println(output);
}

void loop() {
  // If new data is available
  if (iaqSensor.run()) { 
    output = String(millis());
    output += "," + String(iaqSensor.iaq);
    output += "," + String(iaqSensor.iaqAccuracy);
    output += "," + String(iaqSensor.temperature);
    output += "," + String(iaqSensor.pressure);
    output += "," + String(iaqSensor.humidity);
    output += "," + String(iaqSensor.gasResistance);
    output += ",";

    // Blink LED
    digitalWrite(LED_BUILTIN, HIGH);
    delay(50);
    digitalWrite(LED_BUILTIN, LOW);

    // Check if iaq readings are ready
    if (!iaqSensor.iaqAccuracy) {
      setServo(pos_neutral);
      // output += " \t| Sensor warming up";
      if (serial_enable) Serial.println(output);
      return;
    } 
    
    // Check iaq compared to threshold
    if (iaqSensor.iaq < iaq_threshold) {
      good_air++;
      bad_air = 0;
    } else {
      good_air = 0;
      bad_air++;
    }

    // Trigger change
    if (good_air > trigger_change) {
      setServo(pos_good);
      // output += " \t| Good air, keep it up!";
    } else if (bad_air > trigger_change) {
      setServo(pos_bad);
      // output += " \t| Bad air, open a window!";
    }

    if (serial_enable) Serial.println(output);

  } else {
    // Go to low power
    checkIaqSensorStatus();
    delay(1000);
    return;
  }
}


// Helper function definitions
void setServo(int pos) {
  digitalWrite(mosfetPin, HIGH);
  delay(1000);
  servo.write(pos);
  delay(1000);
  digitalWrite(mosfetPin, LOW);
}

void servoTest(){
  setServo(pos_bad);
  delay(3000);
  setServo(pos_good);
  delay(3000);
}

void checkIaqSensorStatus(void)
{
  if (iaqSensor.bsecStatus != BSEC_OK) {
    if (iaqSensor.bsecStatus < BSEC_OK) {
      output = "BSEC error code : " + String(iaqSensor.bsecStatus);
      if (serial_enable) Serial.println(output);
      for (;;)
        errLeds(); /* Halt in case of failure */
    } else {
      output = "BSEC warning code : " + String(iaqSensor.bsecStatus);
      if (serial_enable) Serial.println(output);
    }
  }

  if (iaqSensor.bme68xStatus != BME68X_OK) {
    if (iaqSensor.bme68xStatus < BME68X_OK) {
      output = "BME68X error code : " + String(iaqSensor.bme68xStatus);
      if (serial_enable) Serial.println(output);
      for (;;)
        errLeds(); /* Halt in case of failure */
    } else {
      output = "BME68X warning code : " + String(iaqSensor.bme68xStatus);
      if (serial_enable) Serial.println(output);
    }
  }
}

void errLeds(void)
{
  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);
  digitalWrite(LED_BUILTIN, LOW);
  delay(100);
}