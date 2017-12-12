/**
      This test illustrates the use if yield methods and internal StatusRequest objects
      THIS TEST HAS BEEN TESTED ON NODEMCU V.2 (ESP8266)

      The WiFi initialization and NTP update is executed in parallel to blinking the onboard LED
      and an external LED connected to D2 (GPIO04)
      Try running with and without correct WiFi parameters to observe the difference in behaviour
*/

#define _TASK_SLEEP_ON_IDLE_RUN
#define _TASK_STATUS_REQUEST
#include <TaskScheduler.h>

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

Scheduler ts;

// Callback methods prototypes
void connectInit();
void ledCallback();
bool ledOnEnable();
void ledOnDisable();
void ledOn();
void ledOff();
void ntpUpdateInit();

// Tasks

Task  tConnect    (TASK_SECOND, TASK_FOREVER, &connectInit, &ts, true);
Task  tLED        (TASK_IMMEDIATE, TASK_FOREVER, &ledCallback, &ts, false, &ledOnEnable, &ledOnDisable);

// Tasks running on events
Task  tNtpUpdate  (&ntpUpdateInit, &ts);

// Replace with WiFi parameters of your Access Point/Router:
const char *ssid  =  "wifi_network";
const char *pwd   =  "wifi_password";

long  ledDelayOn, ledDelayOff;

#define LEDPIN            D0      // Onboard LED pin - linked to WiFi
#define LEDPIN2           D2      // External LED
#define CONNECT_TIMEOUT   30      // Seconds
#define CONNECT_OK        0       // Status of successful connection to WiFi
#define CONNECT_FAILED    (-99)   // Status of failed connection to WiFi

// NTP Related Definitions
#define NTP_PACKET_SIZE  48       // NTP time stamp is in the first 48 bytes of the message

IPAddress     timeServerIP;       // time.nist.gov NTP server address
const char*   ntpServerName = "time.nist.gov";
byte          packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets
unsigned long epoch;

WiFiUDP udp;                      // A UDP instance to let us send and receive packets over UDP

#define LOCAL_NTP_PORT  2390      // Local UDP port for NTP update



void setup() {
  Serial.begin(74880);
  Serial.println(F("TaskScheduler test #14 - Yield and internal StatusRequests"));
  Serial.println(F("=========================================================="));
  Serial.println();

  pinMode (LEDPIN, OUTPUT);
  pinMode (LEDPIN2, OUTPUT);

  tNtpUpdate.waitFor( tConnect.getInternalStatusRequest() );  // NTP Task will start only after connection is made
}

void loop() {
  ts.execute();                   // Only Scheduler should be executed in the loop
}

/**
   Initiate connection to the WiFi network
*/
void connectInit() {
  Serial.print(millis());
  Serial.println(F(": connectInit."));
  Serial.println(F("WiFi parameters: "));
  Serial.print(F("SSID: ")); Serial.println(ssid);
  Serial.print(F("PWD : ")); Serial.println(pwd);

  WiFi.mode(WIFI_STA);
  WiFi.hostname("esp8266");
  WiFi.begin(ssid, pwd);
  yield();

  ledDelayOn = TASK_SECOND / 2;
  ledDelayOff = TASK_SECOND / 4;
  tLED.enable();

  tConnect.yield(&connectCheck);            // This will pass control back to Scheduler and then continue with connection checking
}

/**
   Periodically check if connected to WiFi
   Re-request connection every 5 seconds
   Stop trying after a timeout
*/
void connectCheck() {
  Serial.print(millis());
  Serial.println(F(": connectCheck."));

  if (WiFi.status() == WL_CONNECTED) {                // Connection established
    Serial.print(millis());
    Serial.print(F(": Connected to AP. Local ip: "));
    Serial.println(WiFi.localIP());
    tConnect.disable();
  }
  else {

    if (tConnect.getRunCounter() % 5 == 0) {          // re-request connection every 5 seconds

      Serial.print(millis());
      Serial.println(F(": Re-requesting connection to AP..."));

      WiFi.disconnect(true);
      yield();                                        // This is an esp8266 standard yield to allow linux wifi stack run
      WiFi.hostname("esp8266");
      WiFi.mode(WIFI_STA);
      WiFi.begin(ssid, pwd);
      yield();                                        // This is an esp8266 standard yield to allow linux wifi stack run
    }

    if (tConnect.getRunCounter() == CONNECT_TIMEOUT) {  // Connection Timeout
      tConnect.getInternalStatusRequest()->signal(CONNECT_FAILED);  // Signal unsuccessful completion
      tConnect.disable();

      Serial.print(millis());
      Serial.println(F(": connectOnDisable."));
      Serial.print(millis());
      Serial.println(F(": Unable to connect to WiFi."));

      ledDelayOn = TASK_SECOND / 16;                  // Blink LEDs quickly due to error
      ledDelayOff = TASK_SECOND / 16;
      tLED.enable();
    }
  }
}

/**
   Initiate NTP update if connection was established
*/
void ntpUpdateInit() {
  Serial.print(millis());
  Serial.println(F(": ntpUpdateInit."));

  if ( tConnect.getInternalStatusRequest()->getStatus() != CONNECT_OK ) {  // Check status of the Connect Task
    Serial.print(millis());
    Serial.println(F(": cannot update NTP - not connected."));
    return;
  }

  udp.begin(LOCAL_NTP_PORT);
  if ( WiFi.hostByName(ntpServerName, timeServerIP) ) { //get a random server from the pool

    Serial.print(millis());
    Serial.print(F(": timeServerIP = "));
    Serial.println(timeServerIP);

    sendNTPpacket(timeServerIP); // send an NTP packet to a time server
  }
  else {
    Serial.print(millis());
    Serial.println(F(": NTP server address lookup failed."));
    tLED.disable();
    udp.stop();
    tNtpUpdate.disable();
    return;
  }

  ledDelayOn = TASK_SECOND / 8;
  ledDelayOff = TASK_SECOND / 8;
  tLED.enable();

  tNtpUpdate.set( TASK_SECOND, CONNECT_TIMEOUT, &ntpCheck );
  tNtpUpdate.enableDelayed();
}

/**
 * Check if NTP packet was received
 * Re-request every 5 seconds
 * Stop trying after a timeout
 */
void ntpCheck() {
  Serial.print(millis());
  Serial.println(F(": ntpCheck."));

  if ( tNtpUpdate.getRunCounter() % 5 == 0) {

    Serial.print(millis());
    Serial.println(F(": Re-requesting NTP update..."));

    udp.stop();
    yield();
    udp.begin(LOCAL_NTP_PORT);
    sendNTPpacket(timeServerIP);
    return;
  }

  if ( doNtpUpdateCheck()) {
    Serial.print(millis());
    Serial.println(F(": NTP Update successful"));

    Serial.print(millis());
    Serial.print(F(": Unix time = "));
    Serial.println(epoch);

    tLED.disable();
    tNtpUpdate.disable();
    udp.stop();
  }
  else {
    if ( tNtpUpdate.isLastIteration() ) {
      Serial.print(millis());
      Serial.println(F(": NTP Update failed"));
      tLED.disable();
      udp.stop();
    }
  }
}

/**
 * Send NTP packet to NTP server
 */
unsigned long sendNTPpacket(IPAddress & address)
{
  Serial.print(millis());
  Serial.println(F(": sendNTPpacket."));

  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  udp.beginPacket(address, 123); //NTP requests are to port 123
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
  yield();
}

/**
 * Check if a packet was recieved.
 * Process NTP information if yes
 */
bool doNtpUpdateCheck() {

  Serial.print(millis());
  Serial.println(F(": doNtpUpdateCheck."));

  yield();
  int cb = udp.parsePacket();
  if (cb) {
    // We've received a packet, read the data from it
    udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

    //the timestamp starts at byte 40 of the received packet and is four bytes,
    // or two words, long. First, esxtract the two words:

    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    unsigned long secsSince1900 = highWord << 16 | lowWord;

    // now convert NTP time into everyday time:
    // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
    const unsigned long seventyYears = 2208988800UL;
    // subtract seventy years:
    epoch = secsSince1900 - seventyYears;
    return (epoch != 0);
  }
  return false;
}

/**
 * Flip the LED state based on the current state
 */
bool ledState;
void ledCallback() {
  if ( ledState ) ledOff();
  else ledOn();
}

/**
 * Make sure the LED starts lit
 */
bool ledOnEnable() {
  ledOn();
  return true;
}

/**
 * Make sure LED ends dimmed
 */
void ledOnDisable() {
  ledOff();
}

/**
 * Turn LEDs on. 
 * Set appropriate delay.
 * PLEASE NOTE: NodeMCU onbaord LED is active-low
 */
void ledOn() {
  ledState = true;
  digitalWrite(LEDPIN, LOW);
  digitalWrite(LEDPIN2, HIGH);
  tLED.delay( ledDelayOn );
}

/**
 * Turn LEDs off. 
 * Set appropriate delay.
 * PLEASE NOTE: NodeMCU onbaord LED is active-low
 */
void ledOff() {
  ledState = false;
  digitalWrite(LEDPIN, HIGH);
  digitalWrite(LEDPIN2, LOW);
  tLED.delay( ledDelayOff );
}

