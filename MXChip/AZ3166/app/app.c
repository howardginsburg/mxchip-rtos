#include <stdio.h>
#include "screen.h"
#include "sntp_client.h"
#include "wwd_networking.h"
#include "wiced_sdk.h"
#include "nx_api.h"
#include "nxd_mqtt_client.h"
#include <tx_api.h>
#include "app_config.h"
#include "board_init.h"
#include "sensor.h"

// Define constants for MQTT client configuration
#define MQTT_CLIENT_STACK_SIZE     4096
#define MQTT_THREAD_PRIORITY       2
#define MQTT_KEEP_ALIVE_TIMER      300
#define MQTT_MESSAGE_EVENT         1
#define ALL_EVENTS                 3
#define QOS0                       0
#define QOS1                       1

// Declare static variables for MQTT client and related data
static ULONG mqtt_client_stack[MQTT_CLIENT_STACK_SIZE / sizeof(ULONG)];
static NXD_MQTT_CLIENT mqtt_client;
static TX_EVENT_FLAGS_GROUP mqtt_app_flag;
static UCHAR message_buffer[NXD_MQTT_MAX_MESSAGE_LENGTH];
static UCHAR topic_buffer[NXD_MQTT_MAX_TOPIC_NAME_LENGTH];
static ULONG error_counter = 0;

/**
 * @brief Callback function for MQTT client disconnection.
 * 
 * This function is called when the MQTT client disconnects from the server.
 * It prints a message indicating the disconnection.
 * 
 * @param client_ptr Pointer to the MQTT client structure.
 */
static VOID mqtt_disconnect(NXD_MQTT_CLIENT *client_ptr)
{
    //This macro used to indicate that the client_ptr paramter is
    //not used within the function. It helps to avoid compiler warnings about unused parameters.
    NX_PARAMETER_NOT_USED(client_ptr);
    printf("Client disconnected from server\n");
}

/**
 * @brief Callback function for MQTT message arrival.
 * 
 * This function is called when a new MQTT message arrives.
 * It sets the MQTT_MESSAGE_EVENT flag to indicate the arrival of a new message.
 * 
 * @param client_ptr Pointer to the MQTT client structure.
 * @param number_of_messages Number of messages received.
 */
static VOID mqtt_message_arrive(NXD_MQTT_CLIENT* client_ptr, UINT number_of_messages)
{
    //This macro used to indicate that the client_ptr and number_of_messages parameters are
    //not used within the function. It helps to avoid compiler warnings about unused parameters.
    NX_PARAMETER_NOT_USED(client_ptr);
    NX_PARAMETER_NOT_USED(number_of_messages);

    //sets the MQTT_MESSAGE_EVENT flag in the mqtt_app_flag event flags group. The TX_OR option specifies
    //that the flag should be set using a logical OR operation. This flag indicates that a new MQTT message has arrived, 
    //and it can be used by other threads (such as the MQTT subscribe thread) to trigger message processing.
    tx_event_flags_set(&mqtt_app_flag, MQTT_MESSAGE_EVENT, TX_OR);
}

/**
 * @brief Entry function for the MQTT subscribe thread.
 * 
 * This thread waits for the MQTT_MESSAGE_EVENT flag and processes incoming MQTT messages.
 * It retrieves the topic and message content and prints them to the console.
 * 
 * @param parameter Thread parameter (not used).
 */
void mqtt_subscribe_thread_entry(ULONG parameter)
{
    UINT status;
    ULONG events;
    UINT topic_length, message_length;

    while (1)
    {
        // Wait for message event
        tx_event_flags_get(&mqtt_app_flag, ALL_EVENTS, TX_OR_CLEAR, &events, TX_WAIT_FOREVER);
        if (events & MQTT_MESSAGE_EVENT)
        {
            status = nxd_mqtt_client_message_get(&mqtt_client, topic_buffer, sizeof(topic_buffer), &topic_length,
                                                 message_buffer, sizeof(message_buffer), &message_length);
            if (status == NXD_MQTT_SUCCESS)
            {
                topic_buffer[topic_length] = 0;
                message_buffer[message_length] = 0;
                printf("Received topic: %s, message: %s\n", topic_buffer, message_buffer);
            }
        }
    }
}

/**
 * @brief Entry function for the MQTT publish thread.
 * 
 * This thread publishes a predefined message to the MQTT broker every 5 seconds.
 * It prints a message to the console each time a message is published.
 * 
 * @param parameter Thread parameter (not used).
 */
void mqtt_publish_thread_entry(ULONG parameter)
{
    //board_init();
    //UINT status;

    // Define the message buffer and its size
    UCHAR message[256];
    
    while (1)
    {
        lps22hb_t lps22hb_data = lps22hb_data_read();
        hts221_data_t hts221_data = hts221_data_read();
        lsm6dsl_data_t lsm6dsl_data = lsm6dsl_data_read();
        lis2mdl_data_t lis2mdl_data = lis2mdl_data_read();

         // Debug prints to check sensor data
         printf("Pressure: %.2f hPa\n", (double)lps22hb_data.pressure_hPa);
         printf("Temperature: %.2f °C\n", (double)lps22hb_data.temperature_degC);
         printf("Humidity: %.2f %%\n", (double)hts221_data.humidity_perc);
         printf("Acceleration: [%.2f, %.2f, %.2f] mg\n", 
                (double)lsm6dsl_data.acceleration_mg[0],
                (double)lsm6dsl_data.acceleration_mg[1],
                (double)lsm6dsl_data.acceleration_mg[2]);
         printf("Angular rate: [%.2f, %.2f, %.2f] mdps\n", 
                (double)lsm6dsl_data.angular_rate_mdps[0],
                (double)lsm6dsl_data.angular_rate_mdps[1],
                (double)lsm6dsl_data.angular_rate_mdps[2]);
         printf("Magnetic: [%.2f, %.2f, %.2f] mG\n", 
                (double)lis2mdl_data.magnetic_mG[0],
                (double)lis2mdl_data.magnetic_mG[1],
                (double)lis2mdl_data.magnetic_mG[2]);
       
        // Build the message string with sensor data include all attributes
        snprintf((char*)message, sizeof(message), 
                 "{\"pressure_hPa\":%.2f,\"temperature_degC\":%.2f,\"humidity_perc\":%.2f,\"acceleration_mg\":[%.2f,%.2f,%.2f],\"angular_rate_mdps\":[%.2f,%.2f,%.2f],\"magnetic_mG\":[%.2f,%.2f,%.2f]}",
                 (double)lps22hb_data.pressure_hPa,
                 (double)lps22hb_data.temperature_degC,
                 (double)hts221_data.humidity_perc,
                 (double)lsm6dsl_data.acceleration_mg[0],
                 (double)lsm6dsl_data.acceleration_mg[1],
                 (double)lsm6dsl_data.acceleration_mg[2],
                 (double)lsm6dsl_data.angular_rate_mdps[0],
                 (double)lsm6dsl_data.angular_rate_mdps[1],
                 (double)lsm6dsl_data.angular_rate_mdps[2],
                 (double)lis2mdl_data.magnetic_mG[0],
                 (double)lis2mdl_data.magnetic_mG[1],
                 (double)lis2mdl_data.magnetic_mG[2]);


        printf("Publishing message...\n");
        // Publish message every 5 seconds
        // status = nxd_mqtt_client_publish(&mqtt_client, TOPIC_NAME, strlen(TOPIC_NAME),
        //                                  (CHAR*)message, strlen((CHAR*)message), 0, QOS1, NX_WAIT_FOREVER);
        // if (status)
        // {
        //     printf("Error publishing message: 0x%02x\n", status);
        //     error_counter++;
        // }

        // Flash the user led
        USER_LED_ON();
        tx_thread_sleep(1 * TX_TIMER_TICKS_PER_SECOND);
        USER_LED_OFF();

        // Sleep for x seconds
        tx_thread_sleep(SEND_INTERVAL * TX_TIMER_TICKS_PER_SECOND);
    }
}

/**
 * @brief Entry function for the main application thread.
 * 
 * This thread initializes the network, connects to the MQTT broker, and creates
 * separate threads for subscribing to and publishing MQTT messages. It also handles
 * the main application logic.
 * 
 * @param parameter Thread parameter (not used).
 */
void app_thread_entry(ULONG parameter)
{
    UINT status;
    NXD_ADDRESS server_ip;

    printf("Starting Application ThreadX thread\r\n\r\n");

    screen_print(HOSTNAME, L0);

    // Initialize the network
    if ((status = wwd_network_init(WIFI_SSID, WIFI_PASSWORD, WIFI_MODE)))
    {
        printf("ERROR: Failed to initialize the network (0x%08x)\r\n", status);
        return;
    }

    // Connect to the network
    if ((status = wwd_network_connect()))
    {
        printf("ERROR: Failed to connect to the network (0x%08x)\r\n", status);
        return;
    }

    WIFI_LED_ON();

    // Initialize the LCD Screen
    screen_print(WIFI_SSID, L1);

    // Get the IP address and print it to the screen.
    ULONG ip_address, network_mask;
    nx_ip_address_get(&nx_ip, &ip_address, &network_mask);

    // Create a char buffer to hold the IP address
    char ip_buffer[16];
    // Convert the IP address to a string
    snprintf(ip_buffer, sizeof(ip_buffer), "%lu.%lu.%lu.%lu", 
             (ip_address >> 24) & 0xFF, 
             (ip_address >> 16) & 0xFF, 
             (ip_address >> 8) & 0xFF, 
             ip_address & 0xFF);
    // Print the IP address to the screen
    screen_print(ip_buffer, L2);

    // Create MQTT client instance
    status = nxd_mqtt_client_create(&mqtt_client, "my_client", CLIENT_ID_STRING, strlen(CLIENT_ID_STRING),
                                    &nx_ip, &nx_pool[0], (VOID*)mqtt_client_stack, sizeof(mqtt_client_stack),
                                    MQTT_THREAD_PRIORITY, NX_NULL, 0);
    if (status)
    {
        printf("Error creating MQTT client: 0x%02x\n", status);
        error_counter++;
        return;
    }

    // Set disconnect notification
    nxd_mqtt_client_disconnect_notify_set(&mqtt_client, mqtt_disconnect);

    // Create event flags
    status = tx_event_flags_create(&mqtt_app_flag, "mqtt_app_flag");
    if (status)
    {
        printf("Error creating event flags: 0x%02x\n", status);
        error_counter++;
        return;
    }

    // Resolve MQTT broker address (IP or DNS)
    server_ip.nxd_ip_version = 4;
    server_ip.nxd_ip_address.v4 = MQTT_SERVER_ADDRESS;

#ifdef MQTT_USERNAME
    // Set MQTT username and password
    status = nxd_mqtt_client_login_set(&mqtt_client, MQTT_USERNAME, strlen(MQTT_USERNAME), MQTT_PASSWORD, strlen(MQTT_PASSWORD));
    if (status)
    {
        printf("Error setting MQTT login: 0x%02x\n", status);
        error_counter++;
        return;
    }
#endif

    // Connect to MQTT broker using resolved address and configured port
    status = nxd_mqtt_client_connect(&mqtt_client, &server_ip, MQTT_SERVER_PORT,
                                     MQTT_KEEP_ALIVE_TIMER, 0, NX_WAIT_FOREVER);
    if (status)
    {
        printf("Error connecting MQTT client: 0x%02x\n", status);
        error_counter++;
        return;
    }

    CLOUD_LED_ON();

    // Subscribe to topic
    status = nxd_mqtt_client_subscribe(&mqtt_client, TOPIC_NAME, strlen(TOPIC_NAME), QOS0);
    if (status)
    {
        printf("Error subscribing to topic: 0x%02x\n", status);
        error_counter++;
    }

    // Set receive notify function
    status = nxd_mqtt_client_receive_notify_set(&mqtt_client, mqtt_message_arrive);
    if (status)
    {
        printf("Error setting notify function: 0x%02x\n", status);
        error_counter++;
    }

    // Create a separate thread for processing incoming messages
    TX_THREAD mqtt_subscribe_thread;
    static ULONG mqtt_subscribe_thread_stack[1024];
    status = tx_thread_create(&mqtt_subscribe_thread, "mqtt_subscribe_thread", mqtt_subscribe_thread_entry, 0,
                              mqtt_subscribe_thread_stack, sizeof(mqtt_subscribe_thread_stack),
                              MQTT_THREAD_PRIORITY, MQTT_THREAD_PRIORITY, TX_NO_TIME_SLICE, TX_AUTO_START);
    if (status)
    {
        printf("Error creating message processing thread: 0x%02x\n", status);
        error_counter++;
        return;
    }

    // Create a separate thread for publishing messages
    TX_THREAD mqtt_publish_thread;
    static ULONG mqtt_publish_thread_stack[1024];
    status = tx_thread_create(&mqtt_publish_thread, "mqtt_publish_thread", mqtt_publish_thread_entry, 0,
                              mqtt_publish_thread_stack, sizeof(mqtt_publish_thread_stack),
                              MQTT_THREAD_PRIORITY, MQTT_THREAD_PRIORITY, TX_NO_TIME_SLICE, TX_AUTO_START);
    if (status)
    {
        printf("Error creating publish thread: 0x%02x\n", status);
        error_counter++;
        return;
    }

    // Main thread can perform other tasks or sleep
    while (1)
    {
        tx_thread_sleep(TX_TIMER_TICKS_PER_SECOND);
    }

    // Unsubscribe and disconnect
    nxd_mqtt_client_unsubscribe(&mqtt_client, TOPIC_NAME, strlen(TOPIC_NAME));
    nxd_mqtt_client_disconnect(&mqtt_client);
    nxd_mqtt_client_delete(&mqtt_client);
}