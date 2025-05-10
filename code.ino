#include <SoftwareSerial.h>

SoftwareSerial gsmSerial(2,3); //rx , tx
int MotorPin = 8;
int soilMoisturePin = A0;

const int dryValue = 800;
const int wetValue = 300;

String crops[] = {"Wheat", "Rice", "Maize", "Potato"};
int cropThresholds[] = {500, 700, 550, 600};

String authorizedNumbers[] = {
    "+91xxxxxxxxx"
};
int numOfAuthorizedNumbers = sizeof(authorizedNumbers) / sizeof(authorizedNumbers[0]);
bool autoIrrigation = false;

int selectedCropIndex = 0;
int soilMoistureThreshold = cropThresholds[selectedCropIndex];

const String CMD_STATUS = "status";
const String CMD_AUTO_ON = "ai on";
const String CMD_AUTO_OFF = "ai off";
const String CMD_CROP = "crop:";
const String CMD_MOTOR_ON = "chal";
const String CMD_MOTOR_OFF = "band";
const String CMD_SWITCH = "switch";

void setup() {
    pinMode(MotorPin, OUTPUT);
    gsmSerial.begin(9600);
    Serial.begin(9600);
    Serial.println("Ready...");
    initializeGSM();
}

void loop() {
    if (autoIrrigation) {
        checkSoilMoisture();
    }

    if (gsmSerial.available()) {
        String inputString = gsmSerial.readString();
        Serial.print("Input String: ");
        Serial.println(inputString);

        if (inputString.indexOf("RING") != -1) {
            handleIncomingCall(inputString);
        }

        if (inputString.indexOf("+CMT:") != -1) {
            handleIncomingSMS(inputString);
        }
    }
}

void initializeGSM() {
    gsmSerial.println("ATE0");
    delay(1000);
    gsmSerial.println("AT+CLIP=1");
    delay(1000);
    gsmSerial.println("AT+CMGF=1");
    delay(1000);
}

void checkSoilMoisture() {
    int soilMoistureValue = analogRead(soilMoisturePin);
    Serial.print("Soil Moisture Level: ");
    Serial.println(soilMoistureValue);
    digitalWrite(MotorPin, soilMoistureValue > soilMoistureThreshold ? HIGH : LOW);
    delay(10000);
}

void handleIncomingCall(String inputString) {
    Serial.println("Incoming Call Detected");
    for (int i = 0; i < numOfAuthorizedNumbers; i++) {
        if (inputString.indexOf(authorizedNumbers[i]) != -1) {
            Serial.println("Authorized Number Calling: " + authorizedNumbers[i]);
            toggleMOTOR();
            delay(1000);
            gsmSerial.println("ATA ");
            delay(5000);
            gsmSerial.println("ATH");
            delay(1000);
            break;
        }
    }
}

void handleIncomingSMS(String inputString) {
    Serial.println("Incoming SMS Detected");
    String senderNumber = extractSenderNumber(inputString);
    String message = extractMessage(inputString);
    Serial.print("Sender Number: ");
    Serial.println(senderNumber);
    Serial.print("Received command: ");
    Serial.println(message);

    String lowerCaseMessage = message;
    lowerCaseMessage.toLowerCase();

    for (int i = 0; i < numOfAuthorizedNumbers; i++) {
        if (senderNumber.indexOf(authorizedNumbers[i]) != -1) {
            Serial.println("Authorized number found");
            processCommand(lowerCaseMessage, senderNumber);
            break;
        }
    }
}

String extractSenderNumber(String inputString) {
    int numberStart = inputString.indexOf("+91");
    int numberEnd = inputString.indexOf("\",\"", numberStart);
    String senderNumber = inputString.substring(numberStart, numberEnd);
    senderNumber.trim();
    return senderNumber;
}

String extractMessage(String inputString) {
    int numberEnd = inputString.indexOf("\",\"", inputString.indexOf("+91"));
    int messageStart = inputString.indexOf("\n", numberEnd) + 1;
    String message = inputString.substring(messageStart);
    message.trim();
    return message;
}

void processCommand(String command, String senderNumber) {
    if (command.equals(CMD_STATUS)) {
        sendStatus(senderNumber);
    } else if (command.equals(CMD_AUTO_ON)) {
        autoIrrigation = true;
        sendSMS(senderNumber, "Auto irrigation enabled.");
    } else if (command.equals(CMD_AUTO_OFF)) {
        autoIrrigation = false;
        sendSMS(senderNumber, "Auto irrigation disabled.");
    } else if (command.startsWith(CMD_CROP)) {
        String cropName = command.substring(CMD_CROP.length());
        cropName.trim();
        selectCrop(cropName, senderNumber);
    } else if (command.equals(CMD_MOTOR_ON)) {
        Serial.println("Turning Motor ON");
        digitalWrite(MotorPin, HIGH);
        sendSMS(senderNumber, "Motor is ON");
    } else if (command.equals(CMD_MOTOR_OFF)) {
        Serial.println("Turning Motor OFF");
        digitalWrite(MotorPin, LOW);
        sendSMS(senderNumber, "Motor is OFF");
    } else if (command.equals(CMD_SWITCH)) {
        toggleMOTOR();
        sendSMS(senderNumber, "Motor state toggled.");
    }
}

void sendStatus(String number) {
    int soilMoistureValue = analogRead(soilMoisturePin);
    String statusMessage = "Soil Moisture Level: " + String(soilMoistureValue) +
                           "\nMotor is " + (digitalRead(MotorPin) == HIGH ? "ON" : "OFF") +
                           "\nAuto Irrigation: " + (autoIrrigation ? "Enabled" : "Disabled");
    sendSMS(number, statusMessage);
}

void selectCrop(String cropName, String senderNumber) {
    cropName.toLowerCase();
    for (int i = 0; i < sizeof(crops) / sizeof(crops[0]); i++) {
        String cropLower = crops[i];
        cropLower.toLowerCase();
        if (cropName.equals(cropLower)) {
            selectedCropIndex = i;
            soilMoistureThreshold = cropThresholds[selectedCropIndex];
            Serial.print("Selected Crop: ");
            Serial.println(crops[selectedCropIndex]);
            Serial.print("New Soil Moisture Threshold: ");
            Serial.println(soilMoistureThreshold);
            sendSMS(senderNumber, "Crop selected: " + crops[selectedCropIndex] + ". New threshold: " + String(soilMoistureThreshold));
            return;
        }
    }
    sendSMS(senderNumber, "Crop not found.");
}

void toggleMOTOR() {
    static bool motorState = false;
    motorState = !motorState;
    digitalWrite(MotorPin, motorState ? HIGH : LOW);
}

void sendSMS(String number, String text) {
    Serial.print("Sending SMS to: ");
    Serial.println(number);
    gsmSerial.print("AT+CMGS=\"");
    gsmSerial.print(number);  
    gsmSerial.println("\"");
    delay(1000);
    printGSMResponse();

    gsmSerial.print(text);
    delay(1000);
    printGSMResponse();

    gsmSerial.write(26);
    delay(1000);
    printGSMResponse();
}

void printGSMResponse() {
    while (gsmSerial.available()) {
        Serial.write(gsmSerial.read());
    }
}
