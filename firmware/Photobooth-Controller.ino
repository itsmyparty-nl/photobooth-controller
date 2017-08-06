// *** PhotoboothController ***
// Control the photobooth by supplying buttons which can be armed and released,
// which send commands to the application via a serial interface and the
// CmdMessenger library
// Based on the ArduinoController example

#include <CmdMessenger.h>  // CmdMessenger
#include <SoftTimer.h>
#include <Heartbeat.h>
#include <BlinkTask.h>
#include <SoftPwmTask.h>
#include <Dimmer.h>

// status heartbeat led variables 
const int ledPin            = 13;  // Pin of internal Led
bool ledState               = 1;   // Current state of Led

bool triggerBtnPushed = false;
bool printBtnPushed = false;
bool printTwiceBtnPushed = false;
bool powerBtnPushed = false;

#define INPUT_PIN_1 2
#define INPUT_PIN_2 4
#define INPUT_PIN_3 7
#define INPUT_PIN_4 8

/* In this demonstration a pulsating is implemented. */
#define OUT_PIN_1  3
#define OUT_PIN_2  5
#define OUT_PIN_3  6
#define OUT_PIN_4  9

#define triggerBtnLed  OUT_PIN_1
#define powerBtnLed  OUT_PIN_4
#define printBtnLed  OUT_PIN_2
#define printTwiceBtnLed  OUT_PIN_3

#define triggerBtnPin  INPUT_PIN_1
#define powerBtnPin  INPUT_PIN_4
#define printBtnPin  INPUT_PIN_2
#define printTwiceBtnPin  INPUT_PIN_3

Task buttonTaskTrigger(110, buttonReadTrigger);
Task buttonTaskPrint(80, buttonReadPrint);
Task buttonTaskPrintTwice(120, buttonReadPrintTwice);
Task buttonTaskPower(90, buttonReadPower);
Task commandHandlerTask(500, readSerial);

Heartbeat triggerLedTask(triggerBtnLed);
BlinkTask printLedTask(printBtnLed,1000);
BlinkTask printTwiceLedTask(printTwiceBtnLed,1000);
BlinkTask powerLedTask(powerBtnLed,1000);

//SoftPwmTask triggerLockedLedTask(triggerBtnLed);
//SoftPwmTask printLockedLedTask(printBtnLed);
//SoftPwmTask printTwiceLockedLedTask(printTwiceBtnLed);
//SoftPwmTask powerLockedLedTask(powerBtnLed);
//Dimmer triggerLockedDimmer(&triggerLockedLedTask, 1500);
//Dimmer printLockedDimmer(&printLockedLedTask, 1500);
//Dimmer printTwiceLockedDimmer(&printTwiceLockedLedTask, 1500);
//Dimmer powerLockedDimmer(&powerLockedLedTask, 1500);

// Attach a new CmdMessenger object to the default Serial port
CmdMessenger cmdMessenger = CmdMessenger(Serial);

// This is the list of recognized commands. These kcan be commands that can either be sent or received. 
// In order to receive, attach a callback function to these events
enum
{
  kAcknowledge,
  kError,
  kTrigger,
  kPrint,
  kPower,
  kPrepareControl,
  kReleaseControl,
  kPrintTwice,
  kLockControl,
  kUnlockControl,
  kInitialize
};

// Callbacks define on which received commands we take action
void attachCommandCallbacks()
{
  // Attach callback methods
  cmdMessenger.attach(OnUnknownCommand);
  cmdMessenger.attach(kPrepareControl, OnPrepareControl);
  cmdMessenger.attach(kReleaseControl, OnReleaseControl);
  cmdMessenger.attach(kLockControl, OnLockControl);
  cmdMessenger.attach(kUnlockControl, OnLockControl);
  cmdMessenger.attach(kInitialize, Initialize);
}

// Called when a received command has no attached function
void OnUnknownCommand()
{
  cmdMessenger.sendCmd(kError,"Command without attached callback");
}

// Initialize all controls to a ready state
void Initialize()
{
  // Remove all tasks, since the UDOO arduino has persistent state
  SoftTimer.remove(&buttonTaskPrintTwice);
  SoftTimer.remove(&printTwiceLedTask);
  SoftTimer.remove(&buttonTaskPrint);
  SoftTimer.remove(&printLedTask);
  SoftTimer.remove(&buttonTaskTrigger);
  SoftTimer.remove(&triggerLedTask);
  SoftTimer.remove(&buttonTaskPower);
  
  // Also stop all dimmers
  //triggerLockedDimmer.off();
  //powerLockedDimmer.off();
  //printLockedDimmer.off();
  //printTwiceLockedDimmer.off();
 
  //Set all pins low to check connections
  digitalWrite(OUT_PIN_1, LOW);
  digitalWrite(OUT_PIN_2, LOW);
  digitalWrite(OUT_PIN_3, LOW);
  digitalWrite(OUT_PIN_4, LOW);
}

// Callback function that sets led on or off
void OnLockControl()
{
  int control;
  // Read led state argument, interpret string as int
  control = cmdMessenger.readIntArg();
  switch (control)
  {
    case kTrigger:
      digitalWrite(triggerBtnLed, LOW);
      //triggerLockedDimmer.startPulsate();
      digitalWrite(triggerBtnLed, HIGH);
      break;
    case kPower:
      digitalWrite(powerBtnLed, LOW);
      //powerLockedDimmer.startPulsate();
      digitalWrite(powerBtnLed, HIGH); 
      break;
    case kPrint:
      digitalWrite(printBtnLed, LOW);
      //printLockedDimmer.startPulsate();
      digitalWrite(printBtnLed, HIGH);
      break;
    case kPrintTwice:
      digitalWrite(printTwiceBtnLed, LOW);
      //printTwiceLockedDimmer.startPulsate();
      digitalWrite(printTwiceBtnLed, HIGH);
      break;
    default:
      cmdMessenger.sendCmd(kError,"Unsupported button");
  }

  cmdMessenger.sendCmd(kAcknowledge,control);
}

void OnUnlockControl()
{
  int control;
  // Read led state argument, interpret string as int
  control = cmdMessenger.readIntArg();
  switch (control)
  {
    case kTrigger:
      //triggerLockedDimmer.off();
      digitalWrite(triggerBtnLed, LOW);
      break;
    case kPower:
      //powerLockedDimmer.off();
      digitalWrite(powerBtnLed, LOW);
      break;
    case kPrint:
      //printLockedDimmer.off();
      digitalWrite(printBtnLed, LOW);
      break;
    case kPrintTwice:
      //printTwiceLockedDimmer.off();
      digitalWrite(printTwiceBtnLed, LOW);
      break;
    default:
      cmdMessenger.sendCmd(kError,"Unsupported button");
  }

  cmdMessenger.sendCmd(kAcknowledge,control);
}

// Callback function that sets led on or off
void OnPrepareControl()
{
  int control;
  // Read led state argument, interpret string as int
  control = cmdMessenger.readIntArg();
  switch (control)
  {
    case kTrigger:
      SoftTimer.add(&buttonTaskTrigger);
      SoftTimer.add(&triggerLedTask);
      break;
    case kPower:
      SoftTimer.add(&buttonTaskPower);
      digitalWrite(powerBtnLed, HIGH);
      break;
    case kPrint:
      SoftTimer.add(&buttonTaskPrint);
      SoftTimer.add(&printLedTask);
      break;
    case kPrintTwice:
      SoftTimer.add(&buttonTaskPrintTwice);
      SoftTimer.add(&printTwiceLedTask);
      break;
    default:
      cmdMessenger.sendCmd(kError,"Unsupported button");
  }

  cmdMessenger.sendCmd(kAcknowledge,control);
}

// Callback function that sets led on or off
void OnReleaseControl()
{
  int control;
  // Read led state argument, interpret string as string
  control = cmdMessenger.readIntArg();
  switch (control)
  {
    case kTrigger:
      SoftTimer.remove(&buttonTaskTrigger);
      SoftTimer.remove(&triggerLedTask);
      digitalWrite(triggerBtnLed, LOW);
      break;
    case kPower:
      SoftTimer.remove(&buttonTaskPower);
      digitalWrite(powerBtnLed, LOW);
      break;
    case kPrint:
      SoftTimer.remove(&buttonTaskPrint);
      SoftTimer.remove(&printLedTask);
      digitalWrite(printBtnLed, LOW);
      break;
    case kPrintTwice:
      SoftTimer.remove(&buttonTaskPrintTwice);
      SoftTimer.remove(&printTwiceLedTask);
      digitalWrite(printTwiceBtnLed, LOW);
      break;
    default:
      cmdMessenger.sendCmd(kError,"Unsupported button");
  }
  cmdMessenger.sendCmd(kAcknowledge,control);
}

// Setup function
void setup() 
{
  //Set all pins to their correct mode
  pinMode(INPUT_PIN_1, INPUT);
  pinMode(INPUT_PIN_2, INPUT);
  pinMode(INPUT_PIN_3, INPUT);
  pinMode(INPUT_PIN_4, INPUT);
  pinMode(OUT_PIN_1, OUTPUT);
  pinMode(OUT_PIN_2, OUTPUT);
  pinMode(OUT_PIN_3, OUTPUT);
  pinMode(OUT_PIN_4, OUTPUT);
  
  //Set all controls to their default state
  Initialize();  
  
  //Set all pins high to check connections
  digitalWrite(OUT_PIN_1, HIGH);
  digitalWrite(OUT_PIN_2, HIGH);
  digitalWrite(OUT_PIN_3, HIGH);
  digitalWrite(OUT_PIN_4, HIGH);
  
  // Listen on serial connection for messages from the PC
  Serial.begin(115200); 
  // Wait until the serial connection is present
  //while (!Serial) ;
  // Adds newline to every command
  //cmdMessenger.printLfCr();   

  // Attach my application's user-defined callback methods
  attachCommandCallbacks();

  // Send the status to the PC that says the Arduino has booted
  // Note that this is a good debug function: it will let you also know 
  // if your program had a bug and the arduino restarted  
  cmdMessenger.sendCmd(kAcknowledge,"Arduino has started!");

  // set pin for blink LED
  pinMode(ledPin, OUTPUT);
  
  SoftTimer.add(&commandHandlerTask);
}

// Loop function
void readSerial(Task* me) 
{
  // Process incoming serial data, and perform callbacks
  cmdMessenger.feedinSerialData();
  // Show a heartbeat to know that command processing is not frozen
  ledState = ledState ? LOW :HIGH;
  digitalWrite(ledPin, ledState);
}

void buttonReadTrigger(Task* me)
{
  readButton(triggerBtnPin,kTrigger, &triggerBtnPushed);
}

void buttonReadPrint(Task* me)
{
  readButton(printBtnPin,kPrint, &printBtnPushed);
}

void buttonReadPrintTwice(Task* me)
{
  readButton(printTwiceBtnPin,kPrintTwice, &printTwiceBtnPushed);
}

void buttonReadPower(Task* me)
{
  readButton(powerBtnPin,kPower, &powerBtnPushed);
}

void readButton(int inputPin, int buttonCommand, bool* wasPushed)
{
  // read the state of the pushbutton value:
  int buttonState = digitalRead(inputPin);
  
  // check if the pushbutton is pressed.
  // if it is, the buttonState is HIGH:
  if (buttonState == HIGH) {
    if ( *wasPushed == false)
    {
     cmdMessenger.sendCmd(buttonCommand, 0);
     
    *wasPushed = true; 
    }
  } 
  else {
    if (*wasPushed == true)
    {
      *wasPushed = false;
    } 
  }
}
