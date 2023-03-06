/*************************************************************************
  Código IMS BLE gauge board
  Tarjeta:
  Arduino Nano 33 BLE
  Crea un periférico BLE con un servicio personalizado que comprende una 
  característica para la lectura del ADS1232 en modo Notify. Se utiliza 
  la librería ADS123x de https://github.com/HamidSaffari/ADS123X.
 ************************************************************************/


#include <ArduinoBLE.h>
#include <ADS123X.h>


#define LED_PCB_IMS_STAT D11
#define BTN_PCB_IMS_START D10

#define BTN_PRESSED 0
#define BTN_NOT_PRESSED 1

#define BLE_DEVICE_NAME "BLE IMS gauge board"
#define BLE_SERVICE_UUID "68D2E014-B38D-11EC-B909-0242AC120002"
#define BLE_CHARACTERISTIC_UUID "68D2E015-B38D-11EC-B909-0242AC120002"

#define ADC_DOUT D2
#define ADC_SCLK D3
#define ADC_PDWN D4
#define ADC_GAIN0 D7
#define ADC_GAIN1 D6
#define ADC_SPEED D5
#define ADC_A0 D0
#define ADC_A1 D1

#define ADC_READ_INTERVAL_MS 200
#define ADS_AVERAGE_READ_COUNT 10


enum States {
  BLE_START_ADVERTISING,
  BLE_STOP_ADVERTISING,
  BLE_ADVERTISING,
  BLE_IDLE_ADVERTISING
};

States state = BLE_IDLE_ADVERTISING;

// BLE Custom Gauge Service
BLEService strainGaugeService(BLE_SERVICE_UUID);

// BLE Custom Characteristic
BLEFloatCharacteristic ads1232ReadingCharacteristic(BLE_CHARACTERISTIC_UUID, BLERead | BLENotify);

// ADS123X instance
ADS123X ads1232;

BLEDevice central;


/****************************************
   Main Functions
 ****************************************/

void setup() {
  Serial.begin(9600);
  delay(1000);

  Serial.println("\n--- Start---");

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(BTN_PCB_IMS_START, INPUT);

  // BLE setup
  setupBLEPeripheral();

  // ADS1232 initialization
  ads1232.begin(ADC_DOUT, ADC_SCLK, ADC_PDWN, ADC_GAIN0, ADC_GAIN1, ADC_SPEED, ADC_A0, ADC_A1);
}

void loop() {
  static unsigned long previousMillis = 0;
  unsigned long currentMillis = millis();

  switch (state) {
    case BLE_START_ADVERTISING:
      digitalWrite(LED_PCB_IMS_STAT, HIGH);

      // start advertising
      BLE.advertise();
      Serial.println("Bluetooth device active, waiting for connections...");

      state = BLE_ADVERTISING;
      break;

    case BLE_ADVERTISING:
      central = BLE.central();

      // if a central is connected to the peripheral
      if (central) {
        Serial.println("* Connected to central device!");
        Serial.print("* Device MAC address: ");
        Serial.println(central.address());
        Serial.println(" ");

        digitalWrite(LED_BUILTIN, HIGH);

        while (central.connected()) {
          previousMillis = 0;
          currentMillis = millis();
          static unsigned long adcReadingMillis = 0;

          if (digitalRead(BTN_PCB_IMS_START) == BTN_PRESSED && currentMillis - previousMillis >= 2000) {
            // stop advertising
            BLE.stopAdvertise();
            Serial.println("Bluetooth device stop advertising...");

            // disconnect from central device
            central.disconnect();

            previousMillis = currentMillis;
            state = BLE_IDLE_ADVERTISING;
          }

          if (currentMillis - adcReadingMillis >= ADC_READ_INTERVAL_MS) {
            adcReadingMillis = currentMillis;
            Serial.println("ADS1232 gauge value notify");
            //updateADS1232Reading();
          }
        }

        // when the central disconnects, turn off the LED:
        digitalWrite(LED_BUILTIN, LOW);
        digitalWrite(LED_PCB_IMS_STAT, LOW);

        Serial.println("* Disconnected to central device!");
        Serial.print("* Device MAC address: ");
        Serial.println(central.address());
        Serial.println(" ");

        Serial.println("Bluetooth device active, waiting for connections...");
      }

      if (digitalRead(BTN_PCB_IMS_START) == BTN_PRESSED && currentMillis - previousMillis >= 2000) {
        // stop advertising
        BLE.stopAdvertise();
        Serial.println("Bluetooth device stop advertising...");

        previousMillis = currentMillis;
        state = BLE_IDLE_ADVERTISING;
      }
      break;

    case BLE_IDLE_ADVERTISING:
      digitalWrite(LED_PCB_IMS_STAT, LOW);

      if (digitalRead(BTN_PCB_IMS_START) == BTN_PRESSED && currentMillis - previousMillis >= 2000) {
        previousMillis = currentMillis;
        state = BLE_START_ADVERTISING;
      }
      break;
  }

}

/****************************************
   Auxiliar Functions
 ****************************************/

void setupBLEPeripheral() {
  // begin BLE initialization
  while (!BLE.begin()) {
    Serial.println("starting BLE failed!...");
    delay(1000);
  }

  Serial.println("IMS BLE gauge board (Peripheral Device)");
  Serial.println(" ");

  Serial.print("My BLE MAC:\t\t ");
  Serial.println(BLE.address());

  Serial.print("Service UUID:\t\t ");
  Serial.println(strainGaugeService.uuid());

  Serial.print("Characteristic UUID:\t ");
  Serial.println(ads1232ReadingCharacteristic.uuid());
  Serial.println();

  // set a local name for the BLE device
  BLE.setLocalName(BLE_DEVICE_NAME);

  // set the BLE service UUID
  BLE.setAdvertisedService(strainGaugeService);

  // add BLE characteristic
  strainGaugeService.addCharacteristic(ads1232ReadingCharacteristic);

  // add the BLE service
  BLE.addService(strainGaugeService);

  // set an initial value for this characteristic
  ads1232ReadingCharacteristic.setValue(0.f);
}

void updateADS1232Reading() {
  static float oldValue = 0;
  float averageValue;

  // promedio de lecturas del ADS1232
  ads1232.get_units(AINP1, averageValue, ADS_AVERAGE_READ_COUNT, true);

  // si lectura del ADS1232 cambió
  if (averageValue != oldValue) {
    Serial.print("ADS1232 reading: ");
    Serial.println(averageValue);

    // update the ads1232Reading characteristic
    ads1232ReadingCharacteristic.writeValue(averageValue);

    // save the value for next comparison
    oldValue = averageValue;
  }
}