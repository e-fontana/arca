################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/Modules/Communication/cc1101.c \
../Core/Src/Modules/Communication/com.c \
../Core/Src/Modules/Communication/dw_stm32_delay.c \
../Core/Src/Modules/Communication/events.c \
../Core/Src/Modules/Communication/uart_protocol.c 

OBJS += \
./Core/Src/Modules/Communication/cc1101.o \
./Core/Src/Modules/Communication/com.o \
./Core/Src/Modules/Communication/dw_stm32_delay.o \
./Core/Src/Modules/Communication/events.o \
./Core/Src/Modules/Communication/uart_protocol.o 

C_DEPS += \
./Core/Src/Modules/Communication/cc1101.d \
./Core/Src/Modules/Communication/com.d \
./Core/Src/Modules/Communication/dw_stm32_delay.d \
./Core/Src/Modules/Communication/events.d \
./Core/Src/Modules/Communication/uart_protocol.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/Modules/Communication/%.o Core/Src/Modules/Communication/%.su Core/Src/Modules/Communication/%.cyclo: ../Core/Src/Modules/Communication/%.c Core/Src/Modules/Communication/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F411xE -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-Modules-2f-Communication

clean-Core-2f-Src-2f-Modules-2f-Communication:
	-$(RM) ./Core/Src/Modules/Communication/cc1101.cyclo ./Core/Src/Modules/Communication/cc1101.d ./Core/Src/Modules/Communication/cc1101.o ./Core/Src/Modules/Communication/cc1101.su ./Core/Src/Modules/Communication/com.cyclo ./Core/Src/Modules/Communication/com.d ./Core/Src/Modules/Communication/com.o ./Core/Src/Modules/Communication/com.su ./Core/Src/Modules/Communication/dw_stm32_delay.cyclo ./Core/Src/Modules/Communication/dw_stm32_delay.d ./Core/Src/Modules/Communication/dw_stm32_delay.o ./Core/Src/Modules/Communication/dw_stm32_delay.su ./Core/Src/Modules/Communication/events.cyclo ./Core/Src/Modules/Communication/events.d ./Core/Src/Modules/Communication/events.o ./Core/Src/Modules/Communication/events.su ./Core/Src/Modules/Communication/uart_protocol.cyclo ./Core/Src/Modules/Communication/uart_protocol.d ./Core/Src/Modules/Communication/uart_protocol.o ./Core/Src/Modules/Communication/uart_protocol.su

.PHONY: clean-Core-2f-Src-2f-Modules-2f-Communication

