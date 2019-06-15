#include <WebSite.h>
#include <CustomWebServer.h>
#include <WiFiUtils.h>
#include <RGBAnimator.h>

WebSite* SetUpWebSite;
WebSite* LEDSettingsSite;
CustomWebServer* mainWebServer;

const int rgbPins[] = {1, 2, 3};

RGBAnimator animator(rgbPins);

String LED_Controller_Site = "\
<form action\"/\" method=\"post\">\
<input name=\"color\" type=\"color\" />\
<br>\
<input type=\"submit\" class=\"btn btn-primary\" value=\"Anwenden\">\
</form>";

void handleLogInCredentials()
{
  String html = "";

  if (!mainWebServer->getWebServer()->hasArg("ssid"))
  {
    html += "<form action=\"/\" method=\"post\">\n<p>";
    html += "Network:\n";
    String nets[MAX_NETS_LIST];
    int netsCount = MAX_NETS_LIST;
    WiFiUtils::getWiFiNetworks(nets, netsCount);
    WiFiUtils::appendNamedSelection(html, nets, netsCount, "ssid", true);
    html += "<input type=\"password\" name=\"pw\" placeholder=\"Passwort (optional)\">\n";
    html += "<input type=\"submit\" value=\"Connect\">\n";
    html += "<p></form>";
    html += "<a class=\"btn btn-primary\" href=\"/cleanUp\">Reset chip</a>\n";
    html += "<a class=\"btn btn-primary\" href=\"/noNetwork\">Weiter ohne Netzwerk integration</a>\n";
  }
  else
  {
    String ssid = mainWebServer->getWebServer()->arg("ssid");
    String pw = "";
    if (mainWebServer->getWebServer()->hasArg("pw") && mainWebServer->getWebServer()->arg("pw") != NULL)
      pw = mainWebServer->getWebServer()->arg("pw");

    mainWebServer->launchServerInNetwork(ssid, pw, false);
    html += "<h2 style=\"font-size:8vw;\">Try to connect with network: "+ssid+"</h2>\n";
  }

  if (mainWebServer->hasValidCredentials())
  {
    String ssid = "";
    String pw = "";
    if (mainWebServer->getWLANCredentialsFromEEPROM(ssid, pw)) {
      html += "<p>Saved credentials found for SSID: "+ssid+"</p>\n";
    }
    else {
      html += "<p>Saved credentials found for SSID: "+ssid+" with errors!</p>\n";
    }
  }
  else{
    html += "<p>No credentials used upto now!</p>";
  }

  mainWebServer->showPage(html, "WiFi Setup");
}

void handleCleanUpRequest()
{
  mainWebServer->cleanUpEEPROM();
  mainWebServer->showPage("<h1 style=\"font-size:10vw;\">Complete reset successful!</h1>", "Reset chip");
  mainWebServer->launchServerInNetwork(String(""), String(""), false);
  delay(1000);
  mainWebServer->doRedirect("/");
}

void handleLaunchServerFromAP()
{
  mainWebServer->showPage("<h1 style=\"font-size:10vw;\">Server will not be integrated to network!</h1>", "No embedd");
  delay(1000);
  mainWebServer->launchServerInNetwork(String(""), String(""), true);
  mainWebServer->doRedirect("/");
}

void hex2RGB(String hex, int& r, int& g, int& b)
{
  // Get rid of '#' and convert it to integer
  int number = (int) strtol( &hex[1], NULL, 16);

  r = number >> 16;
  g = number >> 8 & 0xFF;
  b = number & 0xFF;
}

void handleLEDSettings()
{
  if (mainWebServer->getWebServer()->hasArg("color"))
  {
    Serial.println("color: " + mainWebServer->getWebServer()->arg("color"));
    int r, g, b;
    hex2RGB(mainWebServer->getWebServer()->arg("color"), r, g, b);
    animator.setColor(r, g, b);
  }
  mainWebServer->showPage(LED_Controller_Site, "LED Settings");
}

void setup(void)
{
  Serial.begin(9600);
  
  SetUpWebSite = new WebSite();
  SetUpWebSite->addSubSite("/", handleLogInCredentials);
  SetUpWebSite->addSubSite("/cleanUp", handleCleanUpRequest);
  SetUpWebSite->addSubSite("/noNetwork", handleLaunchServerFromAP);

  LEDSettingsSite = new WebSite();
  LEDSettingsSite->addSubSite("/", handleLEDSettings);
  SetUpWebSite->addSubSite("/cleanUp", handleCleanUpRequest);
  mainWebServer = new CustomWebServer(SetUpWebSite, LEDSettingsSite);
}

void loop(void)
{
  mainWebServer->handleClient();
  animator.update();
  delay(25);
}
