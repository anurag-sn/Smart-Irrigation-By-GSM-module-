#include <SoftwareSerial.h>
#include <avr/wdt.h>

// Pins
const int GSM_RX_PIN = 2;
const int GSM_TX_PIN = 3;
const int MotorPin = 13;
const int soilMoisturePin = A0;
const int soilPowerPin = 7; // Sensor power

SoftwareSerial gsmSerial(GSM_TX_PIN, GSM_RX_PIN);

// Crop config
String crops[] = {"Wheat", "Rice", "Maize", "Potato"};
int cropThresholds[] = {500, 700, 550, 600};
int selectedCropIndex = 0;
int soilMoistureThreshold = cropThresholds[selectedCropIndex];

// Authorized numbers
String authorizedNumbers[5] = { "+91xxxxxxxxx" }; //enter the mobile number
int numOfAuthorizedNumbers = 1;

// Dynamic config
unsigned long SOIL_CHECK_INTERVAL = 120000;
unsigned long lastSoilCheckTime = 0;
bool autoIrrigation = false;

// GSM call handling
bool isRingPending = false;
String lastCaller = "";

void setup() {
  pinMode(MotorPin, OUTPUT);
  pinMode(soilPowerPin, OUTPUT);
  digitalWrite(MotorPin, LOW);
  digitalWrite(soilPowerPin, LOW);

  gsmSerial.begin(9600);
  Serial.begin(9600);
  Serial.println("Ready...");
  gsmSerial.println("ATE0");
  delay(800);
  gsmSerial.println("AT+CLIP=1");
  delay(800);
  gsmSerial.println("AT+CMGF=1");
  delay(800);

  wdt_enable(WDTO_8S); // Watchdog
}

void loop() {
  wdt_reset();

  // Auto irrigation
  if (autoIrrigation && millis() - lastSoilCheckTime >= SOIL_CHECK_INTERVAL) {
    checkSoilMoisture();
    lastSoilCheckTime = millis();
  }

  // GSM input
  while (gsmSerial.available()) {
    String inputLine = gsmSerial.readStringUntil('\n');
    inputLine.trim();
    if (inputLine.length() == 0) continue;
    Serial.print("GSM Serial: ");
    Serial.println(inputLine);

    // Call detection logic
    if (inputLine.indexOf("RING") != -1) {
      isRingPending = true;
      Serial.println("RING detected.");
    }
    if (inputLine.indexOf("+CLIP:") != -1 && isRingPending) {
      int start = inputLine.indexOf("\"") + 1;
      int end = inputLine.indexOf("\"", start);
      if (start > 0 && end > start) {
        lastCaller = inputLine.substring(start, end);
        Serial.print("Caller: ");
        Serial.println(lastCaller);

        bool authorized = false;
        for (int i = 0; i < numOfAuthorizedNumbers; i++) {
          if (lastCaller == authorizedNumbers[i]) {
            authorized = true;
            break;
          }
        }
        if (authorized) {
          digitalWrite(MotorPin, !digitalRead(MotorPin)); // Toggle motor
          Serial.println("Motor toggled for authorized call.");
        } else {
          Serial.println("Unauthorized call. No action.");
        }
        gsmSerial.println("ATH"); // Hang up
        Serial.println("Call disconnected.");
      }
      isRingPending = false;
    }
    // SMS handling: read next line after +CMT: as message
    if (inputLine.indexOf("+CMT:") != -1) {
      handleIncomingSMS(inputLine);
    }
  }
}

// Sensor reading
int readSoilMoisture() {
  digitalWrite(soilPowerPin, HIGH);
  delay(500);
  long sum = 0;
  for (int i = 0; i < 10; i++) {
    sum += analogRead(soilMoisturePin);
    delay(10);
  }
  digitalWrite(soilPowerPin, LOW);
  return sum / 10;
}

void checkSoilMoisture() {
  int soilMoistureValue = readSoilMoisture();
  Serial.print("Soil: ");
  Serial.println(soilMoistureValue);
  digitalWrite(MotorPin, soilMoistureValue > soilMoistureThreshold ? HIGH : LOW);
}

// SMS Handler (reads next line as actual message)
void handleIncomingSMS(String cmtLine) {
  // Wait until the message body is available
  while (!gsmSerial.available());
  String msgLine = gsmSerial.readStringUntil('\n');
  msgLine.trim();

  String senderNumber = extractSenderNumber(cmtLine);

  Serial.println("SMS Detected:");
  Serial.print("From: "); Serial.println(senderNumber);
  Serial.print("Msg: "); Serial.println(msgLine);

  String cmd = msgLine;
  cmd.toLowerCase();

  processCommand(cmd, senderNumber);
}

String extractSenderNumber(String inputString) {
  int numberStart = inputString.indexOf("+91");
  int numberEnd = inputString.indexOf("\",\"", numberStart);
  if (numberStart == -1 || numberEnd == -1) return "";
  String senderNumber = inputString.substring(numberStart, numberEnd);
  senderNumber.trim();
  return senderNumber;
}

void processCommand(String command, String senderNumber) {
  bool isAuthorized = false;
  for (int i = 0; i < numOfAuthorizedNumbers; i++) {
    if (senderNumber == authorizedNumbers[i]) {
      isAuthorized = true;
      break;
    }
  }

  if (command == "status") {
    sendStatus(senderNumber);

    
  } else if (command == "ai on" && isAuthorized) {
    autoIrrigation = true;
    sendSMS(senderNumber, "Auto irrigation enabled.");


  } else if (command == "ai off" && isAuthorized) {
    autoIrrigation = false;
    sendSMS(senderNumber, "Auto irrigation disabled.");



  } else if (command.startsWith("crop:") && isAuthorized) {
    String cropName = command.substring(5); cropName.trim(); selectCrop(cropName, senderNumber);
  
  
  } else if (command == "chal" && isAuthorized) {
    digitalWrite(MotorPin, HIGH); sendSMS(senderNumber, "Motor is ON");
  
  
  } else if (command == "band" && isAuthorized) {
    digitalWrite(MotorPin, LOW); sendSMS(senderNumber, "Motor is OFF");
  
  
  } else if (command.startsWith("set threshold:") && isAuthorized) {
    int newThreshold = command.substring(14).toInt();
  
    if (newThreshold > 0) {
      soilMoistureThreshold = newThreshold;
      sendSMS(senderNumber, "Threshold set to " + String(newThreshold));
  
  
    } else sendSMS(senderNumber, "Invalid threshold.");
  
  
  } else if (command.startsWith("set interval:") && isAuthorized) {
    unsigned long newInterval = command.substring(13).toInt() * 1000;
  
    if (newInterval >= 60000 && newInterval <= 3600000) {
      SOIL_CHECK_INTERVAL = newInterval;
      sendSMS(senderNumber, "Interval set to " + String(newInterval / 1000) + " seconds.");
  
  
    } else sendSMS(senderNumber, "Invalid interval.");
  
  
  } else if (command.startsWith("add auth:") && isAuthorized) {
    String newNumber = command.substring(9); newNumber.trim();
    if (numOfAuthorizedNumbers < 5 && newNumber.startsWith("+91")) {
      bool alreadyPresent = false;
      for (int i = 0; i < numOfAuthorizedNumbers; i++)
        if (authorizedNumbers[i] == newNumber) alreadyPresent = true;
      if (!alreadyPresent) {
        authorizedNumbers[numOfAuthorizedNumbers++] = newNumber;
        sendSMS(senderNumber, "Authorized added: " + newNumber);
      } else sendSMS(senderNumber, "Already authorized.");
    } else sendSMS(senderNumber, "Limit reached or invalid format.");
  
  
  
  } else if (command.startsWith("remove auth:") && isAuthorized) {
    String delNumber = command.substring(12); delNumber.trim();
    bool removed = false;
    for (int i = 0; i < numOfAuthorizedNumbers; i++) {
      if (authorizedNumbers[i] == delNumber) {
        for (int j = i; j < numOfAuthorizedNumbers - 1; j++)
          authorizedNumbers[j] = authorizedNumbers[j + 1];
        numOfAuthorizedNumbers--;
        sendSMS(senderNumber, "Authorized removed: " + delNumber);
        removed = true;
        break;
      }
    }
    if (!removed) sendSMS(senderNumber, "Number not found.");
  
  
  
  } else {
    sendSMS(senderNumber, "Invalid or unauthorized command.");
  }
}

void sendStatus(String number) {
  int soilMoistureValue = readSoilMoisture();
  String status = "Soil: " + String(soilMoistureValue) +
    "\nMotor: " + (digitalRead(MotorPin) == HIGH ? "ON" : "OFF") +
    "\nAuto: " + (autoIrrigation ? "Enabled" : "Disabled") +
    "\nThreshold: " + String(soilMoistureThreshold) +
    "\nInterval: " + String(SOIL_CHECK_INTERVAL / 1000) +
    "\nAuthorized:";
  for (int i = 0; i < numOfAuthorizedNumbers; i++)
    status += "\n" + authorizedNumbers[i];
  sendSMS(number, status);
}

void selectCrop(String cropName, String senderNumber) {
  cropName.toLowerCase();
  for (int i = 0; i < 4; i++) {
    String c = crops[i]; c.toLowerCase();
    if (cropName == c) {
      selectedCropIndex = i; soilMoistureThreshold = cropThresholds[i];
      sendSMS(senderNumber, "Crop: " + crops[i] + ". Threshold: " + String(soilMoistureThreshold));
      return;
    }
  }
  sendSMS(senderNumber, "Crop not found.");
}

void sendSMS(String number, String text) {
  gsmSerial.print("AT+CMGS=\""); gsmSerial.print(number); gsmSerial.println("\"");
  delay(1000);
  gsmSerial.print(text);
  delay(1000);
  gsmSerial.write(26); 
  delay(1000);
}