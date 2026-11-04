#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "main.h"
#include "FreeRTOS.h"
#include "task.h"

UART_HandleTypeDef uart2;

void clock_init();

int green_led_intv = 0;
int orange_led_intv = 0;
int red_led_intv = 0;
int blue_led_intv = 0;
struct Task_Params {
  int* led_intv;
  uint16_t led_pin;
};

static void led_task(void *args)
{
  struct Task_Params *params = (struct Task_Params *) args;
  while (1)
  {
    if (*(params->led_intv) == 0) {
      HAL_GPIO_WritePin(LED_PORT, params->led_pin, GPIO_PIN_RESET);
      HAL_Delay(200);
    } else {
      HAL_GPIO_TogglePin(LED_PORT, params->led_pin);
      HAL_Delay(*(params->led_intv));
    }
  }
}

static void io_task(void *args __attribute((unused)))
{
  while (1) {
    char color[10];
    while (1) {
      printf("Enter LED [g(reen), o(range), r(ed), or b(lue)]: ");
      int ret =scanf("%9s", color);
      if (strncmp(color, "green", 9) == 0 || strncmp(color, "g", 9) == 0
          || strncmp(color, "orange", 9) == 0 || strncmp(color, "o", 9) == 0
          || strncmp(color, "red", 9) == 0 || strncmp(color, "r", 9) == 0
          || strncmp(color, "blue", 9) == 0 || strncmp(color, "b", 9) == 0) {
        break;
      }
      if (strlen(color) == 9) {
        printf("\r\n");
      }
      printf("%s does not identify an LED.\r\n", color);
    }
    
    int intv = -1;
    while (1) {
      printf("Enter interval in ms [200 - 4000 or 0 to turn off]: ");
      int ret = scanf("%d", &intv);
      if (ret == 1 && (intv == 0 || (intv >= 200 && intv <= 4000))) {
        break;
      }
      if (ret != 1) {
        getchar();
        printf("\r\n");
      }
      printf("This input is not a valid interval.\r\n");
    }
    
    if (strncmp(color, "green", 9) == 0 || strncmp(color, "g", 9) == 0) {
      green_led_intv = intv;
    } else if (strncmp(color, "orange", 9) == 0 || strncmp(color, "o", 9) == 0) {
      orange_led_intv = intv;
    } else if (strncmp(color, "red", 9) == 0 || strncmp(color, "r", 9) == 0) {
      red_led_intv = intv;
    } else if (strncmp(color, "blue", 9) == 0 || strncmp(color, "b", 9) == 0) {
      blue_led_intv = intv;
    }
  }
}

void main(void)
{
  HAL_Init();
  clock_init();
  SystemCoreClockUpdate(); // Update the internal clock frequency variable

  // Initialize LED GPIO
  __HAL_RCC_GPIOD_CLK_ENABLE();

  GPIO_InitTypeDef gpio_init = {0};
  gpio_init.Pin = LED_PIN;
  gpio_init.Mode = GPIO_MODE_OUTPUT_PP;
  gpio_init.Pull = GPIO_NOPULL;
  gpio_init.Speed = GPIO_SPEED_LOW;
  gpio_init.Alternate = 0;
  HAL_GPIO_Init(LED_PORT, &gpio_init);

  gpio_init.Pin = GPIO_PIN_13;
  HAL_GPIO_Init(LED_PORT, &gpio_init);

  gpio_init.Pin = GPIO_PIN_14;
  HAL_GPIO_Init(LED_PORT, &gpio_init);

  gpio_init.Pin = GPIO_PIN_15;
  HAL_GPIO_Init(LED_PORT, &gpio_init);

  // Initialize UART
  uart2.Instance = USART2;
  uart2.Init.BaudRate = 115200;
  uart2.Init.Mode = UART_MODE_TX_RX;
  uart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  uart2.Init.WordLength = UART_WORDLENGTH_8B;
  uart2.Init.StopBits = UART_STOPBITS_1;
  uart2.Init.Parity = UART_PARITY_NONE;
  uart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&uart2) != HAL_OK)
  {
    while (1);
  }

  // Dummy write, because the first byte seems to always be dropped
  USART2->DR = 0;
  while (!(USART2->SR & USART_SR_TC));

  static struct Task_Params green_params = {&green_led_intv, GPIO_PIN_12};
  static struct Task_Params orange_params = {&orange_led_intv, GPIO_PIN_13};
  static struct Task_Params red_params = {&red_led_intv, GPIO_PIN_14};
  static struct Task_Params blue_params = {&blue_led_intv, GPIO_PIN_15};

  if (xTaskCreate(led_task, "greenLED", 512, &green_params, configMAX_PRIORITIES - 1, NULL) != pdPASS)
  {
    while (1);
  }
  if (xTaskCreate(led_task, "orangeLED", 512, &orange_params, configMAX_PRIORITIES - 1, NULL) != pdPASS)
  {
    while (1);
  }
  if (xTaskCreate(led_task, "redLED", 512, &red_params, configMAX_PRIORITIES - 1, NULL) != pdPASS)
  {
    while (1);
  }
  if (xTaskCreate(led_task, "blueLED", 512, &blue_params, configMAX_PRIORITIES - 1, NULL) != pdPASS)
  {
    while (1);
  }
  if (xTaskCreate(io_task, "ioTask", 512, NULL, configMAX_PRIORITIES - 1, NULL) != pdPASS)
  {
    while (1);
  }

  vTaskStartScheduler();
  while (1);
}

void clock_init()
{
  /* By default HSI (16 MHz RC oscillator) is selected as system clock.
   * We want to use the HSE (8 MHz MCO from ST-LINK connected to OSC_IN)
   * through the PLL to get 100 MHz system clock.
   */

  // Enable power controller and set voltage scale mode 1
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  // Configure PLL dividers and multiplier
  /* Input to PLL should be 1-2 MHz (preferably 2 MHz). Choosing M=4 gives
   * us 8 MHz / 4 = 2 MHz.
   * The output of the PLL should be 100-438 MHz, so setting the feedback
   * multiplier to N=200 gives us 2 MHz * 200 = 400 MHz.
   * The system clock should be 100 MHz. Choosing P=4 gives us
   * 400 MHz / 4 = 100 MHz
   */
  RCC_OscInitTypeDef osc_init = {0};
  osc_init.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  osc_init.HSEState = RCC_HSE_BYPASS;
  osc_init.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  osc_init.PLL.PLLState = RCC_PLL_ON;
  osc_init.PLL.PLLM = 4;
  osc_init.PLL.PLLN = 200;
  osc_init.PLL.PLLP = RCC_PLLP_DIV4;
  osc_init.PLL.PLLQ = 8;
  if (HAL_RCC_OscConfig(&osc_init) != HAL_OK)
  {
    while (1);
  }

  /* Set PLL output as the source for the system clock.
   * Since APB1 clock must not be more than 50 MHz, set the PCKL1 divider to 2.
   */
  RCC_ClkInitTypeDef clock_init = {0};
  clock_init.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2 | RCC_CLOCKTYPE_HCLK;
  clock_init.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  clock_init.AHBCLKDivider = RCC_SYSCLK_DIV1;
  clock_init.APB1CLKDivider = RCC_HCLK_DIV2;
  clock_init.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&clock_init, FLASH_LATENCY_3) != HAL_OK) // Configure flash controller for 3V3 supply and 100 MHz -> 3 wait states
  {
    while (1);
  }
}
