#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "asio.hpp"
#include "esp_timer.h"
#include <math.h>

#include "WiFiConnection.hpp"
#include "server.hpp"
#include "System.hpp"

extern "C" 
{
   void app_main(void);
}

void app_main(void)
{
   const char* TAG = "SYSTEM";
   const short PORT = 3333;

   System& sys = System::getInstance();
   ESP_LOGI(TAG, "Inicjalizacja systemu");
   sys.initFileSystem();
   sys.initLedCDriver();
   sys.initEngines();

   WiFiConnection& wifi = WiFiConnection::instance();
   {
      char ssids[3][20] = {"wifi2", "Wifis9", "900/2100/2"};
      char passs[3][20] = {"iubppac46eze", "zaq1@WSX", "ppp123abcAyA"};
      int connectionCount = 0;

      while(!wifi.isConnected() && connectionCount < 3)
      {
         wifi.connect(ssids[connectionCount], passs[connectionCount]);
         connectionCount++;
         vTaskDelay(1000/portTICK_PERIOD_MS);
      }
      if(!wifi.isConnected())
      {
         ESP_LOGE(TAG, "nie mozna polaczyc sie z siecia wifi!!");
         ESP_LOGE(TAG, "Restart za 5 sekund");
         for(int i = 0; i < 5; i++)
         {
            printf(".");
            vTaskDelay(1000/portTICK_PERIOD_MS);
         }
         esp_restart();
      }
   }

   ESP_LOGI(TAG, "Uruchamianie kontekstu ASIO");
   ESP_LOGI(TAG, "Uruchamianie serwera tcp na porcie: %i", PORT);
   asio::io_context io_context;
   server s(io_context, PORT);
   xTaskCreatePinnedToCore([](void* ptr) -> void {
      ((asio::io_context*)ptr)->run();
   }, "tcp_server", 4096, &io_context, 30, NULL, 1);

   xTaskCreatePinnedToCore([] (void*ptr) -> void{
      gpio_config_t pinEncoderConfig;
      memset(&pinEncoderConfig, 0, sizeof(gpio_config_t));
      pinEncoderConfig.intr_type       = gpio_int_type_t::GPIO_INTR_DISABLE;
      pinEncoderConfig.mode            = gpio_mode_t::GPIO_MODE_INPUT_OUTPUT;
      pinEncoderConfig.pin_bit_mask    = ((1 << (int)gpio_num_t::GPIO_NUM_2));
      pinEncoderConfig.pull_up_en      = GPIO_PULLUP_DISABLE;
      pinEncoderConfig.pull_down_en    = GPIO_PULLDOWN_DISABLE;
      gpio_config(&pinEncoderConfig);
      while(1)
      {
         gpio_set_level(gpio_num_t::GPIO_NUM_2, !gpio_get_level(gpio_num_t::GPIO_NUM_2));
         vTaskDelay(800/portTICK_PERIOD_MS);
      }
   }, "LED_Blink_Task", 4096, NULL, 5, NULL, 1);


   while(true)
   {
      switch (sys.getRobotMode())
      {
         case RobotMode::ROBOT_MANUAL:
            switch (sys._manRobotDir)
            {
            case RobotDirection::DIR_FORWARD:
               sys.robotForward();
               break;
            case RobotDirection::DIR_BACKWARD:
               sys.robotBackward();
               break;
            case RobotDirection::DIR_LEFT:
               sys.robotRotateLeft();
               break;
            case RobotDirection::DIR_RIGHT:
               sys.robotRotateRight();
               break;
            default:
               sys.robotStop();
               break;
            }
         break;
         case RobotMode::ROBOT_AUTO:
            // if robot is in full path and run mode 
            // get next node from quuee
            // pid controller for absolute distance
            // if quuee is empty you reached goal
         break;
      }
      vTaskDelay(100 / portTICK_PERIOD_MS);   // just to keep the watchdog happy
   }
}