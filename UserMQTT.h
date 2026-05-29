/**
 * @file UserMQTT.h
 *
 * @date May 27, 2026
 * @author SERRA
 */

/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    UserMQTT.h
  * @brief   MQTT client — lwIP 2.1.x
  *          Ref: https://www.nongnu.org/lwip/2_1_x/group__mqtt.html
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef __USER_MQTT_H__
#define __USER_MQTT_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "mqtt.h"
#include "lwip/apps/mqtt_priv.h"
#include "lwip/ip_addr.h"
#include "cmsis_os.h"

/* Exported defines ----------------------------------------------------------*/
#define MQTT_BROKER_IP_0    10
#define MQTT_BROKER_IP_1    38
#define MQTT_BROKER_IP_2    1
#define MQTT_BROKER_IP_3    5

#define MQTT_Port           1883
#define MQTT_CLIENT_ID      "E10"

#define MQTT_PUB_TOPIC      "GEII/NRF/E10"
#define MQTT_SUB_TOPIC      "GEII/NRF/E10/cmd"

/* Exported variables --------------------------------------------------------*/
extern mqtt_client_t *client;

/* Exported functions --------------------------------------------------------*/
int32_t  UserMQTTInit(void);
void     UserMQTTTask(void *param);
int     connect(mqtt_client_t *client);
static void     mqtt_connection_cb(mqtt_client_t *client, void *arg,
                             mqtt_connection_status_t status);

#ifdef __cplusplus
}
#endif

#endif /* __USER_MQTT_H__ */
