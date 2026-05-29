/*

 * ledHandler.c

 *

 * Created on: Sep 12, 2023

 * Author: Jean-Roch

 */



#include "ledHandler.h"

#include "main.h"

#include "gpio.h"

#include "FreeRTOS.h"

#include "task.h"

#include "semphr.h"

#include "cmsis_os.h"

#include "debugTerminal.h"

#include "mqtt_priv.h"

#include "lwip.h"





mqtt_client_t static_client;





/*

 * La tâche "testTask_01" doit permettre d'afficher la valeur de "xTaskGetTickCount()" toutes les 100ms

 * en haut à droite de l'écran LCD

 */



void testTask_01(void *param)

{



 debugLogInfo("je suis dans ma task");

 vTaskDelay(1000);



 while( example_do_connect()!=1){

 debugLogInfoTrace("status mqtt:je tente de me connecter");

 vTaskDelay(1000);



 }

 //mqtt_client_t static_client;



 //uint32_t tick = 0;





 while (1)

 {





 vTaskDelay(5000);

 example_publish();







 }





 }



int32_t testTasksInit(void)

{

 if (xTaskCreate(testTask_01, "TEST_TASK01", 512, 0, osPriorityNormal,

 0)!=pdPASS)

 return -1;

 return 0;

}

int example_do_connect()

{

 struct mqtt_connect_client_info_t ci;

 err_t err;

 ip4_addr_t ip_addr;

 ip4addr_aton("10.38.1.5",&ip_addr);







 /* Setup an empty client info structure */

 memset(&ci, 0, sizeof(ci));



 /* Minimal amount of information required is client identifier, so set it here */

 ci.client_id = "bench_E02";



 /* Initiate client and connect to server, if this fails immediately an error code is returned

 otherwise mqtt_connection_cb will be called with connection result after attempting

 to establish a connection with the server.

 For now MQTT version 3.1.1 is always used */



 err = mqtt_client_connect(&static_client,&ip_addr, MQTT_PORT, mqtt_connection_cb, 0, &ci);



 //debugLogInfo("avant le if");

 /* For now just print the result code if something goes wrong */

 if(err != ERR_OK) {

 debugLogInfo("mqtt_connect return %d\n", err);

 return -1;

 }

 //debugLogInfo("apres le if");

 return 1;

};



static void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status)

{

 err_t err;

 debugLogInfo("dans call back");

 debugLogInfoTrace("status mqtt:%d",status);

 /* Setup callback for incoming publish requests */

 mqtt_set_inpub_callback(&static_client, mqtt_incoming_publish_cb, mqtt_incoming_data_cb, arg);



 /* Subscribe to a topic named "subtopic" with QoS level 1, call mqtt_sub_request_cb with result */

 err = mqtt_subscribe(&static_client, "GEII/NRF/E02", 1, mqtt_sub_request_cb, arg);



 // mqtt_disconnect(client);

}

void example_publish()

{

 const char *pub_payload= "faut ecrire sur pp";

 err_t err;

 u8_t qos = 0; /* 0 1 or 2, see MQTT specification */

 u8_t retain = 0; /* No don't retain such crappy payload... */

 err = mqtt_publish(&static_client, "GEII/NRF/E02",pub_payload, strlen(pub_payload), qos, retain, mqtt_pub_request_cb, 0);

 if(err != ERR_OK) {

 printf("Publish err: %d\n", err);

 }

}



/* Called when publish is complete either with sucess or failure */

static void mqtt_pub_request_cb(void *arg, err_t result)

{

 if(result != ERR_OK) {

 printf("Publish result: %d\n", result);

 }

}





static int inpub_id;

static void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len)

{

 debugLogInfo("Incoming publish at topic %s with total length %u\n", topic, (unsigned int)tot_len);



 /* Decode topic string into a user defined reference */

// if(strcmp(topic, "print_payload") == 0) {

 inpub_id = 0;

// } else if(topic[0] == 'A') {

// /* All topics starting with 'A' might be handled at the same way */

// inpub_id = 1;

// } else {

// /* For all other topics */

// inpub_id = 2;

// }

}



static void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags)

{

 uint8_t buff_data[128];

 debugLogInfo("Incoming publish payload with length %d, flags %u\n", len, (unsigned int)flags);







 if(flags & MQTT_DATA_FLAG_LAST) {

 /* Last fragment of payload received (or whole part if payload fits receive buffer

 See MQTT_VAR_HEADER_BUFFER_LEN) */



 /* Call function or do action depending on reference, in this case inpub_id */

 if(inpub_id == 0) {

 /* Don't trust the publisher, check zero termination */



 memcpy(buff_data, data, len);

 buff_data[len] = '\0';

 debugLogInfo("mqtt_incoming_data_cb: %s\n", (const char *)buff_data);





 } else if(inpub_id == 1) {

 /* Call an 'A' function... */

 } else {

 printf("mqtt_incoming_data_cb: Ignoring payload...\n");

 }

 } else {

 /* Handle fragmented payload, store in buffer, write to file or whatever */

 }

}

static void mqtt_sub_request_cb(void *arg, err_t result)

{

 /* Just print the result code here for simplicity,

 normal behaviour would be to take some action if subscribe fails like

 notifying user, retry subscribe or disconnect from server */

 printf("Subscribe result: %d\n", result);

}




--
Jérémy LATIL
BUT3 Génie Electrique et Informatique Industrielle
Formation en Apprentissage Electronique et Systèmes Embarqués
IUT de Montpellier Sète
06.40.95.18.29