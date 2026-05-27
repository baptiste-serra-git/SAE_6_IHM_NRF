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

/* Variables -----------------------------------------------------------------*/
mqtt_client_t *client = NULL;

/* ---------------------------------------------------------------------------*/

/**
  * @brief  Crée la tâche FreeRTOS MQTT.
  * @retval  0 si OK, -1 si erreur création tâche
  */
int32_t UserMQTTInit(void)
{
    if (xTaskCreate(UserMQTTTask, "UserMQTTTask", 512, 0, osPriorityNormal, 0) != pdPASS)
        return -1;

    return 0;
}

/* ---------------------------------------------------------------------------*/

/**
  * @brief  Tâche FreeRTOS MQTT.
  *         - Alloue le client
  *         - Tente la connexion en boucle jusqu'au succès
  *         - Publie un message toutes les secondes
  */
void UserMQTTTask(void *param)
{
    /* Laisser le temps à LwIP de s'initialiser */
    vTaskDelay(1000);

    /* Allocation du client MQTT */
    client = mqtt_client_new();

    /* Boucle de connexion : on réessaie toutes les secondes tant que non connecté */
    while (connect(client))
    {
        vTaskDelay(1000);
    }

    /* Boucle principale : publication périodique */
    while (1)
    {
        if (mqtt_client_is_connected(client))
        {
            const char *payload = "hello from STM32";
            mqtt_publish(client,
                         MQTT_PUB_TOPIC,
                         payload,
                         strlen(payload),
                         0,     /* QoS 0 */
                         0,     /* retain */
                         NULL,
                         NULL);
        }
        else
        {
            /* Reconnexion si lien perdu */
            connect(client);
        }

        vTaskDelay(1000);
    }
}

/* ---------------------------------------------------------------------------*/

/**
  * @brief  Connexion au broker MQTT.
  *         Pattern issu de la doc officielle lwIP 2.1.x :
  *         mqtt_client_connect(client, ip, port, cb, arg, &ci)
  * @param  client : pointeur sur le client MQTT alloué
  * @retval  0 si connexion lancée avec succès, -1 sinon
  */
void connect(mqtt_client_t *client)
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
        /* Echec immédiat (pas de route réseau, etc.) */
    }
}

/* ---------------------------------------------------------------------------*/

/**
  * @brief  Callback statut de connexion au broker.
  *         Appelé de manière asynchrone par lwIP après mqtt_client_connect().
  *         (doc : mqtt_connection_cb_t)
  */
void mqtt_connection_cb(mqtt_client_t *client, void *arg,
                         mqtt_connection_status_t status)
{
    (void)arg;

    if (status == MQTT_CONNECT_ACCEPTED)
    {
        /* Connecté — on peut souscrire ici si besoin */
        mqtt_set_inpub_callback(client,
                                mqtt_incoming_publish_cb,
                                mqtt_incoming_data_cb,
                                NULL);

        mqtt_subscribe(client, MQTT_SUB_TOPIC, 1, NULL, NULL);
    }
    /* Si status != ACCEPTED : la tâche relancera connect() */
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
