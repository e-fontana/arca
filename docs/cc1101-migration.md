# Migração CC1101: implementação atual → suleymaneskil/CC1101_STM32_Library

## Contexto

A implementação atual do driver CC1101 foi escrita manualmente com API prefixada `CC1101_*`. O objetivo é substituí-la pela biblioteca do repositório [suleymaneskil/CC1101_STM32_Library](https://github.com/suleymaneskil/CC1101_STM32_Library), que usa prefixos `TI_*` para funções e `CCxxx0_*` para registradores. A biblioteca foi originalmente escrita para STM32L4 — as adaptações necessárias para o STM32F401CEUx deste projeto são mínimas.

---

## Diferenças entre as implementações

| Aspecto | Atual | Biblioteca GitHub |
|---|---|---|
| Prefixo de funções | `CC1101_*` | `TI_*` |
| Prefixo de registradores | `CC1101_*` | `CCxxx0_*` (mesmos valores numéricos) |
| HAL include | `stm32f4xx_hal.h` | `stm32l4xx_hal.h` → **adaptar para F4** |
| SPI transfer | `HAL_SPI_TransmitReceive` (uma chamada) | `HAL_SPI_Transmit` + `HAL_SPI_Receive` separados |
| Delays | `_DelayCycles` (busy-loop por ciclos) | DWT via `dw_stm32_delay` (precisão em µs) |
| Init | `CC1101_Init(hspi)` — tudo-em-um | `Power_up_reset()` + `TI_init(hspi, cs_port, cs_pin)` — dois passos |
| Recepção | `CC1101_ReceivePacket(CC1101_Packet_t*)` — retorna struct com RSSI/LQI | `TI_receive_packet(buf, *len)` — retorna CRC_OK |
| Envio | `CC1101_SendPacket` — polling GDO0, ativa SRX após TX | `TI_send_packet` — sem polling, sem SRX automático |
| Pino MISO (hardcoded) | `GPIOA, GPIO_PIN_6` | `GPIOA, GPIO_PIN_6` — **mesmo hardware, sem conflito** |

---

## Análise de pinagem: biblioteca vs. projeto

O projeto original da biblioteca usa um **STM32L432KCUx** com pinagem diferente da nossa:

| Sinal | Biblioteca (L432) | Nosso projeto (F401) |
|---|---|---|
| SPI1_SCK | PA1 | PA5 |
| SPI1_MISO | PA6 | PA6 ✓ |
| SPI1_MOSI | PA7 | PA7 ✓ |
| CS (NSS) | PA5 | PA10 (`NSS_CC1101_Pin`) |
| GDO0 | PB0 | PB15 (`INT_CC1101_Pin`) |

SCK em pinos diferentes não afeta o software (configurado pelo CubeMX/HAL). CS e GDO0 são passados como parâmetros ou tratados via macros de `main.h` — sem conflito. O MISO em PA6 é o único pino hardcoded no código da biblioteca e coincide com o nosso hardware.

### Bug na biblioteca: `Power_up_reset()` usa três globais não inicializados

`cc1101.c` declara globais de módulo inicializados **somente dentro de `TI_init()`**:

```c
SPI_HandleTypeDef* hal_spi;   // usado em TI_strobe → __spi_write → HAL_SPI_Transmit
uint16_t           CS_Pin;
GPIO_TypeDef*      CS_GPIO_Port;
```

Porém o `main.c` da própria biblioteca chama:

```c
Power_up_reset();                        // hal_spi, CS_GPIO_Port e CS_Pin ainda são NULL/0 !
TI_init(&hspi1, CS_GPIO_Port, CS_Pin);  // CS_GPIO_Port/CS_Pin aqui são macros do main.h gerado
```

Dentro de `Power_up_reset()` (escopo de `cc1101.c`) os três identificadores resolvem para as **variáveis globais**, não para as macros. Resultado: `HAL_SPI_Transmit(NULL, ...)` e `HAL_GPIO_WritePin(NULL, 0, ...)` — hard fault garantido.

### Correção: modificar `Power_up_reset()` para receber os parâmetros

Em vez de depender dos globais, `Power_up_reset()` deve inicializá-los ela mesma:

**`cc1101.h`** — alterar assinatura:
```c
// antes:
void Power_up_reset();

// depois:
void Power_up_reset(SPI_HandleTypeDef* hspi, GPIO_TypeDef* cs_port, uint16_t cs_pin);
```

**`cc1101.c`** — alterar implementação:
```c
void Power_up_reset(SPI_HandleTypeDef* hspi, GPIO_TypeDef* cs_port, uint16_t cs_pin)
{
    hal_spi      = hspi;
    CS_GPIO_Port = cs_port;
    CS_Pin       = cs_pin;

    DWT_Delay_Init();
    // ... restante inalterado
}
```

**Ordem de inicialização no `main.c`** (reset antes de aplicar settings — correto):
```c
Power_up_reset(&hspi1, NSS_CC1101_GPIO_Port, NSS_CC1101_Pin); // reset com globais válidos
TI_init(&hspi1, NSS_CC1101_GPIO_Port, NSS_CC1101_Pin);        // aplica toda a configuração
TI_write_reg(CCxxx0_MCSM1, 0x30);
```

---

## Arquivos a modificar / criar

### 1. `Core/Inc/cc1101.h` — substituir

Conteúdo: cópia da biblioteca com as seguintes adaptações:

- Trocar `#include "stm32l4xx_hal.h"` por `#include "stm32f4xx_hal.h"`
- Remover a guarda `#ifndef __STM32L4xx_HAL_H`
- Adicionar guardas `#ifndef` em cada typedef para evitar conflito com outros headers do projeto:
  ```c
  #ifndef BOOL
  typedef unsigned char BOOL;
  #endif
  #ifndef BYTE
  typedef unsigned char BYTE;
  #endif
  // ... idem para WORD, DWORD, UINT8, UINT16, UINT32, INT8, INT16, INT32
  ```
- Corrigir assinatura de `Power_up_reset()` (ver seção acima)
- Remover as declarações sem implementação: `TI_write_burst_reg_c` e `get_random_byte`

### 2. `Core/Src/Modules/Communication/cc1101.c` — substituir

Conteúdo: cópia da biblioteca com as seguintes adaptações:

- Corrigir implementação de `Power_up_reset()` para receber os três parâmetros e inicializar os globais (ver seção acima)
- O include do HAL já chega via `cc1101.h` — nenhuma outra alteração necessária

### 3. `Core/Inc/dw_stm32_delay.h` — criar (novo arquivo)

Utilitário de delay por DWT requerido por `cc1101.c`. Adaptações:

- Trocar `#include "stm32l4xx_hal.h"` por `#include "stm32f4xx_hal.h"`
- Remover a guarda `#ifndef __STM32L4xx_HAL_H`

### 4. `Core/Src/dw_stm32_delay.c` — criar (novo arquivo)

Implementação do utilitário de delay. Conteúdo exato da biblioteca, sem alterações.

### 5. `CMakeLists.txt` — modificar

Adicionar `dw_stm32_delay.c` à lista de fontes em `target_sources`:

```cmake
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    Core/Src/Modules/nfc.c
    Core/Src/Modules/temp.c
    Core/Src/Modules/press.c
    Core/Src/Modules/Communication/cc1101.c
    Core/Src/dw_stm32_delay.c   # novo
)
```

### 6. `Core/Src/main.c` — modificar (apenas seções USER CODE)

**Variáveis (USER CODE BEGIN PV):**

```c
// remover:
volatile uint8_t cc1101_rx_flag;
CC1101_Packet_t  rx_packet;

// adicionar:
uint8_t rx_buf[64];
uint8_t rx_len;
```

**Inicialização (USER CODE BEGIN 2):**

```c
// antes:
uint8_t init_ok = CC1101_Init(&hspi1);
CC1101_WriteReg(CC1101_MCSM1, 0x30);

// depois (ver seção "Correção: Power_up_reset()"):
Power_up_reset(&hspi1, NSS_CC1101_GPIO_Port, NSS_CC1101_Pin); // reset com globais válidos
TI_init(&hspi1, NSS_CC1101_GPIO_Port, NSS_CC1101_Pin);        // aplica toda a configuração
TI_write_reg(CCxxx0_MCSM1, 0x30);
```

**Testes de estado (debug, USER CODE BEGIN 2 continuação):**

| Antes | Depois |
|---|---|
| `CC1101_ReadStatusReg(0x35)` | `TI_read_status(CCxxx0_MARCSTATE)` |
| `CC1101_ReadStatusReg(0x3B)` | `TI_read_status(CCxxx0_RXBYTES)` |
| `CC1101_Strobe(CC1101_SIDLE)` | `TI_strobe(CCxxx0_SIDLE)` |
| `CC1101_Strobe(CC1101_SRX)` | `TI_strobe(CCxxx0_SRX)` |

**Teste 4 — acesso direto ao SPI (SNOP):**

```c
// CC1101_CS_LOW()  → HAL_GPIO_WritePin(NSS_CC1101_GPIO_Port, NSS_CC1101_Pin, GPIO_PIN_RESET)
// CC1101_CS_HIGH() → HAL_GPIO_WritePin(NSS_CC1101_GPIO_Port, NSS_CC1101_Pin, GPIO_PIN_SET)
// CC1101_SNOP      → CCxxx0_SNOP
```

**Loop `while(1)`:**

```c
// CC1101_GDO0_PORT, CC1101_GDO0_PIN → INT_CC1101_GPIO_Port, INT_CC1101_Pin  (de main.h)
// CC1101_ReadStatusReg(0x35)         → TI_read_status(CCxxx0_MARCSTATE)
// CC1101_ReadStatusReg(0x3B)         → TI_read_status(CCxxx0_RXBYTES)
```

**Callback EXTI:** sem alteração — apenas seta flag por `INT_CC1101_Pin`.

---

## Como enviar e receber dados com a nova API

### Envio de pacote

A função de envio **não bloqueia** e não monitora o GDO0 internamente. O padrão correto é:

```c
// 1. Preparar o buffer
uint8_t payload[] = {0x01, 0x02, 0x03, 0xAA};

// 2. Garantir IDLE e limpar TX FIFO antes de enviar
TI_strobe(CCxxx0_SIDLE);
TI_strobe(CCxxx0_SFTX);

// 3. Enviar — coloca dados no TX FIFO e dispara STX
TI_send_packet(payload, sizeof(payload));

// 4. Aguardar conclusão via GDO0 (configurado como IOCFG0=0x06: assert no início da transmissão,
//    deassert ao final). GDO0 = INT_CC1101_Pin = PB15 no nosso hardware.
while (!HAL_GPIO_ReadPin(INT_CC1101_GPIO_Port, INT_CC1101_Pin)); // espera início
while ( HAL_GPIO_ReadPin(INT_CC1101_GPIO_Port, INT_CC1101_Pin)); // espera fim do TX

// 5. Voltar ao modo RX manualmente (TI_send_packet não faz isso)
TI_strobe(CCxxx0_SRX);
```

> **Atenção:** os dois `while` de polling bloqueiam a CPU. Se o módulo não responder (ausência de cristal, problema de SPI), o código trava. Para uso em produção, adicionar timeout por DWT.

---

### Recepção de pacote (modo interrupção — padrão do projeto)

O projeto usa EXTI15 (PB15 = GDO0) para detectar chegada de pacote. O fluxo completo é:

**Variáveis globais em `main.c`:**
```c
volatile uint8_t gdo0_flag = 0;
uint8_t rx_buf[64];
uint8_t rx_len;
```

**Callback EXTI em `main.c` (USER CODE BEGIN 4):**
```c
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == INT_CC1101_Pin)
        gdo0_flag = 1;
}
```

**Habilitação do EXTI e entrada em RX (após o init):**
```c
// Limpar pendências e habilitar interrupção
__HAL_GPIO_EXTI_CLEAR_IT(INT_CC1101_Pin);
NVIC_ClearPendingIRQ(EXTI15_10_IRQn);
HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

// Entrar em modo RX
TI_strobe(CCxxx0_SFRX);
TI_strobe(CCxxx0_SRX);
```

**Loop de recepção em `while(1)`:**
```c
if (gdo0_flag) {
    gdo0_flag = 0;

    // Verificar se há bytes válidos no FIFO (bit 7 = overflow)
    uint8_t rxbytes = TI_read_status(CCxxx0_RXBYTES);
    if (!(rxbytes & 0x7F)) {
        // FIFO vazio ou overflow — limpar e reiniciar RX
        TI_strobe(CCxxx0_SIDLE);
        TI_strobe(CCxxx0_SFRX);
        TI_strobe(CCxxx0_SRX);
        continue; // ou goto / flag para próxima iteração
    }

    // Verificar CRC via LQI antes de ler o FIFO
    uint8_t lqi = TI_read_status(CCxxx0_LQI);
    if (lqi & 0x80 /* CRC_OK */) {
        rx_len = sizeof(rx_buf);
        if (TI_receive_packet(rx_buf, &rx_len)) {
            // rx_buf[0..rx_len-1] contém o payload recebido
            // rx_len foi atualizado com o tamanho real

            // RSSI (opcional) — ler separadamente
            uint8_t rssi_raw = TI_read_status(CCxxx0_RSSI);
            int8_t rssi_dbm = (rssi_raw >= 128)
                ? (int8_t)((rssi_raw - 256) / 2) - 74
                : (int8_t)(rssi_raw / 2) - 74;
        }
    }

    // Reiniciar RX para o próximo pacote
    TI_strobe(CCxxx0_SFRX);
    TI_strobe(CCxxx0_SRX);
}
```

> **Nota sobre `TI_receive_packet`:** o segundo parâmetro `&rx_len` serve como entrada **e** saída. Passe o tamanho máximo do buffer na entrada; a função sobrescreve com o tamanho real recebido. Se o pacote for maior que o buffer, a função faz flush do FIFO e retorna `FALSE`.

---

## Mudanças comportamentais a observar

- **Frequência RF**: a biblioteca configura FREQ1=`0xB4` / FREQ0=`0x2E` (~433.0 MHz, 1.2 kbps) versus a implementação atual (FREQ1=`0xA7` / FREQ0=`0x62`, 4.8 kbps). Os valores podem ser ajustados no `TI_write_settings()` após a migração se necessário.
- **Conclusão do TX**: `TI_send_packet` não faz polling do GDO0 nem reativa o RX após o envio. O chamador deve chamar `TI_strobe(CCxxx0_SRX)` manualmente para voltar ao modo recepção.
- **RSSI/LQI na recepção**: `TI_receive_packet` não devolve RSSI/LQI na struct. Para obter o RSSI, usar `TI_read_status(CCxxx0_RSSI)` separadamente.

---

## Verificação pós-implementação

1. **Build limpo**: `cmake --build Debug`
2. **Flash + UART**: saída esperada na serial (115200 baud):
   ```
   [1] STATE=1 (IDLE após init)
   [2] Apos SIDLE STATE=1
   [3] Apos SRX+10ms STATE=13 (RX)
   [4] SNOP status=0x... chip_state=1
   GDO0=0 STATE=0x0D RXBYTES=0   ← loop em stand-by com RX ativo
   ```
3. **Recepção**: enviar um pacote de outro nó CC1101 e verificar que `RXBYTES > 0` e o callback EXTI é disparado.
