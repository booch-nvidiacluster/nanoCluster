#include "Arduino.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include "Adafruit_FT6206.h"
#include "Fonts/FreeSans12pt7b.h"
#include "Fonts/FreeSansOblique9pt7b.h"
#include "SPI.h"
#include "Wire.h"                   
#include "wiring_private.h"

// Nodes ***********************************************************************s

#define clusterSize 4

const int poweringOnDelay = 2000;
const int confirmPowerOnDelay = 1000;
const int bootingDelay = 10000;
const int confirmBootedDelay = 1000;
const int loggingInDelay = 2000;
const int confirmLoggedInDelay = 0;
const int writeDelay = 50;
const int readDelay = 50;

const String userName = "nano";
const String password = "Turing7Test*";

const String poweredOffName = "Powered off";
const String poweringOnName = "Powering on...";
const String poweredOnName = "Powered on";
const String bootingName = "Booting...";
const String bootedName = "Booted";
const String loggingInName = "Logging in...";

const String defaultIP = "xxx.xxx.x.xx";
const String noIP = "";

const String requestHostname = "hostname";
const String requestIP = "ping -c 1 ";
const String requestPowerOff = "echo ";
const String requestPowerOffSuffix = " | sudo -S shutdown -P now";
const String requestCPUUtilization = "mpstat";
const String requestMemoryUtilization = "free";

const int requestUtilizationThreshold = (clusterSize * 2);

enum nodeState {poweredOff, poweringOn, poweredOn, booting, booted, loggingIn, loggedIn};

typedef struct node {
  int id;
  String name;
  String ip;
  int cpuUtilization;
  int memoryUtilization;
  boolean heartbeat;
  int heartbeatCount;
  nodeState state;

};

int totalNodesPoweredOn = 0;
int totalNodesBooted = 0;
int totalNodesLoggedIn = 0;

node nodes[clusterSize];

void initializeNodes() {

  for (int nodeNumber = 0; nodeNumber < clusterSize; nodeNumber++) {
    nodes[nodeNumber].id = nodeNumber;
    nodes[nodeNumber].name = poweredOffName;
    nodes[nodeNumber].ip = defaultIP;
    nodes[nodeNumber].cpuUtilization = 0;
    nodes[nodeNumber].memoryUtilization = 0;
    nodes[nodeNumber].state = poweredOff;
    nodes[nodeNumber].heartbeat = false;
    nodes[nodeNumber].heartbeatCount = 0;
  }
  
}

// Node serial communication ***************************************************

#define serial2TX 14
#define serial2RX 15
#define serial3TX 16
#define serial3RX 17
#define serial4TX 18
#define serial4RX 19

const int baudRate = 115200;
const int newLine = 10;

Uart Serial2(&sercom5, serial2TX, serial2RX, SERCOM_RX_PAD_1, UART_TX_PAD_0);
Uart Serial3(&sercom1, serial3TX, serial3RX, SERCOM_RX_PAD_1, UART_TX_PAD_0);
Uart Serial4(&sercom4, serial4TX, serial4RX, SERCOM_RX_PAD_1, UART_TX_PAD_0);

void SERCOM1_0_Handler() {Serial3.IrqHandler();}
void SERCOM1_1_Handler() {Serial3.IrqHandler();}
void SERCOM1_2_Handler() {Serial3.IrqHandler();}
void SERCOM1_3_Handler() {Serial3.IrqHandler();}
 
void SERCOM4_0_Handler() {Serial4.IrqHandler();}
void SERCOM4_1_Handler() {Serial4.IrqHandler();}
void SERCOM4_2_Handler() {Serial4.IrqHandler();}
void SERCOM4_3_Handler() {Serial4.IrqHandler();}

void SERCOM5_0_Handler() {Serial2.IrqHandler();}
void SERCOM5_1_Handler() {Serial2.IrqHandler();}
void SERCOM5_2_Handler() {Serial2.IrqHandler();}
void SERCOM5_3_Handler() {Serial2.IrqHandler();}

void initializeSerialCommunication() {
  
  pinPeripheral(serial2TX, PIO_SERCOM_ALT);
  pinPeripheral(serial2RX, PIO_SERCOM_ALT);
  pinPeripheral(serial3TX, PIO_SERCOM_ALT);
  pinPeripheral(serial3RX, PIO_SERCOM_ALT);
  pinPeripheral(serial4TX, PIO_SERCOM_ALT);
  pinPeripheral(serial4RX, PIO_SERCOM_ALT);

  Serial1.begin(baudRate);
  Serial2.begin(baudRate);
  Serial3.begin(baudRate);
  Serial4.begin(baudRate);

}

boolean serialIsAvailable(int nodeNumber) {

  switch (nodeNumber) {
    case 0: return Serial1.available();
    case 1: return Serial2.available();
    case 2: return Serial3.available();
    case 3: return Serial4.available();
    default: return false;
  }
  
}

int readSerial(int nodeNumber) 
{

  switch (nodeNumber) {
    case 0: return Serial1.read();
    case 1: return Serial2.read();
    case 2: return Serial3.read();
    case 3: return Serial4.read();
    default: return 0;
  }
  
}

String readSerialLine(int nodeNumber) {

  int output = 0;
  
  String line = "";

  delay(readDelay);
       
  while (serialIsAvailable(nodeNumber)) {
    output = readSerial(nodeNumber);
    if (output == newLine)
      break;
    line = line + char(output);
  }
  return line;
  
}

void writeSerial(int nodeNumber, int output) {

  switch (nodeNumber) {
    case 0: Serial1.write(output); break;
    case 1: Serial2.write(output); break;
    case 2: Serial3.write(output); break;
    case 3: Serial4.write(output); break;
  }
  
}

void writeSerialLine(int nodeNumber, String line) {

  for (int characterPosition = 0; characterPosition < line.length(); characterPosition++)
    writeSerial(nodeNumber, line[characterPosition]);
  writeSerial(nodeNumber, newLine);
 
  delay(writeDelay);

}

void flushSerial(int nodeNumber) {

  int output;
  
  while (serialIsAvailable(nodeNumber))
    output = readSerial(nodeNumber);
  
}

String parseIP(String line) {

  int leftParenthesis;
  int rightParenthesis;

  leftParenthesis = line.indexOf('(');
  rightParenthesis = line.indexOf(')');
  return line.substring(leftParenthesis + 1, rightParenthesis);
  
}

 int parseCPUUtilization(String line) {

  int cpuUtilization = -1;

  String value;

  int leftDigit;
  int rightDigit;

  leftDigit = line.lastIndexOf(' ') + 1;
  rightDigit = line.length() - 1;
  if ((leftDigit >= 0) && (rightDigit >= 0) && (leftDigit < rightDigit)) {
    value = line.substring(leftDigit, rightDigit);
    cpuUtilization = 100 - (int)value.toFloat();
  }
  return cpuUtilization;

 }

 int parseMemoryUtilization(String line) {

  unsigned long memoryUtilization = -1;
  unsigned long totalMemory = 0;
  unsigned long availableMemory = 0;
 
  String value;

  int leftDigit;
  int rightDigit;

  leftDigit = -1;
  for (int characterPosition = 0; characterPosition < line.length(); characterPosition++)
    if (isDigit(line.charAt(characterPosition))) {
      leftDigit = characterPosition;
      break;
    }
  rightDigit = line.indexOf(' ', leftDigit);
  if ((leftDigit >= 0) && (rightDigit >= 0) && (leftDigit < rightDigit)) {
    value = line.substring(leftDigit, rightDigit);
    totalMemory = value.toInt();
  }
  leftDigit = line.lastIndexOf(' ') + 1;
  rightDigit = line.length() - 1;
  if ((leftDigit >= 0) && (rightDigit >= 0) && (leftDigit < rightDigit)) {
    value = line.substring(leftDigit, rightDigit);
    availableMemory = value.toInt();
  }
  if ((totalMemory > 0) && (availableMemory > 0))
    memoryUtilization = 100 - (availableMemory/(totalMemory / 100));

  return memoryUtilization;
  
 }

// Node serial buffers *********************************************************

const int maximumLineLength = SERIAL_BUFFER_SIZE;

enum bufferState {waiting, receivedL, receivedO, receivedG, receivedI, receivedN, loginReceived};

typedef struct serialBuffer {
  int line[maximumLineLength];
  int index;
  bufferState state;
};

serialBuffer serialBuffers[clusterSize];

void initializeSerialBuffers() {

  for (int nodeNumber = 0; nodeNumber < clusterSize; nodeNumber++) {
    serialBuffers[nodeNumber].index = 0;
    serialBuffers[nodeNumber].state = waiting;
  };
  
}

// Node GPIO *******************************************************************

#define node0PowerState 22
#define node1PowerState 24
#define node2PowerState 26
#define node3PowerState 28

#define node0Heartbeat 30
#define node1Heartbeat 32
#define node2Heartbeat 34
#define node3Heartbeat 36

const boolean isPoweredOn = HIGH;

void initializeGPIO() {

  pinMode(node0PowerState, INPUT);
  pinMode(node1PowerState, INPUT);
  pinMode(node2PowerState, INPUT);
  pinMode(node3PowerState, INPUT);

  pinMode(node0Heartbeat, INPUT);
  pinMode(node1Heartbeat, INPUT);
  pinMode(node2Heartbeat, INPUT);
  pinMode(node3Heartbeat, INPUT);
   
}

boolean readPowerStatus(int nodeNumber) {

   switch(nodeNumber) {
     case 0: return digitalRead(node0PowerState);
     case 1: return digitalRead(node1PowerState);
     case 2: return digitalRead(node2PowerState);
     case 3: return digitalRead(node3PowerState);
     default: return !isPoweredOn;
   }
   
}

boolean readHeartbeat(int nodeNumber) {

   switch(nodeNumber) {
     case 0: return digitalRead(node0Heartbeat);
     case 1: return digitalRead(node1Heartbeat);
     case 2: return digitalRead(node2Heartbeat);
     case 3: return digitalRead(node3Heartbeat);
     default: return false;
   }
   
}

// Display *********************************************************************

#define TFT_DC 9
#define TFT_CS 10

const int fontHeightFreeSans12pt7b = 17;
const int fontIndentFreeSans12pt7b = 7;

const int fontHeightFreeSansOblique9pt7b = 12;
const int fontIndentFreeSansOblique9pt7b = 4;

const int displayWidth = 320;
const int displayHeight = 240;
const int displayBackgroundColor = ILI9341_BLACK;

const int nameBackgroundColor = ILI9341_WHITE;
const int nameColor = ILI9341_BLACK;
const int nameVerticalPadding = 3;
const int nameHeight = fontHeightFreeSans12pt7b + (nameVerticalPadding * 2) + 1;

const int ipBackgroundColor = ILI9341_WHITE;
const int ipColor = ILI9341_BLACK;
const int ipVerticalPadding = 3;
const int ipHeight = fontHeightFreeSansOblique9pt7b + (ipVerticalPadding * 2) + 1;

const int utilizationBackgroundColor = ILI9341_BLACK;
const int cpuUtilizationColor = 0x8410;
const int memoryUtilizationColor = 0x04FF;
const int utilizationHeight = 5;

const int dividerHeight = 9;

const int nodeHeight = nameHeight + ipHeight + (utilizationHeight * 2) + dividerHeight;

const int powerBackgroundColor = ILI9341_WHITE;
const int powerOnColor = ILI9341_GREEN;
const int powerOffColor = ILI9341_RED;
const int powerVerticalOffset = 21;
const int powerHorizontalOffset = displayWidth - 26;
const int powerOuterRadius = 16;
const int powerMiddleRadius = 11;
const int powerInnerRadius = 8;
const int powerMaskWidth = 4;
const int powerMaskOffset = 2;
const int powerSwitchWidth = 1;
const int powerSwitchOffset = -1;

const int tickColor = powerOnColor;
const int tockColor = ILI9341_PURPLE;
const int heartbeatVerticalOffset = powerVerticalOffset;
const int heartbeatHorizontalOffset = powerHorizontalOffset;
const int heartbeatInnerRadius = 2;

Adafruit_ILI9341 display = Adafruit_ILI9341(TFT_CS, TFT_DC);

void initializeDisplay() {

  display.begin();
  display.fillScreen(displayBackgroundColor);
  display.setRotation(3);
  
}

void displayName(int nodeNumber, String name) {

  int nodePosition = nodeNumber * nodeHeight;

  display.fillRect(0, nodePosition, displayWidth - 1, fontHeightFreeSans12pt7b + (nameVerticalPadding * 2) + 3, nameBackgroundColor);
  display.setFont(&FreeSans12pt7b);
  display.setTextColor(nameColor);
  display.setCursor(fontIndentFreeSans12pt7b, nodePosition + fontHeightFreeSans12pt7b + nameVerticalPadding);
  display.println(name);

}

void displayIP(int nodeNumber, String iP) {

  int nodePosition = nodeNumber * nodeHeight + nameHeight;

  display.fillRect(0, nodePosition, displayWidth - 1, fontHeightFreeSansOblique9pt7b + (ipVerticalPadding * 2) + 1, ipBackgroundColor);
  display.setFont(&FreeSansOblique9pt7b);
  display.setTextColor(ipColor);
  display.setCursor(fontIndentFreeSansOblique9pt7b, nodePosition + fontHeightFreeSansOblique9pt7b + ipVerticalPadding);
  display.println(iP);

}

void displayCPUUtilization(int nodeNumber, int oldCPUUtilization, int newCPUUtilization) {

  int nodePosition = nodeNumber * nodeHeight + nameHeight + ipHeight;
  
  if (newCPUUtilization >= oldCPUUtilization)
    display.fillRect(0, nodePosition, (((displayWidth - 1) * newCPUUtilization) / 100), utilizationHeight, cpuUtilizationColor); 
  else
    display.fillRect((((displayWidth - 1) * newCPUUtilization) / 100), nodePosition, (((displayWidth - 1) * oldCPUUtilization) / 100), utilizationHeight, utilizationBackgroundColor);

}

void displayMemoryUtilization(int nodeNumber, int oldMemoryUtilization, int newMemoryUtilization) {

  int nodePosition = nodeNumber * nodeHeight + nameHeight + ipHeight + utilizationHeight;
  
  if (newMemoryUtilization >= oldMemoryUtilization)
    display.fillRect(0, nodePosition, (((displayWidth - 1) * newMemoryUtilization) / 100), utilizationHeight, memoryUtilizationColor); 
  else
    display.fillRect((((displayWidth - 1) * newMemoryUtilization) / 100), nodePosition, (((displayWidth - 1) * oldMemoryUtilization) / 100), utilizationHeight, utilizationBackgroundColor);
  
}

void displayPower(int nodeNumber, boolean power) {

  int nodePosition = nodeNumber * nodeHeight;
  
  int buttonColor;

  if (power)
    buttonColor = powerOnColor; 
  else
    buttonColor = powerOffColor;
  
  display.fillCircle(powerHorizontalOffset, nodePosition + powerVerticalOffset, powerOuterRadius, buttonColor);
  display.fillCircle(powerHorizontalOffset, nodePosition + powerVerticalOffset, powerMiddleRadius, powerBackgroundColor);
  display.fillCircle(powerHorizontalOffset, nodePosition + powerVerticalOffset, powerInnerRadius, buttonColor);
  display.fillRect(powerHorizontalOffset - powerMaskWidth, nodePosition + powerVerticalOffset - powerOuterRadius, powerMaskWidth * 2 + 1, powerOuterRadius + powerMaskOffset, buttonColor);
  display.fillRect(powerHorizontalOffset - powerSwitchWidth, nodePosition + powerVerticalOffset - powerOuterRadius + 3,  powerSwitchWidth * 2 + 1, powerOuterRadius + powerSwitchOffset, powerBackgroundColor);  

}

void displayHeartbeat(int nodeNumber, boolean value) {

  int nodePosition = nodeNumber * nodeHeight;

  int heartbeatColor;

  if (value)
    heartbeatColor = tickColor;
  else
    heartbeatColor = tockColor;

  display.fillCircle(heartbeatHorizontalOffset, nodePosition + heartbeatVerticalOffset, heartbeatInnerRadius, heartbeatColor);

}

// Touchscreen *****************************************************************

const int touchSensitivity = 40;
const int touchDelay = 250;

const int touchBoxPadding = 10;
const int touchBoxOffset = touchBoxPadding / 2;
const int touchBoxSize = (powerOuterRadius * 2) + touchBoxPadding;

typedef struct touchBox {
  TS_Point upperLeft;
  TS_Point bottomRight;
};

touchBox touchBoxes[clusterSize];

Adafruit_FT6206 touchscreen = Adafruit_FT6206();

void initializeTouchscreen() {

  if (!touchscreen.begin(touchSensitivity))
    while(true);  

  for (int nodeNumber = 0; nodeNumber < clusterSize; nodeNumber++) {
    touchBoxes[nodeNumber].upperLeft.x = powerHorizontalOffset - powerOuterRadius - touchBoxOffset;
    touchBoxes[nodeNumber].upperLeft.y = (nodeNumber * nodeHeight) + powerVerticalOffset - powerOuterRadius - touchBoxOffset;
    touchBoxes[nodeNumber].bottomRight.x = touchBoxes[nodeNumber].upperLeft.x + touchBoxSize;
    touchBoxes[nodeNumber].bottomRight.y = touchBoxes[nodeNumber].upperLeft.y + touchBoxSize;
  }
  
}

// *****************************************************************************

void poweringOnNodes() {
  
  for (int nodeNumber = 0; nodeNumber < clusterSize; nodeNumber++) { 
    displayName(nodeNumber, poweringOnName);
    displayIP(nodeNumber, defaultIP);
    nodes[nodeNumber].name = poweringOn;
    nodes[nodeNumber].state = poweringOn;
  }

  delay(poweringOnDelay);
    
}

void confirmPoweredOnNodes() {

  for (int nodeNumber = 0; nodeNumber < clusterSize; nodeNumber++) {
    if (readPowerStatus(nodeNumber) == isPoweredOn) {
      displayName(nodeNumber, poweredOnName);
      displayPower(nodeNumber, true);
      nodes[nodeNumber].name = poweredOnName;
      nodes[nodeNumber].state = poweredOn;
      totalNodesPoweredOn++;
    } else {
      displayName(nodeNumber, poweredOffName);
      displayIP(nodeNumber, noIP);
      nodes[nodeNumber].name = poweredOffName;
      nodes[nodeNumber].ip = noIP;
      nodes[nodeNumber].state = poweredOff;
    }
  }

  delay(confirmPowerOnDelay);

}

void bootingNodes () {

  for (int nodeNumber = 0; nodeNumber < clusterSize; nodeNumber++) {
    if (nodes[nodeNumber].state == poweredOn) {
      displayName(nodeNumber, bootingName);
      displayPower(nodeNumber, true);
      nodes[nodeNumber].name = bootingName;
      nodes[nodeNumber].state = booting;
    }
  }

  delay(bootingDelay);
  
}

void confirmBootedNodes() {

  int input;
  
  while (totalNodesBooted < totalNodesPoweredOn) {
    for (int nodeNumber = 0; nodeNumber < clusterSize; nodeNumber++) {
      if (nodes[nodeNumber].state == booting) {
        if (serialIsAvailable(nodeNumber)) {
          input = readSerial(nodeNumber);
          if (input == newLine) {
            serialBuffers[nodeNumber].index = 0;
          } else {
            serialBuffers[nodeNumber].line[serialBuffers[nodeNumber].index] = input;
            serialBuffers[nodeNumber].index++;
            switch (serialBuffers[nodeNumber].state) {
             case waiting:
                if (input == 'l')
                  serialBuffers[nodeNumber].state = receivedL;
                break;
              case receivedL:
                if (input == 'o')
                  serialBuffers[nodeNumber].state = receivedO;
                else
                  serialBuffers[nodeNumber].state = waiting;
                break;
              case receivedO:
                if (input == 'g')
                  serialBuffers[nodeNumber].state = receivedG;
                else
                  serialBuffers[nodeNumber].state = waiting;
                break;
              case receivedG:
                if (input == 'i')
                  serialBuffers[nodeNumber].state = receivedI;
                else
                  serialBuffers[nodeNumber].state = waiting;
                break;
              case receivedI:
                if (input == 'n')
                  serialBuffers[nodeNumber].state = receivedN;
                else
                  serialBuffers[nodeNumber].state = waiting;
                 break;
              case receivedN:
                if (input == ':') {
                  serialBuffers[nodeNumber].state = loginReceived;
                  displayName(nodeNumber, bootedName);
                  displayPower(nodeNumber, true);
                  nodes[nodeNumber].name = bootedName;
                  nodes[nodeNumber].state = booted;
                   totalNodesBooted++;
                } else
                  serialBuffers[nodeNumber].state = waiting;
              case loginReceived:
                readSerial(nodeNumber);
                break;
            }  
          }
        }
      }
    }
  }
  
  delay(confirmBootedDelay); 

}

void loggingInNodes() {

  for (int nodeNumber = 0; nodeNumber < clusterSize; nodeNumber++) {
    if (nodes[nodeNumber].state == booted) {
      writeSerialLine(nodeNumber, userName);
      writeSerialLine(nodeNumber, password);
      displayName(nodeNumber, loggingInName);
      displayPower(nodeNumber, true);
      nodes[nodeNumber].name = loggingInName;
      nodes[nodeNumber].state = loggingIn;
      totalNodesBooted--;
      totalNodesLoggedIn++;
    }
  }        

  delay(loggingInDelay);

  for (int nodeNumber = 0; nodeNumber < clusterSize; nodeNumber++)
    if (nodes[nodeNumber].state == loggingIn)
      flushSerial(nodeNumber);

}

void confirmLoggedInNodes() {

  String name;
  String ip;

  for (int nodeNumber = 0; nodeNumber < clusterSize; nodeNumber++) {
    if (nodes[nodeNumber].state == loggingIn) {
      writeSerialLine(nodeNumber, requestHostname);
      readSerialLine(nodeNumber);
      name = readSerialLine(nodeNumber);
      displayName(nodeNumber, name);
      displayPower(nodeNumber, true);
      nodes[nodeNumber].name = name;
      nodes[nodeNumber].state = loggedIn;
      flushSerial(nodeNumber);
    }
  }

  delay(confirmLoggedInDelay);
       
  for (int askingNodeNumber = 0; askingNodeNumber < clusterSize; askingNodeNumber++) {
    if (nodes[askingNodeNumber].state == loggedIn)  {
      for (int askedNodeNumber = 0; askedNodeNumber < clusterSize; askedNodeNumber++) {
        if (askingNodeNumber != askedNodeNumber)
          if (nodes[askedNodeNumber].ip.equals(defaultIP)) {
            writeSerialLine(askingNodeNumber, requestIP + nodes[askedNodeNumber].name);
            readSerialLine(askingNodeNumber);
            ip = parseIP(readSerialLine(askingNodeNumber));
            if (isDigit(ip.charAt(0)) && isDigit(ip.charAt(1))) {
              displayIP(askedNodeNumber, ip);
              displayPower(askedNodeNumber, true);
              nodes[askedNodeNumber].ip = ip;
             }
            flushSerial(askingNodeNumber);
          }
      }
    }
  }

  delay(confirmLoggedInDelay);
  
}

void checkTouch() {

  TS_Point rawTouchPoint, touchPoint;

  int selection = clusterSize;

  if (!touchscreen.touched())
    return;
  else {
    rawTouchPoint = touchscreen.getPoint();
    touchPoint.x = displayWidth - map(rawTouchPoint.y, 0, displayWidth, displayWidth, 0);
    touchPoint.y = map(rawTouchPoint.x, 0, displayHeight, displayHeight, 0);
    for (int nodeNumber = 0; nodeNumber < clusterSize; nodeNumber++)
      if ((touchBoxes[nodeNumber].upperLeft.x <= touchPoint.x) && (touchPoint.x <= touchBoxes[nodeNumber].bottomRight.x))
        if ((touchBoxes[nodeNumber].upperLeft.y <= touchPoint.y) && (touchPoint.y <= touchBoxes[nodeNumber].bottomRight.y)) {
          selection = nodeNumber;
          break;
        }
    if (selection != clusterSize)
      if (nodes[selection].state != poweredOff) {
        writeSerialLine(selection, requestPowerOff + password + requestPowerOffSuffix);
        nodes[selection].state = poweredOff;
        displayPower(selection, false);
        totalNodesPoweredOn--;
        totalNodesBooted--;
        totalNodesLoggedIn--;
        delay(touchDelay);
      }
  }
  
}

void listenToHeartbeat() {

  String line;

  int newCPUUtilization;
  int newMemoryUtilization;
  
  for (int nodeNumber = 0; nodeNumber < totalNodesLoggedIn; nodeNumber++)
    if (nodes[nodeNumber].state == loggedIn)
      if (nodes[nodeNumber].heartbeat != readHeartbeat(nodeNumber)) {
        nodes[nodeNumber].heartbeat = !nodes[nodeNumber].heartbeat;
        if (nodes[nodeNumber].heartbeatCount == nodeNumber) {
          writeSerialLine(nodeNumber, requestCPUUtilization);
          readSerialLine(nodeNumber);
          readSerialLine(nodeNumber);
          readSerialLine(nodeNumber);
          readSerialLine(nodeNumber);
          line = readSerialLine(nodeNumber);
          newCPUUtilization = parseCPUUtilization(line);
          if (newCPUUtilization >= 0) {
            displayCPUUtilization(nodeNumber, nodes[nodeNumber].cpuUtilization, newCPUUtilization);
            displayPower(nodeNumber, true);
            nodes[nodeNumber].cpuUtilization = newCPUUtilization;
          }
          flushSerial(nodeNumber);
        } else if (nodes[nodeNumber].heartbeatCount == (nodeNumber + clusterSize)) {
          writeSerialLine(nodeNumber, requestMemoryUtilization);
          readSerialLine(nodeNumber);
          readSerialLine(nodeNumber);
          line = readSerialLine(nodeNumber);
          newMemoryUtilization = parseMemoryUtilization(line);
          if (newMemoryUtilization >= 0) {
            displayMemoryUtilization(nodeNumber, nodes[nodeNumber].memoryUtilization, newMemoryUtilization);
            displayPower(nodeNumber, true);
            nodes[nodeNumber].memoryUtilization = newMemoryUtilization;
          }
          flushSerial(nodeNumber);
        }
        nodes[nodeNumber].heartbeatCount++;
        if (nodes[nodeNumber].heartbeatCount == (clusterSize * 2))
          nodes[nodeNumber].heartbeatCount = 0;
        displayHeartbeat(nodeNumber, nodes[nodeNumber].heartbeat);
      }
        
}

// *****************************************************************************

void setup() {

  initializeNodes();
  initializeSerialCommunication();
  initializeSerialBuffers();
  initializeGPIO();
  initializeDisplay();
  initializeTouchscreen();

  poweringOnNodes();
  confirmPoweredOnNodes();
  bootingNodes();
  confirmBootedNodes();
  loggingInNodes();
  confirmLoggedInNodes();

}

void loop(void) {

  checkTouch();

  listenToHeartbeat();

}
