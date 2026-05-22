################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/Modules/RTC/rtc_api.c \
../Core/Src/Modules/RTC/rtc_sync.c 

OBJS += \
./Core/Src/Modules/RTC/rtc_api.o \
./Core/Src/Modules/RTC/rtc_sync.o 

C_DEPS += \
./Core/Src/Modules/RTC/rtc_api.d \
./Core/Src/Modules/RTC/rtc_sync.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/Modules/RTC/%.o Core/Src/Modules/RTC/%.su Core/Src/Modules/RTC/%.cyclo: ../Core/Src/Modules/RTC/%.c Core/Src/Modules/RTC/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F411xE -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-Modules-2f-RTC

clean-Core-2f-Src-2f-Modules-2f-RTC:
	-$(RM) ./Core/Src/Modules/RTC/rtc_api.cyclo ./Core/Src/Modules/RTC/rtc_api.d ./Core/Src/Modules/RTC/rtc_api.o ./Core/Src/Modules/RTC/rtc_api.su ./Core/Src/Modules/RTC/rtc_sync.cyclo ./Core/Src/Modules/RTC/rtc_sync.d ./Core/Src/Modules/RTC/rtc_sync.o ./Core/Src/Modules/RTC/rtc_sync.su

.PHONY: clean-Core-2f-Src-2f-Modules-2f-RTC

