/*-----------------------------------------Every command of this code will hendled by ESP32------------------------------------
                       But in case You can use following command :-
                      # To calibrate the load cell press-----> y or Y.
                      # To skip calibration press any alphanumeric key
                      # For unit conversion press------------> U or u.
*/

/* This is the code for load cell which can be used for any kind of load cell because it has a calibration function it also has a 
  feature of taring and unit conversion.
---------------------------------------------------------------------------------------------------------------------
* components used:-
  1. Load cell
  2.HX711 module
  3. Arduino board

-------------------------------------Connection with arduino and esp32--------------------------------------------------
# connect Rx pin of arduino with Tx pin of esp32 dev module (pin17)
# connect Tx pin of arduino with Rx pin of es32 dev module (Pin16)
# connect ground pin of arduino with ground pin of esp32
 */

/* Required libries for this project Hx711 can be download from arduio library manager */
#include "HX711.h"
#include <EEPROM.h>

// HX711 circuit wiring
const int LOADCELL_DOUT_PIN = 2; // Connect HX711 Dout pin to Arduino digital pin 2
const int LOADCELL_SCK_PIN = 3;  // Connect HX711 Sck pin to Arduino digital pin 3

HX711 scale; // Hx711 object named as scale
int reading;  // This variable will store the actual weight of the load.
int lastReading;
int baudrate=57600; // baudrate for the system 

// Deffault CALIBRATION FACTOR in case if person dosen't want to calibrate in that case it avoid any random value of palced weight
float DEFAULT_CALIBRATION_FACTOR = 326.660099;
float calibrationFactor = 0.0; // To store new actual calibrated value
const int EEPROM_ADDRESS = 0;  // EEProm address initilly it is at zero

/*-----------------------------------This is special type of variable that is used for unit conversinon -----------------------*/
enum WeightUnit { KILOGRAMS, GRAMS };
WeightUnit currentUnit = GRAMS; // initilly it is set to KILOGRAM


/*----------------------------------- This function will convert the weight to kg and kg to gram-----------------------*/

void inKg(float weight) {
  Serial.print("Weight: ");
  if (currentUnit == GRAMS) {
     Serial.print(weight);
    Serial.println(" g");
  } else {
   float kgWeight = static_cast<float>(weight) / 1000.0; // Type casting integer to float
    Serial.print(kgWeight, 3); // Display with 3 decimal places
    Serial.println(" kg");
  }
}

/*-------------------- This function will calibrate the loadcell and store the store that value to the EEPROM of arduino*/

void calibrate() {


  if (scale.is_ready()) {
        scale.tare();
        Serial.print("D");               // It will help esp32 to take decision when esp32 received D then it will exacute condition to enter weight
    bool weightEntered = false;         // For checking weather the actual weight of load entered or not
    bool enterAgain = true;             // Give chance to enter weight again in case of some non positive integer enterted
    float originalWeight = 0.0;         // to store the original weight entered by user


/*----------------------This will used to get actual weight of placed object on load cell within 10 sec--------------*/
while(enterAgain){
      while (Serial.available() > 0) {
        originalWeight = Serial.parseFloat();
        
/*------------------------Below condition check that whether the enterd weight greater the zero and and a number-------------*/      

        if (!isnan(originalWeight) && originalWeight > 0) { 
          weightEntered = true;
          enterAgain= false;
          break; // Exit the inner loop
        }
      }
      if (weightEntered) {
        break; // Exit the outer loop if weight have been entered
      }
}
    
/*----------------- used to calculate actual calibration factor----------------------------*/

    if (weightEntered) {
      long reading = scale.get_units(10);
      calibrationFactor = reading / originalWeight;//By dividing the reading get from hx711 with originalWeight will give our calibratio factor
      DEFAULT_CALIBRATION_FACTOR=calibrationFactor;
      delay(3000);
      scale.tare();

/*---------------------------------This part will store the calibration factor to the EEPROM of arduino-----------------------*/

      for (int i = 0; i < sizeof(calibrationFactor); i++) {
        EEPROM.write(EEPROM_ADDRESS + i, *((char*)&calibrationFactor + i)); // Write the data to EEPROM
      }
      EEPROM.end();  // compleating process

    } else {
      Serial.println("A");// It also help esp32 to take decision whenever esp32 received A then it will exacute condition that false value have entered so try again
    }
  }else {
      Serial.println("B");  // Indicate esp32 that Load cell is not ready.
    }
}

void setup() {
  Serial.begin(57600);
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);

  while (!Serial.available()) {
    // Wait for user input
  }
  char userInput = Serial.read();

  if (userInput == 'y' || userInput == 'Y') {
    
    calibrate();
  } else {
    
/*-------------------------Check if calibrated data is in EEPROM or not------------------------*/
    
    float eepromCalibration;
    EEPROM.get(EEPROM_ADDRESS, eepromCalibration);
    if (!isnan(eepromCalibration) && eepromCalibration != 0.0) {
      calibrationFactor = eepromCalibration;
    } else {
      calibrationFactor = DEFAULT_CALIBRATION_FACTOR; // if data is not available to the EEPROM then use deffault calibration factor
    }
  }

  Serial.println("C"); // tells esp32 to ask from user to weather to view weight in kg or in gram.
  delay(1000);
   scale.set_scale(calibrationFactor);
  scale.tare(); 
}

void loop() {
  if (Serial.available() > 0) {
    char userInput = Serial.read();

/* ----------------------This part is for unit conversion-------------------*/

   if (userInput == 'k'|| userInput == 'K') {
      // Toggle unit conversion
      if (currentUnit == GRAMS) {
        currentUnit = KILOGRAMS;
        Serial.println("Switched to kilograms");
      } else {
        currentUnit = GRAMS;
        Serial.println("Switched to grams");
      }
    }
  }

/* ----------------------Read weight from the HX711-------------------*/
  if (scale.wait_ready_timeout(200)) {
    reading = round(scale.get_units(10));
    inKg(reading);  // calling unit conversion function

    if (reading != lastReading) {
      lastReading = reading;
    }
  } else {
    Serial.println("HX711 not found.");
  }
}