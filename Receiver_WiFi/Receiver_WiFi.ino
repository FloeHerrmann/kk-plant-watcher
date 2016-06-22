#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <SoftwareSerial.h>

#define GREEN_LED D1
#define RED_LED D2

ESP8266WiFiMulti WiFiMulti;

SoftwareSerial SoftSerial(D3, D4, false, 256);

char currentChar = '0';
char previousChar = '0';
String receivedStringTemp = "";
String receivedString = "";

void sendData( String deviceId , String data ) {
	//if( (WiFiMulti.run() == WL_CONNECTED)) {
	if( WiFi.status() == WL_CONNECTED ){
		Serial.println( "HTTP POST Request" );

		HTTPClient http;

		http.begin("http://www.pro-kitchen.net/portal/kk/plantData.do");
		//http.begin("http://192.168.2.102:8080/portal/kk/plantData.do");

		int httpCode = http.POST( "Device=" + deviceId + "&Data=" + data );

		if(httpCode > 0) {
			digitalWrite( GREEN_LED , LOW );
			delay( 100 );
			digitalWrite( GREEN_LED , HIGH );
			Serial.printf("HTTP Status Code: %d\n", httpCode);

			if(httpCode == HTTP_CODE_OK) {
				String payload = http.getString();
				Serial.printf( "HTTP Response > %s\n" , payload.c_str() );
			}
		} else {
			digitalWrite( GREEN_LED , LOW );
			digitalWrite( RED_LED , HIGH );
			Serial.printf("HTTP POST Failed > %s\n", http.errorToString(httpCode).c_str());
		}

		delay( 100 );
		digitalWrite( GREEN_LED , LOW );
		digitalWrite( RED_LED , LOW );

		http.end();
	}
}

void parseReceivedData( String receivedData ) {

	digitalWrite( GREEN_LED , HIGH );
	digitalWrite( RED_LED , HIGH );
	delay( 100 );
	digitalWrite( GREEN_LED , LOW );
	digitalWrite( RED_LED , LOW );

	Serial.print( "Serial Received Data > " );
	Serial.println( receivedData );

	Serial.print( "Serial Parsed Data > " );
	Serial.print( receivedData.substring( 0 , 6 ) );
	Serial.print( " / " );
	Serial.print( receivedData.substring( 6 , 24 ) );
	Serial.print( " / " );
	Serial.println( receivedData.substring( 24, receivedData.length() + 1 ) );

	digitalWrite( GREEN_LED , HIGH );
	sendData( receivedData.substring( 6 , 24 ) , receivedData.substring( 24, receivedData.length() + 1 ) );
}

void setup() {

	Serial.begin(57600);
	SoftSerial.begin(57600);

	pinMode( GREEN_LED , OUTPUT );
	pinMode( RED_LED , OUTPUT );

	digitalWrite( GREEN_LED , HIGH );
	digitalWrite( RED_LED , HIGH );
	delay( 2000 );
	digitalWrite( GREEN_LED , LOW );
	digitalWrite( RED_LED , LOW );
	delay( 1000 );

	//WiFi.begin( "Florians iPhone" , "x115kt7d89mi" );
	//WiFi.begin( "WIFI-24GHZ" , "nDD7xZgAkXKLhYFfUxmsqtUoVLkYbHxVW7yVyHTDLciWAjCauL" );
	//WiFi.begin( "m2msystems" , "88DBF22F5F3C3381CB8D779F3D93C2E4ACEAD192FCE2F63C2AA82374F17B26F" );
	WiFi.begin( "Bellchen" , "xrmp1257" );

	//WiFiMulti.clearAP();
	//WiFiMulti.addAP("Florians_iPhone", "x115kt7d89mi");

	Serial.println( "\nConnect to WiFi" );
	while( WiFi.status() != WL_CONNECTED ) {
		digitalWrite( GREEN_LED , HIGH );
		digitalWrite( RED_LED , HIGH );
		delay(500);
		Serial.print(".");
		digitalWrite( GREEN_LED , LOW );
		digitalWrite( RED_LED , LOW );
		delay(500);
	}
	Serial.println( "DONE" );

	digitalWrite( RED_LED , LOW );

	digitalWrite( GREEN_LED , HIGH ); delay( 250 );
	digitalWrite( GREEN_LED , LOW ); delay( 250 );
	digitalWrite( GREEN_LED , HIGH ); delay( 250 );
	digitalWrite( GREEN_LED , LOW ); delay( 250 );
	digitalWrite( GREEN_LED , HIGH ); delay( 250 );
	digitalWrite( GREEN_LED , LOW ); delay( 250 );
	digitalWrite( GREEN_LED , HIGH ); delay( 250 );
	digitalWrite( GREEN_LED , LOW ); delay( 250 );


}

void loop() {

	while (SoftSerial.available() > 0) {
		char read = SoftSerial.read();

		if( 13 != (int)read && 10 != (int)read ) {
			receivedStringTemp += read;
		}

		previousChar = currentChar;
		currentChar = read;
	
		if( 13 == (int)previousChar && 10 == (int)currentChar ) {
			receivedString = receivedStringTemp;
			receivedStringTemp = "";
			parseReceivedData( receivedString );
		}
	}

}