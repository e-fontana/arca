/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body (Dashboard Inteligente 3 Estados)
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "lcd.h"
#include "GUI.h"
#include "icons.h"  // Nosso arquivo com as imagens convertidas
#include <stdio.h>  // Para formatar os textos (sprintf)
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi1;

/* USER CODE BEGIN PV */
DashboardData_t meu_dashboard;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

// ==============================================================================
// 1. MÁQUINA DE ESTADOS DA IHM
// ==============================================================================

// Estados para Temperatura, Pressão e CC1101 (3 Níveis)
typedef enum {
    ESTADO_PADRAO = 0,
    ESTADO_BOM    = 1,
    ESTADO_RUIM   = 2
} EstadoSensor_t;

// Estados para NFC (2 Níveis)
typedef enum {
    DESCONECTADO = 0,
    CONECTADO    = 1
} EstadoConexao_t;

// Estrutura do Objeto Visual (Ícone)
typedef struct {
    uint16_t x;
    uint16_t y;
    uint16_t width;
    uint16_t height;
    uint8_t current_state;
    uint8_t last_state;
    const unsigned char **images;
} UI_Icon_t;

// ==============================================================================
// 2. MAPEAMENTO DAS IMAGENS AOS ESTADOS
// ==============================================================================
// A ordem AQUI importa! O índice [0] é Padrão, [1] é Bom, [2] é Ruim.
const unsigned char* umidade_imgs[] = {gImage_umidade, gImage_umidade, gImage_umidade};
const unsigned char* nfc_imgs[]     = {gImage_nfc_desconectado, gImage_nfc_conectado};
const unsigned char* cc1101_imgs[]  = {gImage_cc1101_padrao, gImage_cc1101_bom, gImage_cc1101_ruim};
const unsigned char* temp_imgs[]    = {gImage_temperatura_padrao, gImage_temperatura_bom, gImage_temperatura_ruim};
const unsigned char* pressao_imgs[] = {gImage_pressao_padrao, gImage_pressao_bom, gImage_pressao_ruim};

// ==============================================================================
// 3. INSTANCIANDO OS ÍCONES NA TELA (Posições X e Y)
// ==============================================================================
UI_Icon_t icon_temp  = {10, 50,  32, 32, ESTADO_PADRAO, 255, temp_imgs};
UI_Icon_t icon_umid  = {10, 90,  32, 32, ESTADO_PADRAO, 255, umidade_imgs};
UI_Icon_t icon_press = {10, 130, 32, 32, ESTADO_PADRAO, 255, pressao_imgs};
UI_Icon_t icon_nfc   = {10, 180, 32, 32, DESCONECTADO,  255, nfc_imgs};
UI_Icon_t icon_radio = {10, 230, 32, 32, ESTADO_PADRAO, 255, cc1101_imgs};

// ==============================================================================
// 4. MOTOR GRÁFICO (RENDERIZADOR)
// ==============================================================================

// Função Base de Desenho SPI (Agora recebe o Array de 8 bits gerado pelo LVGL)
void Draw_Image(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const unsigned char *image)
{
    LCD_SetWindows(x, y, x + width - 1, y + height - 1);
    LCD_CS_CLR;
    LCD_RS_SET;
    HAL_SPI_Transmit(&hspi1, (uint8_t *)image, width * height * 2, 1000);
    LCD_CS_SET;
}

// Atualiza o ícone apenas se o estado mudar (Economiza muito processamento!)
void UI_Render_Icon(UI_Icon_t *icon) {
    if (icon->current_state != icon->last_state) {
        Draw_Image(icon->x, icon->y, icon->width, icon->height, icon->images[icon->current_state]);
        icon->last_state = icon->current_state; // Salva na memória o estado atual
    }
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_SPI1_Init();
  /* USER CODE BEGIN 2 */

  // Ligar o backlight do Display e Inicializar Hardware
  HAL_GPIO_WritePin(IHM_LED_GPIO_Port, IHM_LED_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOA, IHM_RESET_Pin, GPIO_PIN_SET);
  HAL_Delay(50);
  LCD_Init();

  // Valores iniciais do sistema
  meu_dashboard.temperatura_val = 24.5;
  meu_dashboard.temp_estado = TEMP_PADRAO;
  meu_dashboard.umidade_val = 60.0;
  meu_dashboard.pressao_val = 1013;
  meu_dashboard.pressao_estado = PRESSAO_PADRAO;
  
  meu_dashboard.porta_estado = PORTA_TRANCADA;
  meu_dashboard.nfc_estado = NFC_DESCONECTADO;
  meu_dashboard.cc1101_estado = CC1101_BOM; // Simulando online
  
  sprintf(meu_dashboard.room_number, "---");
  sprintf(meu_dashboard.nfc_id, "---");

  // Desenha a estrutura fixa da tela (uma vez só)
  GUI_InitDashboard();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

    // =========================================================
    // ETAPA A: SIMULAÇÃO DOS SENSORES (Para teste)
    // =========================================================
    meu_dashboard.temperatura_val += 1.5; 
    if(meu_dashboard.temperatura_val > 35.0) meu_dashboard.temperatura_val = 15.0; 

    // Lógica da Temperatura
    if(meu_dashboard.temperatura_val >= 20.0 && meu_dashboard.temperatura_val <= 28.0) {
        meu_dashboard.temp_estado = TEMP_BOM;
    } else if(meu_dashboard.temperatura_val > 28.0) {
        meu_dashboard.temp_estado = TEMP_RUIM;
    } else {
        meu_dashboard.temp_estado = TEMP_PADRAO;
    }

    // Alterna a Porta a cada ciclo (Simulando aproximação do Cartão)
    static int toggle = 0;
    toggle = !toggle;
    
    if(toggle) {
        meu_dashboard.porta_estado = PORTA_ABERTA;
        meu_dashboard.nfc_estado = NFC_CONECTADO;
        sprintf(meu_dashboard.room_number, "101");
        sprintf(meu_dashboard.nfc_id, "AB-12-CD-34");
    } else {
        meu_dashboard.porta_estado = PORTA_TRANCADA;
        meu_dashboard.nfc_estado = NFC_DESCONECTADO;
        sprintf(meu_dashboard.room_number, "---");
        sprintf(meu_dashboard.nfc_id, "---");
    }

    // =========================================================
    // ETAPA B: RENDERIZAÇÃO NA TELA
    // =========================================================
    
    // Manda o Dashboard atualizar apenas o que mudou na struct
    GUI_UpdateDashboard(&meu_dashboard);

    // Pisca LED da placa (opcional)
    HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);

    // Delay de 1 segundo para conseguirmos ver a animação na tela
    HAL_Delay(1000);

  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(IHM_LED_GPIO_Port, IHM_LED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, IHM_RESET_Pin|IHM_CS_Pin|IHM_DC_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : LED_Pin */
  GPIO_InitStruct.Pin = LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : IHM_LED_Pin */
  GPIO_InitStruct.Pin = IHM_LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(IHM_LED_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : IHM_RESET_Pin IHM_CS_Pin IHM_DC_Pin */
  GPIO_InitStruct.Pin = IHM_RESET_Pin|IHM_CS_Pin|IHM_DC_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
