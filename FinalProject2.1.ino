// pulse sensor
#define USE_ARDUINO_INTERRUPTS false
#include <PulseSensorPlayground.h>
const int OUTPUT_TYPE = SERIAL_PLOTTER;
const int PIN_INPUT = A0;
const int PIN_BLINK = 13;    // Pin 13 is the on-board LED
const int PIN_FADE = 5;
const int THRESHOLD = 550;   // Adjust this number to avoid noise when idle
byte samplesUntilReport;
const byte SAMPLES_PER_SERIAL_SAMPLE = 10;
PulseSensorPlayground pulseSensor;

// wifi stuff 
#define USE_ARDUINO_INTERRUPTS false
#include <PulseSensorPlayground.h>
#include <SPI.h>
//#include <SoftwareSerial.h>
#include <WiFi101.h>
char ssid[] = "hp-setup-1"; //  your network SSID (name) 
char pass[] = "WelcomeHal";    // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;            // your network key Index number (needed only for WEP)
int status = WL_IDLE_STATUS;
//thingspeak server
char server[] = "api.thingspeak.com";
WiFiClient client;
//API key for the Thingspeak ThingHTTP
const String apiKey = "BLQ2QWQ7ISECDND4";
//the number the message should be sent to
const String sendNumber = "3603568287";

static boolean alive= true;
static int x =0;
static int LED = 3;
static int LEDon = 12;
static int buzzer = 2;
static int button = 8;
static int onOffButton = 9; 
static boolean on = false;
static unsigned long prevTime = 0;
static boolean firstTime = true;
static boolean firstTimeOn = true;
static boolean firstText = false;
unsigned long startTime;

// for age input
static unsigned int age;
static boolean firstTimeAge = true;
static boolean firstTimeAgeAsk = true;
static boolean gettingAge = false;
static boolean confirmingAge1 = false;
static boolean confirmingAge = false;
static unsigned int maxHeartRate = 200;
static unsigned int minHeartRate = 40;
static int age0 = 4;
static int age30 = 6;

// for reading resting BPM 
static boolean firstTimeBPM = false;
static boolean firstTimeInstructions = false;
static boolean readingPulse = false;
static int count = 0;
static int BPMTotal = 0; 
static boolean printBPM = false;
static int restingBPM;

void setup() {
  Serial.begin(9600);
  // wifi setup stuff 

  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present"); 
    // don't continue:
    while(true);
  } 
  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) { 
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:    
    status = WiFi.begin(ssid, pass);
    Serial.println(status); 
    // wait 10 seconds for connection:
    delay(10000);
  } 
  Serial.println("Connected to wifi");
  //printWifiStatus();

  // pin for turning on LED
  pinMode(LED, OUTPUT); 
  // pin for turning on buzzer
  pinMode(buzzer, OUTPUT);
  // pin for reading state of button
  pinMode(button, INPUT);
  // pin for reading state of buttonDemo 
  pinMode(onOffButton, INPUT);
  // pin for temperature sensor
  pinMode(A3, INPUT); 
  // pin for on light 
  pinMode(12, OUTPUT);

  // lcd stuff 
  //lcd.setMCPType(LTI_TYPE_MCP23008);
  //lcd.begin(16, 2);
  //lcd.setBacklight(HIGH);
  
}

void loop() {
  //lcd.setCursor(0, 1);
  
  // on off button 
  if (digitalRead(onOffButton) == 1 && on == false) {
    Serial.println("TURNING ON");
   // lcd.clear();
    //lcd.print("TURNING ON");
    digitalWrite(LED, LOW);
    digitalWrite(buzzer, LOW);
    digitalWrite(LEDon, HIGH);
    delay(2000);
    firstTimeOn = true;
    alive = true;
    on = true;
    firstTimeAge = true;
    firstTimeAgeAsk = true;
  }
  if (digitalRead(onOffButton) == 1 && on == true) {
    on = false;
    Serial.println("TURNING OFF");
    //lcd.clear();
    //lcd.print("TURNING OFF");
    digitalWrite(LED, LOW);
    digitalWrite(buzzer, LOW);
    digitalWrite(LEDon, LOW);
    delay(2000);
  }

  if (on) {
  // calculating heart rate range 
  
  while(firstTimeAge) {
    if (firstTimeAgeAsk) {
      Serial.println("Press button of your age range");
      //lcd.clear();
      //lcd.print("Press button of your age range");
      firstTimeAgeAsk = false;
    }
   // 0 to 39
    if (digitalRead(age0) == 1){
      firstTimeAge = false;
      Serial.println("Your age range is 0 to 39");
      //lcd.clear();
      //lcd.print("Your age range is 0 to 39");
      age = 25;
      delay(2000);
    }

    // 40+
    if (digitalRead(age30) == 1){
      firstTimeAge = false;
      Serial.println("Your age range is 40+");
     // lcd.clear();
      //lcd.print("Your age range is 40+");
      age = 60;
      delay(2000);
    }
    firstTimeBPM = true;
    startTime = millis();
              /*
    while(firstTimeAge) {
    if (firstTimeAgeAsk) {
      Serial.println("What is your age?");
      firstTimeAgeAsk = false;
      gettingAge = true;
    }

    if (gettingAge) {
      if (Serial.available() > 0) {
        age = Serial.parseInt();
        Serial.print("Your age is ");
        Serial.print(age);
        Serial.println(".");
        confirmingAge1 = true; 
        gettingAge = false;    
      }
    }

    if (confirmingAge1) {
      Serial.println("Is this correct? 1 for yes, 0 for no");
      confirmingAge1 = false;
      confirmingAge = true; 
    }

    if (confirmingAge) {
      
      if (Serial.available() > 0) {
        int yesNo = Serial.parseInt();
        if (yesNo == 0) {
          firstTimeAge = true;
          firstTimeAgeAsk = true;
        } 
        if (yesNo == 1) {
          Serial.println("OK");
          firstTimeAge = false;
        }
        confirmingAge = false;
        firstTimeInstructions = true;
        firstTimeBPM = true;
      }
    }
    startTime = millis();
    // */
  }
  maxHeartRate = 220 - age;
  

  
  // pulse sensor stuff
  if (pulseSensor.sawNewSample()) {
    if (--samplesUntilReport == (byte) 0) {
      samplesUntilReport = SAMPLES_PER_SERIAL_SAMPLE;
      // pulseSensor.outputSample(); 
      /*
         At about the beginning of every heartbeat,
         report the heart rate and inter-beat-interval.
      */
      if (pulseSensor.sawStartOfBeat()) {
        pulseSensor.outputBeat();
      }
    }
    
    // add code here 
    unsigned long currentTime = millis();
    if (pulseSensor.getBeatsPerMinute()- x < 0) {
      Serial.print("BPM: ");
      Serial.println(0);
    } else{
      Serial.print("BPM: ");
      Serial.println(pulseSensor.getBeatsPerMinute()- x);
    }
     // lcd.clear();
      //lcd.print("BPM: ");
      //lcd.print(pulseSensor.getBeatsPerMinute());
      
      if ((pulseSensor.getBeatsPerMinute()- x < minHeartRate || pulseSensor.getBeatsPerMinute()- x > maxHeartRate) && !(firstTimeBPM && currentTime - startTime < 30000)) {
        firstTimeBPM = false;
        alive = false;
        if (firstTimeOn) {
          prevTime = currentTime;
          firstText = true;
        }
        firstTimeOn = false;
      }

      // if senses irregular heartrate
      if (!alive) {
        // turns LED and buzzer on
        digitalWrite(LED, HIGH);
        digitalWrite(buzzer, HIGH);
    
        // gives user 10 seconds to press button if false alarm
        // read if button is pressed or not 
        if (digitalRead(button) == 1 && currentTime - prevTime < 10000 && currentTime > 500) {                                                                                              
          // turns LED and buzzer off bc false alarm
          digitalWrite(LED, LOW);
          digitalWrite(buzzer, LOW);
          // gives time for sensor to stabilize if false alarm
          delay(10000);
          firstTimeOn = true;
          alive = true;
        }
        if (currentTime - prevTime >= 10000 && firstText) {
          // LED and buzzer stay on, no need for code
          Serial.println("sending text");
         // lcd.clear();
         // lcd.print("sending text");
          sendSMS(sendNumber, URLEncode("Heart Alert has detected something wrong with the user"));
          firstText = false;
        }
      }

      // reading temperature and adjust heart rate 
      if (analogRead(A3) > 0) {
        float volt = analogRead(A3);
        float temp = (volt / 1024) * 500; 
        if (temp > 80) {
          maxHeartRate += 10;
        }
      }  
  }
  }
}


void sendSMS(String number,String message)
{
  // Make a TCP connection to remote host
  if (client.connect(server, 80))
  {

    //should look like this...
    //api.thingspeak.com/apps/thinghttp/send_request?api_key={api key}&number={send to number}&message={text body}

    client.print("GET /apps/thinghttp/send_request?api_key=");
    client.print(apiKey);
    client.print("&number=");
    client.print(number);
    client.print("&message=");
    client.print(message);
    client.println(" HTTP/1.1");
    client.print("Host: ");
    client.println(server);
    client.println("Connection: close");
    client.println();
  }
  else
  {
    Serial.println(F("Connection failed"));
  } 

  // Check for a response from the server, and route it
  // out the serial port.
  while (client.connected())
  {
    if ( client.available() )
    {
      char c = client.read();
      Serial.print(c);
    }      
  }
  Serial.println();
  client.stop();
}

String URLEncode(const char* msg)
{
  const char *hex = "0123456789abcdef";
  String encodedMsg = "";

  while (*msg!='\0'){
    if( ('a' <= *msg && *msg <= 'z')
      || ('A' <= *msg && *msg <= 'Z')
      || ('0' <= *msg && *msg <= '9') ) {
      encodedMsg += *msg;
    } 
    else {
      encodedMsg += '%';
      encodedMsg += hex[*msg >> 4];
      encodedMsg += hex[*msg & 15];
    }
    msg++;
  }
  return encodedMsg;
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
