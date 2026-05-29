/**
 * @file UserMQTT.c
 *
 * @date May 27, 2026
 * @author SERRA
 */

/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    UserMQTT.c
  * @brief   MQTT client — lwIP 2.1.x
  *          Ref: https://www.nongnu.org/lwip/2_1_x/group__mqtt.html
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "mqtt.h"
#include "UserMQTT.h"
#include <string.h>
#include "debugTerminal.h"

/* Variables -----------------------------------------------------------------*/
mqtt_client_t *client = NULL;

/* ---------------------------------------------------------------------------*/


int32_t UserMQTTInit(void)
{
    if (xTaskCreate(UserMQTTTask, "UserMQTTTask", 512, 0, osPriorityNormal, 0) != pdPASS)
    {

    	return -1;
    }


    return 0;
}

/* ---------------------------------------------------------------------------*/


void UserMQTTTask(void *param)
{

    vTaskDelay(1000);

    debugLogInfo("Init MQTT task");

    client = mqtt_client_new();


    while (connect(client) != 0)
    {
    	debugLogInfo("Waiting connection");
        vTaskDelay(1000);
    }


    debugLogInfo("Connected !!!");

    while (1)
    {
        if (mqtt_client_is_connected(client))
        {
            const char *payload = "hello from E10";
            mqtt_publish(client, MQTT_PUB_TOPIC, payload, strlen(payload), 0, 0, NULL, NULL);
        }

        else
        {

            connect(client);
        }

        vTaskDelay(1000);
    }
}

/* ---------------------------------------------------------------------------*/


int connect(mqtt_client_t *client)
{
    struct mqtt_connect_client_info_t ci;
    err_t err;

    ip4_addr_t ip_addr;
    IP4_ADDR(&ip_addr,
             MQTT_BROKER_IP_0,
             MQTT_BROKER_IP_1,
             MQTT_BROKER_IP_2,
             MQTT_BROKER_IP_3);

    memset(&ci, 0, sizeof(ci));

    ci.client_id = MQTT_CLIENT_ID;

    err = mqtt_client_connect(client, &ip_addr, MQTT_Port,
                              mqtt_connection_cb, 0, &ci);

    if (err != ERR_OK)
    {
        return -1;
    }

    return 0;
}

/* ---------------------------------------------------------------------------*/


static void mqtt_connection_cb(mqtt_client_t *client, void *arg,
                         mqtt_connection_status_t status)
{
    (void)arg;

    return; // A ENLEVER !!!


    if (status == MQTT_CONNECT_ACCEPTED)
    {

        /*mqtt_set_inpub_callback(client,
                                mqtt_incoming_publish_cb,
                                mqtt_incoming_data_cb,
                                NULL);

        mqtt_subscribe(client, MQTT_SUB_TOPIC, 1, NULL, NULL);*/
    }
    */

}

/* ---------------------------------------------------------------------------*/


void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len)
{
    (void)arg;
    (void)topic;
    (void)tot_len;
}


void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags)
{
    (void)arg;
    (void)data;
    (void)len;
    (void)flags;
}
