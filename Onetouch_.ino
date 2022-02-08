#include <WiFi.h>
#include <WebServer.h>
#include <Arduino.h>
#include <HTML.h>
#include <EEPROM.h>
#include <Firebase_ESP_Client.h>




//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

 

// Insert Firebase project API Key
#define API_KEY  "AIzaSyDwVmSMxz4AKAQX5WE5-iMfqskYsMY3QIg"

// Insert RTDB URLefine the RTDB URL 
#define DATABASE_URL  "https://rilonetouch-default-rtdb.firebaseio.com/" 



//Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;

FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
bool signupOK = false;
String getip;
String modevalue;
String ledstate;
String ledstate2;
String cont;
float floatValue;
int ledpin1 = 27;
int ledpin2 = 25;
int buttonState;
int buttonpin = 0;
String devicestate;
String devicestate2;

WiFiServer server(80);
     
          
     const char * EEP ;
     const char * EEP2;
     


void setup() {
  Serial.begin(115200);
  pinMode(ledpin1 , OUTPUT);
  pinMode(ledpin2 , OUTPUT);
   pinMode(buttonpin,INPUT); //for resetting WiFi creds
   
  
   
     
   //wifi credential setup
  EEPROM.begin(400);

  restart:
  if(!CheckWIFICreds()){
    Serial.println("No WIFI credentials stored in memory. Loading form...");
    while(loadWIFICredsForm());
  }
     else {
      
       String  a= EEPROM.readString(100);
       String b = EEPROM.readString(200);
  
          EEP = strcpy(new char[a.length()+ 1],a.c_str());
              EEP2 = strcpy(new char[b.length()+ 1],b.c_str());
              
                    Serial.print("ssid:");
                    Serial.print(EEP);
                    Serial.print("\npassword:");
                    Serial.print(EEP2);
                  
        WiFi.begin(EEP,EEP2);
  Serial.print("\nif wifi not available you can change your credentials using the boot button:");
       Serial.print("\nConnecting to Wi-Fi:");
        Serial.print(EEP);
        while (WiFi.status() != WL_CONNECTED) {
          buttonState = digitalRead(buttonpin);
               if(buttonState == LOW ){
                         Serial.print("\nWiping any Stored credentials froim database:");
                         for(int i=0;i<400;i++){
                              EEPROM.writeByte(i,0);
                                       }
                                 EEPROM.commit();
                   goto restart;
         }
          delay(10);
          
        }
          Serial.println();
          Serial.print("Connected with IP: ");
        Serial.println(WiFi.localIP());
        Serial.println();
        server.begin();
    }

  // Assign the api key (required) 
   config.api_key = API_KEY;

  // Assign the RTDB URL (required) 
  config.database_url = DATABASE_URL;

  // Sign up 
   
        
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("ok"); 
    signupOK = true;
  }
  else {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());

  }

  // Assign the callback function for the long running token generation task 
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  
}

void internet(){
 
  
 
    if (Firebase.RTDB.getString(&fbdo, "/test/ledstate1")) {
      if (fbdo.dataType() == "string") {   //specifies the data type of the value str holds which is an int in this case
        ledstate = fbdo.stringData();
        char * str;
        
        String str_obj(ledstate);
        str = &str_obj[1];   //using a char pointer to hold value stored in the second address of the variable led state
      
         Serial.println("\nBulb Pin recieved: ");
        Serial.println(*str);
      
         if(*str == '1'){
           Serial.println("Bulb is on");
                    digitalWrite(ledpin1, HIGH);
              }
              else if(*str == '0'){
                 Serial.println("Bulb is off");
                  digitalWrite(ledpin1, LOW);
                   }
      }
      
    }
    
    if (Firebase.RTDB.getString(&fbdo, "/test/ledstate2")) {
      if (fbdo.dataType() == "string") {   //specifies the data type of the value str holds which is an int in this case
        ledstate2 = fbdo.stringData();
        char * strr;
        
        String str_obj2(ledstate2);
        strr = &str_obj2[1];   //using a char pointer to hold value stored in the second address of the variable led state
      
         Serial.println("\nSocket pin recieved: ");
        Serial.println(*strr);
      
         if(*strr == '2'){
           Serial.println("led is on");
                    digitalWrite(ledpin2, HIGH);
              }
              else if(*strr == '3'){
                 Serial.println("led is off");
                  digitalWrite(ledpin2, LOW);
                   }
      }
    }

}
void wlancontrol(){
  buttonState = digitalRead(buttonpin);
  WiFiClient client = server.available();   // listen for incoming clients

  if (client) {                             // if you get a client,
    Serial.println("New Client.");           // print a message out the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();            
        Serial.write(c);                    
        if (c == '\n') {                   

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();

            // the content of the HTTP response follows the header:
            client.print("Click <a href=\"/H\">here</a> to turn the LED on pin 27 on.<br>");
            client.print("Click <a href=\"/L\">here</a> to turn the LED on pin 27 off.<br>");

            // The HTTP response ends with another blank line:
            client.println();
            // break out of the while loop:
            break;
          } else {    // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      
        }

        // Check to see if the client request was "GET /1" or "GET /0":
        if (currentLine.endsWith("GET /1")) {
          digitalWrite(ledpin1, HIGH);               // GET /1 turns the LED on
        }
        if (currentLine.endsWith("GET /0")) {
          digitalWrite(ledpin1, LOW);                // GET /0 turns the LED off
        }
         if (currentLine.endsWith("GET /H")) {
          digitalWrite(ledpin2, HIGH);               // GET /H turns the LED on
        }
        if (currentLine.endsWith("GET /L")) {
          digitalWrite(ledpin2, LOW);                // GET /L turns the LED off
        }
        if(buttonState == LOW  || currentLine.endsWith("GET /S") ){
                    saveconfig();
         }
      }
    }
    // close the connection:
    client.stop();
    Serial.println("Client Disconnected.");
  }
}

void getips(){
   
  
   if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0)){
    sendDataPrevMillis = millis();
    // Write an ip address on the database path test/link
    getip = WiFi.localIP().toString();
    if (Firebase.RTDB.setString(&fbdo, "test/link", getip)){
     
    }
  }
}
void sendstate(){
                 devicestate = "state_on";
                       devicestate2 = "state_off";
    if (Firebase.ready() && signupOK ){
    if(digitalRead(ledpin1) == HIGH){
           if (Firebase.RTDB.setString(&fbdo, "test/devicestate", devicestate)){
       }
    }
    else if(digitalRead(ledpin1) == LOW){
              if (Firebase.RTDB.setString(&fbdo, "test/devicestate", devicestate2)){
       }
    }
     else if(digitalRead(ledpin2) == HIGH){
              if (Firebase.RTDB.setString(&fbdo, "test/devicestate1", devicestate)){
       }
     }
        else if(digitalRead(ledpin2) == LOW){
              if (Firebase.RTDB.setString(&fbdo, "test/devicestate1", devicestate2)){
       }
     }
    } 
}
void wipeEEPROM(){
  for(int i=0;i<400;i++){
    EEPROM.writeByte(i,0);
  }
  EEPROM.commit();
}

void saveconfig(){
   
    Serial.println("Wiping WiFi credentials from memory...");
    while(loadWIFICredsForm());
}

boolean rebootinternet(){
   WiFiClient client = server.available();   // listen for incoming clients

  if (client) {                             // if you get a client,
    Serial.println("New Client.");           // print a message out the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();            
        Serial.write(c);                    
        if (c == '\n') {                   

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();

            // the content of the HTTP response follows the header:
            client.print("Click <a href=\"/H\">here</a> to turn the LED on pin 27 on.<br>");
            client.print("Click <a href=\"/L\">here</a> to turn the LED on pin 27 off.<br>");

            // The HTTP response ends with another blank line:
            client.println();
            // break out of the while loop:
            break;
          } else {    // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      
        }

        
        if (currentLine.endsWith("GET /5")) {
          return true;             
        }
        else{
          return false;
        }
      }
    }
  }
}


void loop() {
  buttonState = digitalRead(buttonpin);
    
   if (Firebase.ready() && signupOK  ) {
    if (Firebase.RTDB.getString(&fbdo, "/test/control")) {
      if (fbdo.dataType() == "string") {   //specifies the data type of the value str holds which is an int in this case
        cont = fbdo.stringData();
        
        char * stre;
        
        String str_obj (cont);
        stre = &str_obj[1];  
       
        if(*stre == '0'){
            wlancontrol();
            getips();
            sendstate();
               }
         else if(*stre == '1'){
            internet();
           sendstate();
        }
        else if(*stre == '2' || buttonState == LOW ){
          Serial.print("\nWiping any Stored credentials froim database:");
            wipeEEPROM();
            saveconfig();
           
        }
         else if(rebootinternet()){
        }

         
         }
         
    }
   }

    else {
      Serial.println(fbdo.errorReason());
    }
    
}
