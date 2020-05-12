// This is the software that powers the console in the nanoCluster. It serves
// two use cases: to display the state of each node in the cluster and to
// permit the user to power off individual nodes. The software is targeted to
// an Adafruit Grand Central, to which is attached a TFT Touch Shield. This
// software is conceptually simple. There are abstractions for each of the
// console's major components: an individual node, a serial communication
// channel for each node, a node's general purpose IO connections, the
// display, and the touchscreen. Because these are each just singletons,
// they are implemented not as classes but as simple data structures. Each
// node goes through a lifecycle - from powered off to powering on to
// powered on to booting to booted to logging in to logged in - and once
// logged in its CPU and memory utilization are queried regularly according
// to its heartbeat, until it is again powered off by user command.

#include "Arduino.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include "Adafruit_FT6206.h"
#include "Fonts/FreeSans12pt7b.h"
#include "Fonts/FreeSansOblique9pt7b.h"
#include "SPI.h"
#include "Wire.h"                   
#include "wiring_private.h"

// Nodes ***********************************************************************
// A cluster is composed of nodes, each of which represents an independent
// computational resource.

#define clusterSize 4

// In the lifecycle of a node, it takes time to bring each change of state to
// a stable place (for example, powering up takes a short amount of time before
// a node can start booting, while booting itself takes several seconds). These
// delays have no precise theoretic function behind them, but rather were
// gathered by experimentation.

const int poweringOnDelay = 2000;
const int confirmPowerOnDelay = 1000;
const int bootingDelay = 10000;
const int confirmBootedDelay = 1000;
const int loggingInDelay = 2000;
const int confirmLoggedInDelay = 0;
const int writeDelay = 50;
const int readDelay = 50;

// Each node's username and password must be set in accordance with how a
// node's software is configured. Having this information in the clear is
// admittedly not particuarly secure, but then, this cluster was never meant to
// be set loose in the wild.

const String userName = "xxxx";
const String password = "xxxxxxxxxxx";

// These strings label each state of a node over its lifecycle (until the
// node's real name can be discerned) and serve to make visible that state to
// the user.

const String poweredOffName = "Powered off";
const String poweringOnName = "Powering on...";
const String poweredOnName = "Powered on";
const String bootingName = "Booting...";
const String bootedName = "Booted";
const String loggingInName = "Logging in...";

// These strings label the IP of a node over its lifecycle (until the node's
// real IP can be discerned).

const String defaultIP = "xxx.xxx.x.xx";
const String noIP = "";

// These strings serve as the Ubntu terminal commands the console directs to
// each node during setup and operation.

const String requestHostname = "hostname";
const String requestIP = "ping -c 1 ";
const String requestPowerOff = "echo ";
const String requestPowerOffSuffix = " | sudo -S shutdown -P now";
const String requestCPUUtilization = "mpstat";
const String requestMemoryUtilization = "free";

// Each node passes through several different states during its lifecycle.

enum nodeState {poweredOff, poweringOn, poweredOn, booting, booted, loggingIn, loggedIn};

// An individual node is represented by a unique ID, a name and its IP address.
// It progresses through several different states and once logged in, its name
// and IP address are discerned and its heartbeat, CPU utiliization, and and
// memory utilization are reqularly queried.

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

// A cluster is composed of several individual nodes; collectively, we keep
// count of the number of nodes powered on, the number of nodes booted,
// and the number of nodes logged in.

int totalNodesPoweredOn = 0;
int totalNodesBooted = 0;
int totalNodesLoggedIn = 0;

node nodes[clusterSize];

// initializeNodes is a constructor that sets the initial state of an
// individual node.

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
// Each node communicates to the console via its own serial channel, which
// means that the console must embrace a different channel for each node in
// the cluster. One channel (Serial1) is predefined in the Grand Central
// environment, but the other three must be configured. Each node's serial
// channel is used to send commands from the console to the node (specifically,
// to query the node's name, IP address, CPU utilization, and memory
// utilization, and to discern if the node is fully booted) as well as to
// receive responses from those commands and to metabolize output from the
// booting process.

// Here we define the pin assignments for the three additional serial channels.

#define serial2TX 14
#define serial2RX 15
#define serial3TX 16
#define serial3RX 17
#define serial4TX 18
#define serial4RX 19

// Here we define the baud rate of each channel, together with a constant
// representing the value of a new line sent or received during transmission.

const int baudRate = 115200;
const int newLine = 10;

// Here we declare the three additional serial channels, together with
// helper functions required by the Grand Central to connect each
// channel to its approprite interrupt handler (which are part of the
// Uart class definition).

Uart Serial2(&sercom5, serial2TX, serial2RX, SERCOM_RX_PAD_1, UART_TX_PAD_0);
Uart Serial3(&sercom1, serial3TX, serial3RX, SERCOM_RX_PAD_1, UART_TX_PAD_0);
Uart Serial4(&sercom4, serial4TX, serial4RX, SERCOM_RX_PAD_1, UART_TX_PAD_0);

void SERCOM5_0_Handler() {Serial2.IrqHandler();}
void SERCOM5_1_Handler() {Serial2.IrqHandler();}
void SERCOM5_2_Handler() {Serial2.IrqHandler();}
void SERCOM5_3_Handler() {Serial2.IrqHandler();}

void SERCOM4_0_Handler() {Serial4.IrqHandler();}
void SERCOM4_1_Handler() {Serial4.IrqHandler();}
void SERCOM4_2_Handler() {Serial4.IrqHandler();}
void SERCOM4_3_Handler() {Serial4.IrqHandler();}

void SERCOM1_0_Handler() {Serial3.IrqHandler();}
void SERCOM1_1_Handler() {Serial3.IrqHandler();}
void SERCOM1_2_Handler() {Serial3.IrqHandler();}
void SERCOM1_3_Handler() {Serial3.IrqHandler();}

// initializeSerialCommunication is a constructor that sets the initial
// state of the serial channels.

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

// serialIsAvailable is an accessor function that returns true if there
// is data ready to be read for given serial channel.

boolean serialIsAvailable(int nodeNumber) {

  switch (nodeNumber) {
    case 0: return Serial1.available();
    case 1: return Serial2.available();
    case 2: return Serial3.available();
    case 3: return Serial4.available();
    default: return false;
  }
  
}

// readSerial is an accessor function that returns the next available
// byte for a given serial channel.

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

// readSerialLine is an accessor function that reads bytes from a
// given serial channel up to and including a new line terminator.

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

// writeSerial is a mutator function that write a byte to the 
// given serial channel.

void writeSerial(int nodeNumber, int output) {

  switch (nodeNumber) {
    case 0: Serial1.write(output); break;
    case 1: Serial2.write(output); break;
    case 2: Serial3.write(output); break;
    case 3: Serial4.write(output); break;
  }
  
}

// writeSerialLine is a mutator function that writes a string
// to the given serial channel, then terminates that string with
// a new line.

void writeSerialLine(int nodeNumber, String line) {

  for (int characterPosition = 0; characterPosition < line.length(); characterPosition++)
    writeSerial(nodeNumber, line[characterPosition]);
  writeSerial(nodeNumber, newLine);
 
  delay(writeDelay);

}

// flushSerial is a manager function that clears the buffer of the given
// serial channel.

void flushSerial(int nodeNumber) {

  int output;
  
  while (serialIsAvailable(nodeNumber))
    output = readSerial(nodeNumber);
  
}

// parseIP is a helper function that takes a string and parses the IP
// address found therein. There is no error checking taking place here:
// the function assumes that the given string is well-formed, having
// derived as the response to a requestIP command sent across a serial
// channel to a given node. By well-formed, the function expects the IP
// address to reside between the first set of matched parenthesis in the
// given string.

String parseIP(String line) {

  int leftParenthesis;
  int rightParenthesis;

  leftParenthesis = line.indexOf('(');
  rightParenthesis = line.indexOf(')');
  return line.substring(leftParenthesis + 1, rightParenthesis);
  
}

// parseCPUUtilization is a helper function that takes a string and
// parses the value found therein. There is no error checking taking
// place here: the function assumes that the given string is well-formed,
// having derived as a response to a requestCPUUtilization command
// sent across a serial channel to a given node. By well-formed, the
// function expects a percentage value in the last numeric token in
// the given string.

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

// parseMemoryUtilization is a helper function that takes a string and
// parses the value found therein. There is no error checking taking place
// here: the function assumes that the given string is well-formed, having
// derived as a response to a requestMemoryUtilization command sent across
// a serial channel to a given node. By well-formed, the function expects to
// find two numeric values, the leftmost enumerating the total amount of
// primary memory in a node, and the right most enumerating the total amount of
// available memory in that node. These two values are parsed, and a percentage
// value is calculated.

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
// During the booting process, a node will transmit a long and complex stream
// of bytes across its serial channel, reflecting the progress of that process
// as output generated by a node's operating system. We can infer that a node is
// booted when and only when that output terminates with a line whose last token
// is the string "login:". There is no error-checking taking place here: we
// assume that nodes are all well-behaved and do in fact finish booting. To aid
// in discerning the booted state of a node, we include serialBuffer, a helper
// abstraction used as the data structure that powers a simple state machine.

const int maximumLineLength = SERIAL_BUFFER_SIZE;

// During the booting process, the buffer will progress through several states,
// the end state being when it recognizes the final login token.

enum bufferState {waiting, receivedL, receivedO, receivedG, receivedI, receivedN, loginReceived};

// The serialBuffer itself encompasses a string, an index in the string, and
// the state machine's state.

typedef struct serialBuffer {
  int line[maximumLineLength];
  int index;
  bufferState state;
};

serialBuffer serialBuffers[clusterSize];

// initializeSerialBuffers is a constructor that sets the initial state
// of the serial buffers.

void initializeSerialBuffers() {

  for (int nodeNumber = 0; nodeNumber < clusterSize; nodeNumber++) {
    serialBuffers[nodeNumber].index = 0;
    serialBuffers[nodeNumber].state = waiting;
  };
  
}

// Node GPIO *******************************************************************
// Each node offers two signals to the console, one representing if the node
// has power and another providing a heartbeat.

// Here we define the pin assignments for these signals.

#define node0PowerState 22
#define node1PowerState 24
#define node2PowerState 26
#define node3PowerState 28

#define node0Heartbeat 30
#define node1Heartbeat 32
#define node2Heartbeat 34
#define node3Heartbeat 36

// Here we define a constant that represents that a node has power.

const boolean isPoweredOn = HIGH;

// initializeGPIO is a constructor that sets the initial state of each
// node's signals.

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

// readPowerStatus is an accessor function that returns true if a 
// given node has power.

boolean readPowerStatus(int nodeNumber) {

   switch(nodeNumber) {
     case 0: return digitalRead(node0PowerState);
     case 1: return digitalRead(node1PowerState);
     case 2: return digitalRead(node2PowerState);
     case 3: return digitalRead(node3PowerState);
     default: return !isPoweredOn;
   }
   
}

// readHeartbeat is an accessor function that returns the state of a
// given node's heartbeat.

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
// The console's display is divided into several rows, each of which represents
// a single node. During the lifecycle of a node, its state is displayed here.
// Once a node is logged in, the node's name is displayed along with its IP
// address. Below these labels are two bars, the gray one representing the
// node's CPU utilizatation and the blue one representing the node's primary
// memory utilization. To the right of these labels is a power icon which
// can be touched to power off a node. In the center of each power icon is a
// blinking dot, mirroring the node's heartbeat.

// Here we define the pin assignments used to commmunicate between the Grand
// Central the TFT Touch Shield.

#define TFT_DC 9
#define TFT_CS 10

// Here we define a plethora of constants that represent the properties of the
// display and the design of each node's visualization.

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

// Here we declare an instance of the display.

Adafruit_ILI9341 display = Adafruit_ILI9341(TFT_CS, TFT_DC);

// initializeDisplay is a constuctor that sets the initial state of the
// display.

void initializeDisplay() {

  display.begin();
  display.fillScreen(displayBackgroundColor);
  display.setRotation(3);
  
}

// displayName is a mutator function that displays the name of a given node.
// There is no error checking taking place here: the function assumes that
// the name is well-formed (generally in the form nodeClusterx, where x is
// the id of the node). Note that because this field in the display is used
// both to display the state of a node as it progresses through its lifetime,
// writing a new name involves re-writing the old name in the background
// color (effectively erasing it from view) then writing the new name.

void displayName(int nodeNumber, String name) {

  int nodePosition = nodeNumber * nodeHeight;

  display.fillRect(0, nodePosition, displayWidth - 1, fontHeightFreeSans12pt7b + (nameVerticalPadding * 2) + 3, nameBackgroundColor);
  display.setFont(&FreeSans12pt7b);
  display.setTextColor(nameColor);
  display.setCursor(fontIndentFreeSans12pt7b, nodePosition + fontHeightFreeSans12pt7b + nameVerticalPadding);
  display.println(name);

}

// displayIP is a mutator function that displays the IP address of a given
// node. There is no error checking place here: the function assumes that
// the IP address is well-formed (generally in the form xxx.xxx.x.xx). Note
// that because this field in the display is used both as a placeholder and
// then as a final value as a node progresses though its lifetime, writing
// a new IP address involves re-writing the old IP address in the background
// color (effectively erasing it from view) then writing the new name.

void displayIP(int nodeNumber, String iP) {

  int nodePosition = nodeNumber * nodeHeight + nameHeight;

  display.fillRect(0, nodePosition, displayWidth - 1, fontHeightFreeSansOblique9pt7b + (ipVerticalPadding * 2) + 1, ipBackgroundColor);
  display.setFont(&FreeSansOblique9pt7b);
  display.setTextColor(ipColor);
  display.setCursor(fontIndentFreeSansOblique9pt7b, nodePosition + fontHeightFreeSansOblique9pt7b + ipVerticalPadding);
  display.println(iP);

}

// displayCPUUtilization is a mutator function that displays the CPU
// utilization of a given node as a percentage (with 100% utilization
// filling the width of the display). This function is optimzed to 
// only extend or erase the utilization bar as a difference between
// the old and new values.

void displayCPUUtilization(int nodeNumber, int oldCPUUtilization, int newCPUUtilization) {

  int nodePosition = nodeNumber * nodeHeight + nameHeight + ipHeight;
  
  if (newCPUUtilization >= oldCPUUtilization)
    display.fillRect(0, nodePosition, (((displayWidth - 1) * newCPUUtilization) / 100), utilizationHeight, cpuUtilizationColor); 
  else
    display.fillRect((((displayWidth - 1) * newCPUUtilization) / 100), nodePosition, (((displayWidth - 1) * oldCPUUtilization) / 100), utilizationHeight, utilizationBackgroundColor);

}

// displayMemoryUtilization is a mutator function that displays the
// memory utilization of a given node as a percentage (with 100%
// utilization filling the width of the display). This function is
// optimzed to only extend or erase the utilization bar as a
// difference between the old and new values.

void displayMemoryUtilization(int nodeNumber, int oldMemoryUtilization, int newMemoryUtilization) {

  int nodePosition = nodeNumber * nodeHeight + nameHeight + ipHeight + utilizationHeight;
  
  if (newMemoryUtilization >= oldMemoryUtilization)
    display.fillRect(0, nodePosition, (((displayWidth - 1) * newMemoryUtilization) / 100), utilizationHeight, memoryUtilizationColor); 
  else
    display.fillRect((((displayWidth - 1) * newMemoryUtilization) / 100), nodePosition, (((displayWidth - 1) * oldMemoryUtilization) / 100), utilizationHeight, utilizationBackgroundColor);
  
}

// displayPower is a mutator function that displays an icon
// for each node, the color of which reflects that node's
// powe state.

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

// displayHeartbeat is a mutator function that displays a dot
// that blinks with each change of a node's heartbeat.

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
// The console's touchscreen overlays its display, and is used to permit
// the user to power off individual nodes.

// Here we have a plethora of constants that represent the properties
// of the touchscreen, with regions corresponding to the visualization of
// of each node.

const int touchSensitivity = 40;
const int touchDelay = 250;

const int touchBoxPadding = 10;
const int touchBoxOffset = touchBoxPadding / 2;
const int touchBoxSize = (powerOuterRadius * 2) + touchBoxPadding;

// A touchBox represents a rectangular region on the display that is
// resonsive to a user's touch.

typedef struct touchBox {
  TS_Point upperLeft;
  TS_Point bottomRight;
};

// Here we declare an instance of the touchscreen and its corresponding boxes.

touchBox touchBoxes[clusterSize];

Adafruit_FT6206 touchscreen = Adafruit_FT6206();

// initializeTouchscreen is a constructor that sets the initial state of the
// touchscreen.

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

// poweringOnNodes is the first function in the set up phase of each node,
// serving to visualize to the user that the cluster is awake.

void poweringOnNodes() {
  
  for (int nodeNumber = 0; nodeNumber < clusterSize; nodeNumber++) { 
    displayName(nodeNumber, poweringOnName);
    displayIP(nodeNumber, defaultIP);
    nodes[nodeNumber].name = poweringOn;
    nodes[nodeNumber].state = poweringOn;
  }

  delay(poweringOnDelay);
    
}

// confirmPoweredOnNodes is the second function in the set up phase of
// each node, serving to confirm and then visualize that a node is powered on.
// If it is discovered that a given node is in fact not powered up, it is 
// marked as such and the cluster's set up is allowed to proceed.

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

// bootingNodes is the third function in the set up phase of each node,
// serving to visualize to the user which nodes have power and are
// proceeding to boot.

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

// confirmBootedNodes is the fourth function in the set up phase of each
// node, serving to confirm and then visualize that a node has booted.
// This is perhaps the most complex phase in a node's lifecycle. During
// this time, a node pushes out a long and complex stream of output
// and this function looks for an indicator that the booting process
// has in fact completed. There is no error-checking that takes place:
// if a node fails to boot, the start up process will hang.

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

// setUp is a function that is exectuted when the console is first powered up,
// encompassing two phases: initialization (of the console itself as well as
// each node and its corresponding parts) and the progress of a simple state
// machine (that tracks the collective lifecyle of each node). There is no
// error-checking taking place here: we assume that all nodes are well-behaved,
// and if they are not, set up will hang (with the user able to visualize exactly
// where the process failed by examining the display).

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

// loop is a function that runs continuously after the console has
// successfully completed set up. Here, we check to see if the
// user has touched any node's power buttons, and we listen for
// each node's heartbeat.

void loop(void) {

  checkTouch();

  listenToHeartbeat();

}
