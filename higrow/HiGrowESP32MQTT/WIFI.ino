void connectWiFi()
{
  int count = 0;
  Serial.print("Connecting to ");
  Serial.println(mySSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(mySSID, myPW);

  String mac = WiFi.macAddress();
  Serial.println(mac);
  
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
    count++;
    if (count > 30) ESP.restart();
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}


