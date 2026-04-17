################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/CMSIS/NN/Source/SoftmaxFunctions/arm_nn_softmax_common_s8.c \
../Drivers/CMSIS/NN/Source/SoftmaxFunctions/arm_softmax_q15.c \
../Drivers/CMSIS/NN/Source/SoftmaxFunctions/arm_softmax_q7.c \
../Drivers/CMSIS/NN/Source/SoftmaxFunctions/arm_softmax_s16.c \
../Drivers/CMSIS/NN/Source/SoftmaxFunctions/arm_softmax_s8.c \
../Drivers/CMSIS/NN/Source/SoftmaxFunctions/arm_softmax_s8_s16.c \
../Drivers/CMSIS/NN/Source/SoftmaxFunctions/arm_softmax_u8.c \
../Drivers/CMSIS/NN/Source/SoftmaxFunctions/arm_softmax_with_batch_q7.c 

OBJS += \
./Drivers/CMSIS/NN/Source/SoftmaxFunctions/arm_nn_softmax_common_s8.o \
./Drivers/CMSIS/NN/Source/SoftmaxFunctions/arm_softmax_q15.o \
./Drivers/CMSIS/NN/Source/SoftmaxFunctions/arm_softmax_q7.o \
./Drivers/CMSIS/NN/Source/SoftmaxFunctions/arm_softmax_s16.o \
./Drivers/CMSIS/NN/Source/SoftmaxFunctions/arm_softmax_s8.o \
./Drivers/CMSIS/NN/Source/SoftmaxFunctions/arm_softmax_s8_s16.o \
./Drivers/CMSIS/NN/Source/SoftmaxFunctions/arm_softmax_u8.o \
./Drivers/CMSIS/NN/Source/SoftmaxFunctions/arm_softmax_with_batch_q7.o 

C_DEPS += \
./Drivers/CMSIS/NN/Source/SoftmaxFunctions/arm_nn_softmax_common_s8.d \
./Drivers/CMSIS/NN/Source/SoftmaxFunctions/arm_softmax_q15.d \
./Drivers/CMSIS/NN/Source/SoftmaxFunctions/arm_softmax_q7.d \
./Drivers/CMSIS/NN/Source/SoftmaxFunctions/arm_softmax_s16.d \
./Drivers/CMSIS/NN/Source/SoftmaxFunctions/arm_softmax_s8.d \
./Drivers/CMSIS/NN/Source/SoftmaxFunctions/arm_softmax_s8_s16.d \
./Drivers/CMSIS/NN/Source/SoftmaxFunctions/arm_softmax_u8.d \
./Drivers/CMSIS/NN/Source/SoftmaxFunctions/arm_softmax_with_batch_q7.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/CMSIS/NN/Source/SoftmaxFunctions/%.o Drivers/CMSIS/NN/Source/SoftmaxFunctions/%.su Drivers/CMSIS/NN/Source/SoftmaxFunctions/%.cyclo: ../Drivers/CMSIS/NN/Source/SoftmaxFunctions/%.c Drivers/CMSIS/NN/Source/SoftmaxFunctions/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -DUSE_HAL_DRIVER -DSTM32F411xE -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-CMSIS-2f-NN-2f-Source-2f-SoftmaxFunctions

clean-Drivers-2f-CMSIS-2f-NN-2f-Source-2f-SoftmaxFunctions:
	-$(RM) ./Drivers/CMSIS/NN/Source/SoftmaxFunctions/arm_nn_softmax_common_s8.cyclo ./Drivers/CMSIS/NN/Source/SoftmaxFunctions/arm_nn_softmax_common_s8.d ./Drivers/CMSIS/NN/Source/SoftmaxFunctions/arm_nn_softmax_common_s8.o ./Drivers/CMSIS/NN/Source/SoftmaxFunctions/arm_nn_softmax_common_s8.su ./Drivers/CMSIS/NN/Source/SoftmaxFunctions/arm_softmax_q15.cyclo ./Drivers/CMSIS/NN/Source/SoftmaxFunctions/arm_softmax_q15.d ./Drivers/CMSIS/NN/Source/SoftmaxFunctions/arm_softmax_q15.o ./Drivers/CMSIS/NN/Source/SoftmaxFunctions/arm_softmax_q15.su ./Drivers/CMSIS/NN/Source/SoftmaxFunctions/arm_softmax_q7.cyclo ./Drivers/CMSIS/NN/Source/SoftmaxFunctions/arm_softmax_q7.d ./Drivers/CMSIS/NN/Source/SoftmaxFunctions/arm_softmax_q7.o ./Drivers/CMSIS/NN/Source/SoftmaxFunctions/arm_softmax_q7.su ./Drivers/CMSIS/NN/Source/SoftmaxFunctions/arm_softmax_s16.cyclo ./Drivers/CMSIS/NN/Source/SoftmaxFunctions/arm_softmax_s16.d ./Drivers/CMSIS/NN/Source/SoftmaxFunctions/arm_softmax_s16.o ./Drivers/CMSIS/NN/Source/SoftmaxFunctions/arm_softmax_s16.su ./Drivers/CMSIS/NN/Source/SoftmaxFunctions/arm_softmax_s8.cyclo ./Drivers/CMSIS/NN/Source/SoftmaxFunctions/arm_softmax_s8.d ./Drivers/CMSIS/NN/Source/SoftmaxFunctions/arm_softmax_s8.o ./Drivers/CMSIS/NN/Source/SoftmaxFunctions/arm_softmax_s8.su ./Drivers/CMSIS/NN/Source/SoftmaxFunctions/arm_softmax_s8_s16.cyclo ./Drivers/CMSIS/NN/Source/SoftmaxFunctions/arm_softmax_s8_s16.d ./Drivers/CMSIS/NN/Source/SoftmaxFunctions/arm_softmax_s8_s16.o ./Drivers/CMSIS/NN/Source/SoftmaxFunctions/arm_softmax_s8_s16.su ./Drivers/CMSIS/NN/Source/SoftmaxFunctions/arm_softmax_u8.cyclo ./Drivers/CMSIS/NN/Source/SoftmaxFunctions/arm_softmax_u8.d ./Drivers/CMSIS/NN/Source/SoftmaxFunctions/arm_softmax_u8.o ./Drivers/CMSIS/NN/Source/SoftmaxFunctions/arm_softmax_u8.su ./Drivers/CMSIS/NN/Source/SoftmaxFunctions/arm_softmax_with_batch_q7.cyclo ./Drivers/CMSIS/NN/Source/SoftmaxFunctions/arm_softmax_with_batch_q7.d ./Drivers/CMSIS/NN/Source/SoftmaxFunctions/arm_softmax_with_batch_q7.o ./Drivers/CMSIS/NN/Source/SoftmaxFunctions/arm_softmax_with_batch_q7.su

.PHONY: clean-Drivers-2f-CMSIS-2f-NN-2f-Source-2f-SoftmaxFunctions

