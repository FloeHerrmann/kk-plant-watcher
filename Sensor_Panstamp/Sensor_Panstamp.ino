#include "HardwareSerial.h"
#include <Wire.h>
#include <Adafruit_HTU21DF.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2561_U.h>

//#define SIMULATION
#define SERIAL_OUTPUT

#define RFCHANNEL 0

#define SYNCWORD1 0x48
#define SYNCWORD0 0x48

#define SOURCE_ADDR 4
#define DESTINATION_ADDR 1
#define PACKET_SIZE 20

#define HTU21DF_POWER_PIN 14
#define TSL2561_POWER_PIN 15
#define MOISTURE_POWER_PIN 16

Adafruit_HTU21DF htu = Adafruit_HTU21DF();
Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345);

byte deviceSerial[8];

void setup() {
	#ifdef SERIAL_OUTPUT
		Serial.begin( 57600 );
		Serial.println( "Plant Sensor" );
		Serial.println( "-----------------------------" );
	#endif

	// Setup LED output pin
	pinMode(LED, OUTPUT);
	digitalWrite( LED , LOW );

	pinMode( HTU21DF_POWER_PIN , OUTPUT );
	digitalWrite( HTU21DF_POWER_PIN , LOW );

	pinMode( TSL2561_POWER_PIN , OUTPUT );
	digitalWrite( TSL2561_POWER_PIN , LOW );

	pinMode( MOISTURE_POWER_PIN , OUTPUT );
	digitalWrite( MOISTURE_POWER_PIN , LOW );

	#ifndef SIMULATION
		#ifdef SERIAL_OUTPUT
			Serial.println( "Initialize HTU21DF" );
		#endif

		digitalWrite( HTU21DF_POWER_PIN , HIGH ); delay( 20 );
		if( !htu.begin() ) {
			Serial.println( "Couldn't find humidity sensor!" );
			while( 1 ) {
				digitalWrite( LED , HIGH );
				delay( 1000 );
				digitalWrite( LED , LOW );
				delay( 1000 );
			}
		}
		digitalWrite( HTU21DF_POWER_PIN , LOW ); delay( 20 );

		#ifdef SERIAL_OUTPUT
			Serial.println( "Initialize TSL2561" );
		#endif
		digitalWrite( TSL2561_POWER_PIN , HIGH ); delay( 20 );
		if( !tsl.begin() ){
			Serial.print( "Ooops, no TSL2561 detected ... Check your wiring or I2C ADDR!" );
			while( 1 ) {
				digitalWrite( LED , HIGH );
				delay( 1000 );
				digitalWrite( LED , LOW );
				delay( 1000 );
			}
		}
		digitalWrite( TSL2561_POWER_PIN , LOW ); delay( 20 );

		tsl.enableAutoRange(true);

		// tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_13MS);
		// tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_101MS);
		tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_402MS);
	#endif

	panstamp.core.getUID(deviceSerial);

	panstamp.radio.setChannel(RFCHANNEL);
	panstamp.radio.setSyncWord(SYNCWORD1, SYNCWORD0);
	panstamp.radio.setDevAddress(SOURCE_ADDR);
	panstamp.radio.setCCregs();
}

void loop() {

	#ifdef SIMULATION
		uint16_t moisture = analogRead( A1 );
		int currentTempI = random( 1800 , 2500 );
		int currentHumI = random( 4500 , 7000 );
		long currentLightI = random( 180000 , 300000 );
		int voltageSupplyI = panstamp.getVcc();
	#else
		analogReference( DEFAULT );
		delay( 500 );
		uint16_t moisture = analogRead( A1 );
		if( moisture < 0 ) moisture = 0;
		else if( moisture == 65535 ) moisture = 0;
		else if( moisture > 4095 ) moisture = 4095;
		
		float currentTemp = htu.readTemperature();
		int currentTempI = currentTemp * 100.0;

		float currentHum = htu.readHumidity();
		int currentHumI = currentHum * 100.0;

		sensors_event_t event;
		tsl.getEvent( &event );
		float currentLight = event.light;
		long currentLightI = currentLight * 100.0;

		int voltageSupplyI = panstamp.getVcc();
	#endif

	#ifdef SERIAL_OUTPUT
		Serial.print( "Moist. " );
		Serial.print( moisture );
		Serial.print( "\t\t" );
		Serial.print( "Tempe. " );
		Serial.print( currentTempI );
		Serial.print( "\t\t" );
		Serial.print( "Humid. " );
		Serial.print( currentHumI );
		Serial.print( "\t\t" );
		Serial.print( "Light " );
		Serial.print( currentLightI );
		Serial.print( "\t\t" );
	#endif

	CCPACKET txPacket;
	txPacket.length = PACKET_SIZE;

	txPacket.data[0] = DESTINATION_ADDR;
	txPacket.data[1] = deviceSerial[0];
	txPacket.data[2] = deviceSerial[1];
	txPacket.data[3] = deviceSerial[2];
	txPacket.data[4] = deviceSerial[3];
	txPacket.data[5] = deviceSerial[4];
	txPacket.data[6] = deviceSerial[5];
	txPacket.data[7] = deviceSerial[6];
	txPacket.data[8] = deviceSerial[7];

	txPacket.data[9] = ( voltageSupplyI >> 8 ) & 0b11111111;
	txPacket.data[10] = voltageSupplyI & 0b11111111;

	txPacket.data[11] = ( moisture >> 8 ) & 0b11111111;
	txPacket.data[12] = moisture & 0b11111111;

	txPacket.data[13] = ( currentTempI >> 8 ) & 0b11111111;
	txPacket.data[14] = currentTempI & 0b11111111;
	
	txPacket.data[15] = ( currentHumI >> 8 ) & 0b11111111;
	txPacket.data[16] = currentHumI & 0b11111111;

	txPacket.data[17] = ( currentLightI >> 16 ) & 0b11111111;
	txPacket.data[18] = ( currentLightI >> 8 ) & 0b11111111;
	txPacket.data[19] = currentLightI & 0b11111111;

	#ifdef SERIAL_OUTPUT
		char tmp[16];
		for( int i = 0 ; i < PACKET_SIZE ; i++ ) {
			sprintf( tmp , "%.2X" , txPacket.data[i] ); 
         	Serial.print( tmp );
         }
		Serial.println( "" );
	#endif

	digitalWrite( LED , HIGH );
	panstamp.radio.sendData(txPacket);
	digitalWrite( LED , LOW );

	panstamp.sleepSec( 60 );

	// For low-power applications replace "delay" by "panstamp.sleepWd(WDTO_8S)" for example
}

