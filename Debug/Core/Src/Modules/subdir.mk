################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/Modules/dht11.c \
../Core/Src/Modules/flash_db.c \
../Core/Src/Modules/fsm.c \
../Core/Src/Modules/nfc.c \
../Core/Src/Modules/press.c 

OBJS += \
./Core/Src/Modules/dht11.o \
./Core/Src/Modules/flash_db.o \
./Core/Src/Modules/fsm.o \
./Core/Src/Modules/nfc.o \
./Core/Src/Modules/press.o 

C_DEPS += \
./Core/Src/Modules/dht11.d \
./Core/Src/Modules/flash_db.d \
./Core/Src/Modules/fsm.d \
./Core/Src/Modules/nfc.d \
./Core/Src/Modules/press.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/Modules/%.o Core/Src/Modules/%.su Core/Src/Modules/%.cyclo: ../Core/Src/Modules/%.c Core/Src/Modules/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F411xE -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-Modules

clean-Core-2f-Src-2f-Modules:
	-$(RM) ./Core/Src/Modules/dht11.cyclo ./Core/Src/Modules/dht11.d ./Core/Src/Modules/dht11.o ./Core/Src/Modules/dht11.su ./Core/Src/Modules/flash_db.cyclo ./Core/Src/Modules/flash_db.d ./Core/Src/Modules/flash_db.o ./Core/Src/Modules/flash_db.su ./Core/Src/Modules/fsm.cyclo ./Core/Src/Modules/fsm.d ./Core/Src/Modules/fsm.o ./Core/Src/Modules/fsm.su ./Core/Src/Modules/nfc.cyclo ./Core/Src/Modules/nfc.d ./Core/Src/Modules/nfc.o ./Core/Src/Modules/nfc.su ./Core/Src/Modules/press.cyclo ./Core/Src/Modules/press.d ./Core/Src/Modules/press.o ./Core/Src/Modules/press.su

.PHONY: clean-Core-2f-Src-2f-Modules

