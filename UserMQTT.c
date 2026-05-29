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
mqtt_client_t       *client                   = NULL;
static SemaphoreHandle_t userMqttConnectSemaphore = NULL;

/* ---------------------------------------------------------------------------*/

/**
  * @brief  Crée le sémaphore de connexion et la tâche FreeRTOS MQTT.
  * @retval  0 si OK, -1 si erreur
  */
int32_t UserMQTTInit(void)
{
    /* Sémaphore binaire donné par mqtt_connection_cb quand ACCEPTED */
    userMqttConnectSemaphore = xSemaphoreCreateBinary();
    if (userMqttConnectSemaphore == NULL)
        return -1;

    if (xTaskCreate(UserMQTTTask, "UserMQTTTask", 512, 0, osPriorityNormal, 0) != pdPASS)
        return -1;

    return 0;
}

/* ---------------------------------------------------------------------------*/

/**
  * @brief  Tâche FreeRTOS MQTT.
  *         - Alloue le client
  *         - Lance connect() puis attend le sémaphore (donné par le callback)
  *         - Publie un message toutes les secondes
  */
void UserMQTTTask(void *param)
{
    vTaskDelay(1000);
    debugLogInfo("Init MQTT task");

    client = mqtt_client_new();

    /* Boucle de connexion :
     * connect() lance la tentative TCP+MQTT (asynchrone).
     * On attend que mqtt_connection_cb donne le sémaphore (timeout 5s).
     * Si timeout ou erreur TCP immédiate : on retente. */
    while (1)
    {
        if (connect(client) == 0)
        {
            /* Tentative lancée — attendre la confirmation du callback */
            if (xSemaphoreTake(userMqttConnectSemaphore, 5000) == pdTRUE)
                break;  /* Sémaphore donné : MQTT_CONNECT_ACCEPTED */
        }
        debugLogInfo("Waiting connection...");
        vTaskDelay(1000);
    }

    debugLogInfo("Connected !!!");

    /* Boucle principale : publication périodique */
    while (1)
    {
        if (mqtt_client_is_connected(client))
        {
            const char *payload = "hello from E10";
            mqtt_publish(client, MQTT_PUB_TOPIC, payload, strlen(payload),
                         0, 0, NULL, NULL);
        }
        else
        {
            /* Reconnexion si lien perdu */
            debugLogInfo("Connection lost, reconnecting...");
            connect(client);
            xSemaphoreTake(userMqttConnectSemaphore, 5000);
        }

        vTaskDelay(1000);
    }
}

/* ---------------------------------------------------------------------------*/

/**
  * @brief  Lance la connexion TCP+MQTT vers le broker.
  *         Asynchrone : le résultat arrive dans mqtt_connection_cb.
  *         (doc : mqtt_client_connect)
  * @retval  0 si tentative lancée, -1 si erreur TCP immédiate
  */
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
        return -1;

    return 0;
}

/* ---------------------------------------------------------------------------*/

/**
  * @brief  Déconnexion propre du broker MQTT.
  *         Envoie le paquet DISCONNECT avant de couper le TCP,
  *         le broker libère immédiatement la session et le client_id.
  *         À appeler avant reset ou extinction.
  *         (doc : mqtt_disconnect)
  */
void userMqttDisconnect(mqtt_client_t *client)
{
    if (client == NULL)
        return;

    if (mqtt_client_is_connected(client))
    {
        mqtt_disconnect(client);
        debugLogInfo("MQTT disconnected");
    }
}

/* ---------------------------------------------------------------------------*/

/**
  * @brief  Callback statut de connexion au broker.
  *         Appelé de manière asynchrone par lwIP après mqtt_client_connect().
  *         - status != 0 : connexion refusée ou perdue → on retente
  *         - status == 0 (ACCEPTED) : on donne le sémaphore
  *         (doc : mqtt_connection_cb_t)
  */
static void mqtt_connection_cb(mqtt_client_t *client, void *arg,
                                mqtt_connection_status_t status)
{
    (void)arg;

    if (status)
    {
        /* Connexion refusée ou perdue */
        debugLogInfo("MQTT Connect Status : %d", status);
        connect(client);
        return;
    }

    /* MQTT_CONNECT_ACCEPTED (status == 0) : débloquer la tâche */
    if (userMqttConnectSemaphore)
        xSemaphoreGive(userMqttConnectSemaphore);
}

/* ---------------------------------------------------------------------------*/

/**
  * @brief  Callback topic d'un message entrant (subscribe).
  */
void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len)
{
    (void)arg;
    (void)topic;
    (void)tot_len;
}

/**
  * @brief  Callback données d'un message entrant.
  *         flags & MQTT_DATA_FLAG_LAST → dernier fragment.
  */
void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags)
{
    (void)arg;
    (void)data;
    (void)len;
    (void)flags;
}
