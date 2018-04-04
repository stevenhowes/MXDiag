#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>


#include "jquery.h"
#include "index.h"
#include "svg.h"
#include "javascript.h"
#include "manifest.h"
#include "icon.h"

#include "dtc.h"

// Current state and previous state of the pin
int oldpin = 1;
int currentpin = 1;

// timenow of current state change, and previous
unsigned long timenow;
unsigned long statestart;

unsigned int diagcount = 0;
int firstpulsedone = 0;

int serialin = 0;

// Current diagnostic stage
int diagstate = -1;

String diagstring = "";

String dtcoutput = "";
String statoutput = "Booting...";

const char *ssid = "MX5Diag";
const char *password = "password";

int waitmilis = 0;

ESP8266WebServer server(80);

void handle_jq_js() {
 server.sendHeader("Content-Encoding","gzip");
 server.send_P(200, "text/javascript", jquery, sizeof(jquery));
}

void handle_mx_js() {
 server.sendHeader("Content-Encoding","gzip");
 server.send_P(200, "text/javascript", mxjs, sizeof(mxjs));
}

void handle_manifest_json() {
 server.sendHeader("Content-Encoding","gzip");
 server.send_P(200, "text/json", manifestjson, sizeof(manifestjson));
}

void handle_icon_png() {
 server.sendHeader("Content-Encoding","gzip");
 server.send_P(200, "image/png", iconpng, sizeof(iconpng));
}

void handle_index() {
 server.sendHeader("Content-Encoding","gzip");
 server.send_P(200, "text/html", indexhtml, sizeof(indexhtml));
}

void handle_go_svg() {
 server.sendHeader("Content-Encoding","gzip");
 server.send_P(200, "image/svg+xml", gosvg, sizeof(gosvg));
}

void handle_mz_svg() {
 server.sendHeader("Content-Encoding","gzip");
 server.send_P(200, "image/svg+xml", mzsvg, sizeof(mzsvg));
}


void handle_dtc() {
  server.send(200, "text/plain", dtcoutput);
}

void handle_stat() {
  server.send(200, "text/plain", statoutput);
}

void handle_run() {
  server.send(200, "text/plain", "");
  begindiag();
}

String identifydtc(String dtc)
{
  int found = -1;
  for(unsigned int i=0; i<(sizeof(dtc_index)/sizeof(dtc_index[0]));i++)
  {
    if(dtc_index[i] == dtc.toInt())
    {
      found = i;    
    }
  }

  if(found == -1)
    return "Unknown";
  else
    return dtc_data[found];
}

void physicalsetup()
{
  pinMode(D1, INPUT);             // Signal from ECU
  pinMode(D3, OUTPUT);             // Diagnosis enable bin
  pinMode(LED_BUILTIN, OUTPUT);   // Status light

  // Don't turn pull-down transistor on yet
  digitalWrite(D3, LOW);

  // Set up serial
  Serial.begin(115200);
  Serial.println();
}

void wifisetup()
{
  WiFi.softAP(ssid/*, password*/);
  
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  // Files served - all pre-gzipped to save flash
  server.on("/jq.js", handle_jq_js);
  server.on("/", handle_index);
  server.on("/go.svg", handle_go_svg);
  server.on("/mz.svg", handle_mz_svg);
  server.on("/mx.js", handle_mx_js);

  // Make Android web app work nicely
  server.on("/manifest.json", handle_manifest_json);
  server.on("/icon.png", handle_icon_png);

  // Dynamic 'files'
  server.on("/dtc", handle_dtc);
  server.on("/stat", handle_stat);
  server.on("/run", handle_run);

  server.begin();
}

void setup()
{
  physicalsetup();
  wifisetup();
  statoutput = "MXDiag Online";
  Serial.println(statoutput);
}

void begindiag()
{
  statoutput = "Turn off ignition";
  Serial.println(statoutput);

  waitmilis = millis() + 6000;
}

void diag()
{
  // Read the pin once here
  currentpin = digitalRead(D1);

  if (diagstate == 0) // Default state on boot, waiting for pulse from ECU
  {
    // Give a little flash each cycle to show we're OK
    digitalWrite(LED_BUILTIN, LOW);
    delay(5);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(200);

    // If the pin drops
    if (currentpin == 0)
    {
      // Record when it happens
      statestart = millis();

      // Move us to the next state
      diagstate = 1;

      // Mirror the ECU output
      digitalWrite(LED_BUILTIN, LOW);
    }
  }
  else if (diagstate == 1) // This state is during the ECU greeting pulse
  {
    // If the pin comes back up, the pulse is over
    if (currentpin == 1)
    {
      // Mirror ECU ouptut
      digitalWrite(LED_BUILTIN, HIGH);

      // Record 'now'
      timenow = millis();

      // Default to returning to the wait (may not be needed)
      diagstate = 0;

      // Calculate the length of the pulse
      int pulselength = (timenow - statestart);

      // If it's too short or too long
      if ((pulselength < 1000) || (pulselength > 5000))
      {
        // Go back to wait state
        diagstate = 0;
      } else {
        // If it's our 3000ms goldilocks pulse, move to the next stage
        statoutput = "ECU Detected";
        Serial.println(statoutput);
        diagstate = 2;
      }
    }
  }
  else if (diagstate == 2) // Waiting for a DTC pule
  {
    // If pin goes low we're starting on a pulse
    if (currentpin == 0)
    {
      // Mirror to LED
      digitalWrite(LED_BUILTIN, LOW);

      // Record when it happens and calculate the length of the silence beforehand
      timenow = millis();
      int pulselength = (timenow - statestart);

      // If it's a sensible length
      if (pulselength > 100)
      {
        // If it's gurt then probably next DTC
        if (pulselength > 3500)
        {
          if (firstpulsedone == 1)
          {
            // Divider for easy debug
            diagstring = diagstring + diagcount;

            diagcount = 0;
          } else {
            firstpulsedone = 1;
            Serial.println("STATUS: Begining DTC read");
          }
        }
        if (pulselength > 7500)
        {
          // Divider for easy debug
          Serial.println(diagstring);
          dtcoutput = dtcoutput + "P" + diagstring + ":" + identifydtc(diagstring) + "<br/>";

          diagstring = "";
        }
      }

      // Record when state change occured
      statestart = millis();

      // Move to 3 (waiting for pulse end)
      diagstate = 3;
    }
  }
  else if (diagstate == 3) // Waits for pulse end
  {
    if (currentpin == 1)
    {
      // Mirror to LED
      digitalWrite(LED_BUILTIN, HIGH);

      // Record when it happened
      timenow = millis();

      // Default to waiting for next pulse
      diagstate = 2;

      // Calculate pulse length
      int pulselength = (timenow - statestart);

      // If it's too short or too long
      if ((pulselength < 300) || (pulselength > 1400))
      {
        // Wait for another one
        diagstate = 2;
      } else {
        // 0.4 is short, 1.2 is long so lets split the difference
        if (pulselength > 800)
        {
          // On late Mk1 / Mk2 all codes are 4 digit so we ignore the longs
        }
        else
        {
          diagcount++;
        }

        // Record for silence calculation
        statestart = millis();
      }
    }
  }
  else
  {
    // An error state that stops all comms and shouts lots
    statoutput = "Diagnostic failure";
    Serial.println(statoutput);
  }
}

void loop()
{
  server.handleClient();

  // Look for instructions
  if (Serial.available() > 0)
  {
    serialin = Serial.read();

    // D for DTC
    if (serialin == 68)
      begindiag();

    // F for fan (5 seconds)
    //if (serialin == 70)
    //  fancycle();// TODO
    
    // P for fuel pump (5 seconds)
    //if (serialin == 80)
    //  fuelprime();// TODO
  }

  int left = (waitmilis - millis());

  if( left > 0)
  {
      left = left / 1000;
      statoutput = "Turn off ignition: " + String(left);
      Serial.println(statoutput);
  }
  else
  {
    if(waitmilis != 0)
    {
        // Turn on diag pin
        digitalWrite(D3, HIGH);

        statoutput = "Turn on ignition (waiting for ECU)";
        Serial.println(statoutput);

        // Put us in default 'ready to go' mode
        diagstate = 0;
        waitmilis = 0;
    }
  }

  if(waitmilis == 0)
    if(diagstate >= 0)
      diag();

}
