#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <FirebaseArduino.h>
#include <EEPROM.h>

#define FIREBASE_HOST "<your host key>"
#define FIREBASE_AUTH "<your auth key>"
#define wifi_connect WiFi.status() == WL_CONNECTED


String ssid, pass;

char* dssid = "test";
char* dpass = "123456789";

boolean hotspot = false, firebase = false, hotclient = false;

ESP8266WebServer server(80);

String st, content;

int n;

int statusCode;

void setup()
{
  Serial.begin(9600);

  EEPROM.begin(512);

  readSsid();
  readPass();

  if (ssid.length() > 1)
    connectToWifi();
  else
  {
    error();
    manual_hotspot();
    createWebPage();
  }
}
void loop()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    //Serial.println("loop/wificonnected");
    if (Serial.available())
    {
      toggling();
    }
    else if (Firebase.getInt("change") == 1)
    {
      Serial.print(1);
      delay(10);
      if (Serial.available())
        toggling();

      Firebase.remove("change");
    }
  }
  else if (!(WiFi.status() == WL_CONNECTED) && hotspot == false)
  {
    //Serial.println("loop/notconnected&&hotspot==false");
    connectToWifi();
    if (!(WiFi.status() == WL_CONNECTED))
    {
      error();
      manual_hotspot();
      createWebPage();
    }
  }
  else if (hotspot == true)
  {
    //Serial.println("loop/hotspot==trure");
    delay(50);
    server.begin();
    while (1)
    {
      //Serial.println("loop/hotspot==true/hotclient==true");
      server.handleClient();
    }
    connectToWifi();
  }
}


void readSsid()
{
  //Serial.println("reading ssid from memory");
  for (int i = 0; i < 32; i++)
    ssid += char(EEPROM.read(i));
  //Serial.println(">>>>>>");
  //Serial.print(ssid);
  //Serial.print("<<<<<<");
  //Serial.println();
}
void readPass()
{
  //Serial.println("reading password from memory");
  for (int i = 32; i < 96; i++)
    pass += char(EEPROM.read(i));
  //Serial.println(">>>>>>");
  //Serial.print(pass);
  //Serial.print("<<<<<<");
  //Serial.println();
}
void writeSsid()
{
  //Serial.println("writting ssid to memory");  
  //Serial.println();
  for ( int i = 0; i < ssid.length(); i++)
  {
    EEPROM.write(i, ssid[i]);
    //Serial.print(ssid[i]);
  }
  EEPROM.commit();
}
void writePass()
{
  //Serial.println("writting password to memory");
  //Serial.println();
  for ( int i = 32; (i - 32) < pass.length(); i++)
  {
    EEPROM.write(i, pass[i - 32]);
    //Serial.print(pass[i - 32]);
  }
  EEPROM.commit();
}
void connectToWifi()
{
  //Serial.println("in connect to wifi block");
  WiFi.mode(WIFI_STA);
  hotspot = false;
  //Serial.println(">>>>>>>>");
  //Serial.print(ssid.c_str());
  //Serial.println(">>>>>>>>");

  //Serial.println(">>>>>>>>");
  //Serial.print(pass.c_str());
  //Serial.println(">>>>>>>>");
  const char *s = ssid.c_str(), *p = pass.c_str();
  WiFi.begin(s, p);
  delay(5000);
  if (!(WiFi.status() == WL_CONNECTED))
    delay(3000);
  if (WiFi.status() == WL_CONNECTED)
  {
    //Serial.print("trying to connect to fireabse");
    Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
    firebase = true;
  }
}
void error()
{
  //Serial.println("in error block");
  st = "<ol>";
  if (!(WiFi.status() == WL_CONNECTED))
  {
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    n = WiFi.scanNetworks();
    if (n == 0)
      st += "no network found";
    else
    {
      for (int i = 0; i < n; i++)
      {
        st += "<li>";
        st += WiFi.SSID(i);
        st += " (";
        st += WiFi.RSSI(i);
        st += ")";
        st += (WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*";
        st += "</li>";
      }
    }
  }
  st += "</ol>";
}
void manual_hotspot()
{
  //Serial.println("in manual hotspot block");
  WiFi.softAP(dssid, dpass);
  Serial.println(WiFi.softAPIP());
  hotspot = true;
}
void createWebPage()
{
  //Serial.println("creating web page");
  server.on("/", []() {
    IPAddress ip = WiFi.softAPIP();
    String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
    content = "<!DOCTYPE HTML>\r\n<html>Hello from ESP8266 at ";
    content += ipStr;
    content += "<p>";
    content += st;
    content += "</p><form method='get' action='setting'><label>SSID: </label><input name='ssid' length=32><input name='pass' length=64><input type='submit'></form>";
    content += "</html>";
    server.send(200, "text/html", content);
  });
  server.on("/setting", []() {
    ssid = server.arg("ssid");
    pass = server.arg("pass");
    //Serial.println("Ssid read from webpage is");
    //Serial.print(ssid);
    //Serial.println("Password read from web is");
    //Serial.print(pass);
    if (ssid.length() > 0 && pass.length() > 0)
    {

      for (int i = 0; i < 96; i++) EEPROM.write(i, 0);

      writeSsid();
      writePass();

      content = "{\"Success\":\"saved to eeprom... \"}";
      statusCode = 200;
      hotclient = false;
      delay(1000);
      ESP.restart();
    }
    else
    {
      content = "{\"Error\":\"404 not found\"}";
      statusCode = 404;
      hotclient = true;
    }
    server.send(statusCode, "application/json", content);
  });
}
void toggling()
{
  //Serial.println("toggling block");
  int val = Serial.read() - '0';
  if (val == 1)
  {
    Firebase.setString("switch", "true");
    if (Firebase.failed())
    {
      Firebase.setString("switch", "true");
      if (Firebase.failed())
      {
        if (!(WiFi.status() == WL_CONNECTED))
        {
          error();
          manual_hotspot();
          createWebPage();
          return;
        }
      }
    }
  }
  else if (val == 0)
  {
    Firebase.setString("switch", "false");
    if (Firebase.failed())
    {
      Firebase.setString("switch", "true");
      if (Firebase.failed())
      {
        if (!(WiFi.status() == WL_CONNECTED))
        {
          error();
          manual_hotspot();
          createWebPage();
          return;
        }
      }
    }
  }
}

