################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/CMSIS/NN/Source/PoolingFunctions/arm_avgpool_s16.c \
../Drivers/CMSIS/NN/Source/PoolingFunctions/arm_avgpool_s8.c \
../Drivers/CMSIS/NN/Source/PoolingFunctions/arm_max_pool_s16.c \
../Drivers/CMSIS/NN/Source/PoolingFunctions/arm_max_pool_s8.c \
../Drivers/CMSIS/NN/Source/PoolingFunctions/arm_pool_q7_HWC.c 

OBJS += \
./Drivers/CMSIS/NN/Source/PoolingFunctions/arm_avgpool_s16.o \
./Drivers/CMSIS/NN/Source/PoolingFunctions/arm_avgpool_s8.o \
./Drivers/CMSIS/NN/Source/PoolingFunctions/arm_max_pool_s16.o \
./Drivers/CMSIS/NN/Source/PoolingFunctions/arm_max_pool_s8.o \
./Drivers/CMSIS/NN/Source/PoolingFunctions/arm_pool_q7_HWC.o 

C_DEPS += \
./Drivers/CMSIS/NN/Source/PoolingFunctions/arm_avgpool_s16.d \
./Drivers/CMSIS/NN/Source/PoolingFunctions/arm_avgpool_s8.d \
./Drivers/CMSIS/NN/Source/PoolingFunctions/arm_max_pool_s16.d \
./Drivers/CMSIS/NN/Source/PoolingFunctions/arm_max_pool_s8.d \
./Drivers/CMSIS/NN/Source/PoolingFunctions/arm_pool_q7_HWC.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/CMSIS/NN/Source/PoolingFunctions/%.o Drivers/CMSIS/NN/Source/PoolingFunctions/%.su Drivers/CMSIS/NN/Source/PoolingFunctions/%.cyclo: ../Drivers/CMSIS/NN/Source/PoolingFunctions/%.c Drivers/CMSIS/NN/Source/PoolingFunctions/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F411xE -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-CMSIS-2f-NN-2f-Source-2f-PoolingFunctions

clean-Drivers-2f-CMSIS-2f-NN-2f-Source-2f-PoolingFunctions:
	-$(RM) ./Drivers/CMSIS/NN/Source/PoolingFunctions/arm_avgpool_s16.cyclo ./Drivers/CMSIS/NN/Source/PoolingFunctions/arm_avgpool_s16.d ./Drivers/CMSIS/NN/Source/PoolingFunctions/arm_avgpool_s16.o ./Drivers/CMSIS/NN/Source/PoolingFunctions/arm_avgpool_s16.su ./Drivers/CMSIS/NN/Source/PoolingFunctions/arm_avgpool_s8.cyclo ./Drivers/CMSIS/NN/Source/PoolingFunctions/arm_avgpool_s8.d ./Drivers/CMSIS/NN/Source/PoolingFunctions/arm_avgpool_s8.o ./Drivers/CMSIS/NN/Source/PoolingFunctions/arm_avgpool_s8.su ./Drivers/CMSIS/NN/Source/PoolingFunctions/arm_max_pool_s16.cyclo ./Drivers/CMSIS/NN/Source/PoolingFunctions/arm_max_pool_s16.d ./Drivers/CMSIS/NN/Source/PoolingFunctions/arm_max_pool_s16.o ./Drivers/CMSIS/NN/Source/PoolingFunctions/arm_max_pool_s16.su ./Drivers/CMSIS/NN/Source/PoolingFunctions/arm_max_pool_s8.cyclo ./Drivers/CMSIS/NN/Source/PoolingFunctions/arm_max_pool_s8.d ./Drivers/CMSIS/NN/Source/PoolingFunctions/arm_max_pool_s8.o ./Drivers/CMSIS/NN/Source/PoolingFunctions/arm_max_pool_s8.su ./Drivers/CMSIS/NN/Source/PoolingFunctions/arm_pool_q7_HWC.cyclo ./Drivers/CMSIS/NN/Source/PoolingFunctions/arm_pool_q7_HWC.d ./Drivers/CMSIS/NN/Source/PoolingFunctions/arm_pool_q7_HWC.o ./Drivers/CMSIS/NN/Source/PoolingFunctions/arm_pool_q7_HWC.su

.PHONY: clean-Drivers-2f-CMSIS-2f-NN-2f-Source-2f-PoolingFunctions

