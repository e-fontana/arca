################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/CMSIS/DAP/Firmware/Source/DAP.c \
../Drivers/CMSIS/DAP/Firmware/Source/DAP_vendor.c \
../Drivers/CMSIS/DAP/Firmware/Source/JTAG_DP.c \
../Drivers/CMSIS/DAP/Firmware/Source/SWO.c \
../Drivers/CMSIS/DAP/Firmware/Source/SW_DP.c \
../Drivers/CMSIS/DAP/Firmware/Source/UART.c 

OBJS += \
./Drivers/CMSIS/DAP/Firmware/Source/DAP.o \
./Drivers/CMSIS/DAP/Firmware/Source/DAP_vendor.o \
./Drivers/CMSIS/DAP/Firmware/Source/JTAG_DP.o \
./Drivers/CMSIS/DAP/Firmware/Source/SWO.o \
./Drivers/CMSIS/DAP/Firmware/Source/SW_DP.o \
./Drivers/CMSIS/DAP/Firmware/Source/UART.o 

C_DEPS += \
./Drivers/CMSIS/DAP/Firmware/Source/DAP.d \
./Drivers/CMSIS/DAP/Firmware/Source/DAP_vendor.d \
./Drivers/CMSIS/DAP/Firmware/Source/JTAG_DP.d \
./Drivers/CMSIS/DAP/Firmware/Source/SWO.d \
./Drivers/CMSIS/DAP/Firmware/Source/SW_DP.d \
./Drivers/CMSIS/DAP/Firmware/Source/UART.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/CMSIS/DAP/Firmware/Source/%.o Drivers/CMSIS/DAP/Firmware/Source/%.su Drivers/CMSIS/DAP/Firmware/Source/%.cyclo: ../Drivers/CMSIS/DAP/Firmware/Source/%.c Drivers/CMSIS/DAP/Firmware/Source/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -DUSE_HAL_DRIVER -DSTM32F411xE -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-CMSIS-2f-DAP-2f-Firmware-2f-Source

clean-Drivers-2f-CMSIS-2f-DAP-2f-Firmware-2f-Source:
	-$(RM) ./Drivers/CMSIS/DAP/Firmware/Source/DAP.cyclo ./Drivers/CMSIS/DAP/Firmware/Source/DAP.d ./Drivers/CMSIS/DAP/Firmware/Source/DAP.o ./Drivers/CMSIS/DAP/Firmware/Source/DAP.su ./Drivers/CMSIS/DAP/Firmware/Source/DAP_vendor.cyclo ./Drivers/CMSIS/DAP/Firmware/Source/DAP_vendor.d ./Drivers/CMSIS/DAP/Firmware/Source/DAP_vendor.o ./Drivers/CMSIS/DAP/Firmware/Source/DAP_vendor.su ./Drivers/CMSIS/DAP/Firmware/Source/JTAG_DP.cyclo ./Drivers/CMSIS/DAP/Firmware/Source/JTAG_DP.d ./Drivers/CMSIS/DAP/Firmware/Source/JTAG_DP.o ./Drivers/CMSIS/DAP/Firmware/Source/JTAG_DP.su ./Drivers/CMSIS/DAP/Firmware/Source/SWO.cyclo ./Drivers/CMSIS/DAP/Firmware/Source/SWO.d ./Drivers/CMSIS/DAP/Firmware/Source/SWO.o ./Drivers/CMSIS/DAP/Firmware/Source/SWO.su ./Drivers/CMSIS/DAP/Firmware/Source/SW_DP.cyclo ./Drivers/CMSIS/DAP/Firmware/Source/SW_DP.d ./Drivers/CMSIS/DAP/Firmware/Source/SW_DP.o ./Drivers/CMSIS/DAP/Firmware/Source/SW_DP.su ./Drivers/CMSIS/DAP/Firmware/Source/UART.cyclo ./Drivers/CMSIS/DAP/Firmware/Source/UART.d ./Drivers/CMSIS/DAP/Firmware/Source/UART.o ./Drivers/CMSIS/DAP/Firmware/Source/UART.su

.PHONY: clean-Drivers-2f-CMSIS-2f-DAP-2f-Firmware-2f-Source

