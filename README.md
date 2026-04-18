# Lab 2 — STM32F411CEUx (Black Pill)

Projeto de firmware para o microcontrolador **STM32F411CEUx** (placa Black Pill), desenvolvido como atividade prática da disciplina de laboratório da UFBA.

## O que o firmware faz

O programa inicializa os periféricos configurados via STM32CubeMX (ADC1, RTC, SPI1 e GPIO) e executa um loop infinito que **pisca o LED onboard** conectado ao pino **PC13**.

## Hardware necessário

| Item | Descrição |
|------|-----------|
| STM32F411CEUx | Placa Black Pill (WeAct ou similar) |
| ST-Link v2 | Programador/depurador (clone USB funciona) |
| Cabo SWD | 4 pinos: SWDIO, SWDCLK, GND, 3.3 V |

Conexão SWD entre ST-Link e Black Pill:

```
ST-Link    →    Black Pill
SWDIO      →    DIO
SWDCLK     →    CLK
GND        →    GND
3.3V       →    3V3
```

---

## Pré-requisitos — Arch Linux

### 1. Pacotes do sistema

```bash
sudo pacman -S git cmake ninja arm-none-eabi-gcc arm-none-eabi-binutils arm-none-eabi-newlib
```

> O pacote `arm-none-eabi-newlib` fornece a libc bare-metal (nano.specs) usada pelo linker.

---

## Extensões do VSCode

Instale as extensões abaixo. O atalho para abrir o painel de extensões é `Ctrl+Shift+X`.

| Extensão | ID | Finalidade |
|----------|----|-----------|
| STM32 VS Code Extension | `stmicroelectronics.stm32-vscode-extension` | Build, IntelliSense (clangd ARM) e debug via ST-Link |
| CMake Tools | `ms-vscode.cmake-tools` | Integração do CMake com a IDE |
| C/C++ Extension Pack | `ms-vscode.cpptools-extension-pack` | Suporte base a C/C++ |

> A extensão da STMicroelectronics baixa automaticamente um bundle próprio com `cmake`, `clangd` e o GDB server para ST-Link. O `CUBE_BUNDLE_PATH` é definido por ela.

Instalação via linha de comando (alternativa):

```bash
code --install-extension stmicroelectronics.stm32-vscode-extension
code --install-extension ms-vscode.cmake-tools
code --install-extension ms-vscode.cpptools-extension-pack
```

---

## Clonando e abrindo o projeto

```bash
git clone <URL_DO_REPOSITORIO>
cd arca
code .
```

Ao abrir o VSCode pela primeira vez, ele pode perguntar se deseja configurar o CMake — confirme com **Yes** ou use o preset `Debug` quando solicitado.

---

## Build

### Via VSCode (recomendado)

1. Abra a paleta de comandos: `Ctrl+Shift+P`
2. Execute **CMake: Select Configure Preset** → escolha `Debug`
3. Execute **CMake: Build** (ou pressione `F7`)

O binário gerado fica em `build/Debug/teste.elf`.

### Via terminal (build manual)

```bash
cmake --preset Debug
cmake --build --preset Debug
```

---

## Flash e debug

### Flash + debug com ST-Link (VSCode)

1. Conecte o ST-Link à placa via cabo SWD e ao PC via USB
2. Vá em **Run → Start Debugging** (ou pressione `F5`)
3. A configuração `STM32Cube: Launch ST-Link GDB Server` em `.vscode/launch.json` compila, grava e inicia a sessão de debug automaticamente

O programa para no `main()` aguardando `F5` para continuar.

### Flash via terminal com OpenOCD (alternativa)

```bash
# Instalar openocd
sudo pacman -S openocd

# Gravar o .elf
openocd -f interface/stlink.cfg \
        -f target/stm32f4x.cfg \
        -c "program build/Debug/teste.elf verify reset exit"
```

---

## Estrutura do projeto

```
arca/
├── Core/
│   ├── Inc/          # Headers da aplicação
│   └── Src/          # Fontes: main.c, inicialização de periféricos
├── Drivers/
│   ├── CMSIS/        # Headers CMSIS (Cortex-M)
│   └── STM32F4xx_HAL_Driver/  # HAL da ST para STM32F4
├── cmake/
│   ├── gcc-arm-none-eabi.cmake   # Toolchain cross-compiler
│   └── stm32cubemx/CMakeLists.txt # Fontes e includes gerados pelo CubeMX
├── CMakeLists.txt    # CMake principal
├── CMakePresets.json # Presets Debug / Release
├── STM32F411XX_FLASH.ld  # Linker script (flash)
├── teste.ioc         # Arquivo de configuração do STM32CubeMX
└── .vscode/          # Configurações da IDE (build, debug, IntelliSense)
```

---

## Periféricos configurados

| Periférico | Configuração |
|------------|-------------|
| **GPIO PC13** | Saída push-pull — LED onboard |
| **ADC1** | Canal 4, resolução 12 bits, conversão por software |
| **RTC** | Formato 24h, clock LSE, saída desabilitada |
| **SPI1** | Master, full-duplex, 8 bits, CPOL=0/CPHA=0, NSS soft |

Clock do sistema: **84 MHz** (HSI 16 MHz × PLL → 84 MHz via PLLCLK).

---

## Solução de problemas

**IntelliSense com erros após o clone**

O arquivo `build/Debug/compile_commands.json` é gerado pelo CMake. Faça o build ao menos uma vez para que o clangd encontre os includes corretos.

**`cube-cmake` não encontrado**

Verifique se a extensão STM32 VS Code Extension terminou o download do bundle. Aguarde a barra de status do VSCode indicar conclusão, depois reabra o projeto.

**ST-Link não reconhecido**

```bash
# Adicionar regras udev para ST-Link
sudo pacman -S stlink
sudo udevadm control --reload-rules
sudo udevadm trigger
```

Reconecte o ST-Link após aplicar as regras.
