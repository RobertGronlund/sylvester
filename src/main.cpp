#include <Arduino.h>
#include <ESP32Servo.h>
#include <bsec.h>

bool serial_enable {true};

// Helper functions declarations
void setServo(int pos);
void servoTest(void);
void checkIaqSensorStatus(void);
void errLeds(void);

// Servo setup 0
Servo servo; // create servo object to control a servo
int servoPin {A1};
int pos_good {180};
int pos_bad {0};
int pos_neutral {90};

// Sensor setup 0
Bsec iaqSensor;
int bme_vcc {32};
int bme_gnd {14};

// Variables
String output = "";
int good_air {0};
int bad_air {0};
int iaq_threshold {100};
int iaq_dif {0};
int switch_threshold {200};

void setup()
{
    pinMode(LED_BUILTIN, OUTPUT);
    if (serial_enable)
        Serial.begin(9600);

    // Servo Setup 1
    ESP32PWM::allocateTimer(0);
    servo.setPeriodHertz(50); // standard 50 hz servo
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
        BSEC_OUTPUT_GAS_PERCENTAGE};

    iaqSensor.updateSubscription(sensorList, 13, BSEC_SAMPLE_RATE_LP);
    checkIaqSensorStatus();

    // Print the header
    output = "Timestamp [ms],IAQ,IAQ accuracy,Temperature [C],Pressure [hPa],Humidity [%],Gas [Ohm],";
    if (serial_enable)
        Serial.println(output);
}

void loop()
{
    // If new data is available
    if (iaqSensor.run())
    {
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
        if (!iaqSensor.iaqAccuracy)
        {
            setServo(pos_neutral); 
            if (serial_enable)
                Serial.println(output);
            return;
        }

        // The algorithm takes the difference between iaq threshold and current iaq value,
        // the difference is added to good_air or bad_air. The bigger the difference from
        // the threshold the quicker the switch happen.
        iaq_dif = iaq_threshold - iaqSensor.iaq;
        if (iaq_dif > 0)
        {
            good_air += iaq_dif;
            bad_air = 0;
        }
        else
        {
            good_air = 0;
            bad_air -= iaq_dif;
        }

        // Trigger change
        if (good_air > switch_threshold)
        {
            setServo(pos_good);
        }
        else if (bad_air > switch_threshold)
        {
            setServo(pos_bad);
        }

        if (serial_enable)
            Serial.println(output);
    }    
}

// Helper function definitions
void setServo(int pos)
{
    servo.write(pos);
}

void checkIaqSensorStatus(void)
{
    if (iaqSensor.bsecStatus != BSEC_OK)
    {
        if (iaqSensor.bsecStatus < BSEC_OK)
        {
            output = "BSEC error code : " + String(iaqSensor.bsecStatus);
            if (serial_enable)
                Serial.println(output);
            for (;;)
                errLeds(); /* Halt in case of failure */
        }
        else
        {
            output = "BSEC warning code : " + String(iaqSensor.bsecStatus);
            if (serial_enable)
                Serial.println(output);
        }
    }

    if (iaqSensor.bme68xStatus != BME68X_OK)
    {
        if (iaqSensor.bme68xStatus < BME68X_OK)
        {
            output = "BME68X error code : " + String(iaqSensor.bme68xStatus);
            if (serial_enable)
                Serial.println(output);
            for (;;)
                errLeds(); /* Halt in case of failure */
        }
        else
        {
            output = "BME68X warning code : " + String(iaqSensor.bme68xStatus);
            if (serial_enable)
                Serial.println(output);
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

// Test functions
void servoTest()
{
    setServo(pos_bad);
    delay(3000);
    setServo(pos_good);
    delay(3000);
}
