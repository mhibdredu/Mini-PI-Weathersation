#include <stdio.h>
#include <MQTTClient.h>
#include <time.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <wiringPi.h>

#define mqtt_client_ID  "BzoWBBcWLh0fMiErMQYWGBw"
#define mqtt_username   "BzoWBBcWLh0fMiErMQYWGBw"
#define mqtt_password   "AwCZGYgOEW1z6cxbfSj1YZWr"
#define mqtt_host       "mqtt3.thingspeak.com"
#define PAYLOAD "field1=%d.%d&field2=%d.%d&field3=%.2f"
#define O_RDONLY         00
#define MAXTIMINGS	85
#define DHTPIN		7

MQTTClient client;
char topic[1024] = "channels/1744947/publish";
char buffer[1024];

int dht11_dat[5] = { 0, 0, 0, 0, 0 };
int rc;


 
void read_dht11_dat()
{
	uint8_t laststate	= HIGH;
	uint8_t counter		= 0;
	uint8_t j		= 0, i;
	float	f; 
 
	dht11_dat[0] = dht11_dat[1] = dht11_dat[2] = dht11_dat[3] = dht11_dat[4] = 0;
 
	pinMode( DHTPIN, OUTPUT );
	digitalWrite( DHTPIN, LOW );
	delay( 18 );
	digitalWrite( DHTPIN, HIGH );
	delayMicroseconds( 40 );
	pinMode( DHTPIN, INPUT );
 
	for ( i = 0; i < MAXTIMINGS; i++ )
	{
		counter = 0;
		while ( digitalRead( DHTPIN ) == laststate )
		{
			counter++;
			delayMicroseconds( 1 );
			if ( counter == 255 )
			{
				break;
			}
		}
		laststate = digitalRead( DHTPIN );
 
		if ( counter == 255 )
			break;
 
		if ( (i >= 4) && (i % 2 == 0) )
		{
			dht11_dat[j / 8] <<= 1;
			if ( counter > 50 )
				dht11_dat[j / 8] |= 1;
			j++;
		}
	}
 
	if ( (j >= 40) &&
	     (dht11_dat[4] == ( (dht11_dat[0] + dht11_dat[1] + dht11_dat[2] + dht11_dat[3]) & 0xFF) ) )
	{
		//f = dht11_dat[2] * 9. / 5. + 32;
			printf( "Feuchtigkeit = %d.%d %% Temperatur = %d.%d C \n\n",
			dht11_dat[0], dht11_dat[1], dht11_dat[2], dht11_dat[3] );
	}else  {
		printf( "Daten nicht gut, nochmal lesen\n" );
		//read_dht11_dat();
	
	}
}

float GetCPULoad() {
	int FileHandler;
	char FileBuffer[1024];
	float load;

	FileHandler = open("/proc/loadavg", O_RDONLY);
	if(FileHandler < 0) {
		return -1; }
	read(FileHandler, FileBuffer, sizeof(FileBuffer) - 1);
	sscanf(FileBuffer, "%f", &load);
	close(FileHandler);
	printf("CPU-Last: %.2f\n\n",load*10);
	return (float)(load * 10);
}

void StartConnection(){
    //MQTTClient client;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;

    conn_opts.keepAliveInterval = 10;
    conn_opts.cleansession = 1;
    conn_opts.username = mqtt_username;
    conn_opts.password = mqtt_password;

    //rc = MQTTClient_create(&client, mqtt_host, mqtt_client_ID,MQTTCLIENT_PERSISTENCE_NONE, NULL);
    if ((rc = MQTTClient_create(&client, mqtt_host, mqtt_client_ID,MQTTCLIENT_PERSISTENCE_NONE, NULL)) != MQTTCLIENT_SUCCESS)
    {
         printf("Client konnte nicht erstellt werden, return code %d\n", rc);
         exit(EXIT_FAILURE);
    } else {
        printf("Client wurde erfolgreich erstellt\n\n");
    }

    //rc = MQTTClient_connect(client, &conn_opts);
    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
    {
        printf("Verbindung nicht möglich, return code %d\n", rc);
        exit(EXIT_FAILURE);
    } else {
        printf("Verbindung hergestellt\n\n");
    }

}

void publishMSG(float cpuload , int tempbf , int humbf , int tempaf , int humaf){

    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    MQTTClient_deliveryToken token;

    snprintf(buffer , sizeof(buffer) , PAYLOAD  , tempbf , tempaf, humbf , humaf , cpuload);

    pubmsg.payload = buffer;
    pubmsg.payloadlen = (int)strlen(buffer);

    //rc = MQTTClient_publishMessage(client, topic, &pubmsg, &token);
    if ((rc = MQTTClient_publishMessage(client, topic, &pubmsg, &token)) != MQTTCLIENT_SUCCESS)
    {
         printf("Nachricht konnte nicht veröffentlicht werden, return code %d\n", rc);
         exit(EXIT_FAILURE);
    } else {
        printf("Nachricht wurde erfolgreich veröffentlicht\n\n");
    }

}

int main(){
    int  tempbf , humbf, tempaf , humaf;
    float cpuload;
    printf("INF208 ES. Erstellt von: 160501309 und 70501040 .Starten des Programms\n\n");
    //printf("MQTTCLIENT_SUCCESS %d             \n\n" , MQTTCLIENT_SUCCESS);
    StartConnection();
    if ( wiringPiSetup() == -1 )
		exit( 1 );
    
    printf("-----------------------------------------------------------\n\n" );
    while (1)
    {
	printf("-----------------------------------------------------------\n\n" );
        cpuload = GetCPULoad();
	read_dht11_dat();
        tempbf = dht11_dat[2];
        humbf = dht11_dat[0];
        tempaf = dht11_dat[3];
        humaf = dht11_dat[1];
        publishMSG(cpuload , tempbf , humbf , tempaf , humaf);
        printf("Veröffentlichte Nachricht: %s\n" , buffer);
        //printf("publish %d\n\n" , rc);
	printf("-----------------------------------------------------------\n\n" );
	
        sleep(3);        
    }
    
    return EXIT_SUCCESS;
}
