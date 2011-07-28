################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../TimerOne-v2/TimerOne.cpp 

OBJS += \
./TimerOne-v2/TimerOne.o 

CPP_DEPS += \
./TimerOne-v2/TimerOne.d 


# Each subdirectory must supply rules for building sources it contributes
TimerOne-v2/%.o: ../TimerOne-v2/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: AVR C++ Compiler'
	avr-g++ -I"C:\arduino-0022\hardware\arduino\cores\arduino" -Wall -Os -fpack-struct -fshort-enums -funsigned-char -funsigned-bitfields -fno-exceptions -mmcu=atmega325p -DF_CPU=16000000UL -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -c -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


