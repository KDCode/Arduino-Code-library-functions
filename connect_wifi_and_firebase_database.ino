/*
  ---> This is an sample code for conencting to wifi and sending data to the database 
  ---> First install the required library for #include <WiFi.h>  and  #include <Firebase_ESP_Client.h>
  ---> Then crearte a database with the help of firebase 
            ---> Create project 
            ---> Create Real-time database
            ---> Authenticate it make sure write down the API Key Database url user email and user password
            ----> Then create a table under database named as led/state in integer type
  ---> Burn this code into the esp32 then you will see in the database led/state changing from 0 to 1 and vice-versa after every 5 s 
  ---> You can also see the state change in the serial monitor 

*/

#include <WiFi.h>
#include <Firebase_ESP_Client.h>

/* 1. Define the WiFi credentials */
#define WIFI_SSID "_your_wifi_name"
#define WIFI_PASSWORD "_your_wifi_password"

/* 2. Define the API Key */
#define API_KEY "_your_API_key"

/* 3. Define the RTDB URL */
#define DATABASE_URL "_your_web_url"

/* 4. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "_your_email_from_which_you_authenticate"
#define USER_PASSWORD "_your_password_from_which_you_authenticate"

// Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long previousMillis = 0;
const long interval = 5000;  // interval to change led state (5 seconds)

void setup()
{
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  // Comment or pass false value when WiFi reconnection will control by your code or third party library e.g. WiFiManager
  Firebase.reconnectNetwork(true);

  // Since v4.4.x, BearSSL engine was used, the SSL buffer need to be set.
  // Large data transmission may require larger RX buffer, otherwise connection issue or data read time out can be occurred.
  fbdo.setBSSLBufferSize(4096 /* Rx buffer size in bytes from 512 - 16384 */, 1024 /* Tx buffer size in bytes from 512 - 16384 */);

  // Limit the size of response payload to be collected in FirebaseData
  fbdo.setResponseSize(2048);

  Firebase.begin(&config, &auth);

  Firebase.setDoubleDigits(5);

  config.timeout.serverResponse = 10 * 1000;
}

void loop()
{
  // Firebase.ready() should be called repeatedly to handle authentication tasks.
  if (Firebase.ready() && (millis() - previousMillis >= interval))
  {
    previousMillis = millis();

    if (WiFi.status() != WL_CONNECTED)
    {
      Serial.println("WiFi not connected, reconnecting...");
      WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
      while (WiFi.status() != WL_CONNECTED)
      {
        Serial.print(".");
        delay(300);
      }
      Serial.println();
      Serial.print("Reconnected with IP: ");
      Serial.println(WiFi.localIP());
      Serial.println();
    }

    int ledState;
    // Read the current state of the LED
    if (Firebase.RTDB.getInt(&fbdo, "/led/state", &ledState))
    {
      // Toggle the LED state
      ledState = !ledState;

      // Update the state in the database
      if (Firebase.RTDB.setInt(&fbdo, "/led/state", ledState))
      {
        Serial.print("LED state updated to: ");
        Serial.println(ledState);
      }
      else
      {
        Serial.print("Error updating LED state: ");
        Serial.println(fbdo.errorReason());
      }
    }
    else
    {
      Serial.print("Error reading LED state: ");
      Serial.println(fbdo.errorReason());
    }
  }
}
