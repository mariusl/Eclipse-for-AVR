################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../DevicePrint/DevicePrint.cpp 

OBJS += \
./DevicePrint/DevicePrint.o 

CPP_DEPS += \
./DevicePrint/DevicePrint.d 


# Each subdirectory must supply rules for building sources it contributes
DevicePrint/%.o: ../DevicePrint/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: AVR C++ Compiler'
	avr-g++ -I"C:\arduino-0022\hardware\arduino\cores\arduino" -Wall -Os -fpack-struct -fshort-enums -funsigned-char -funsigned-bitfields -fno-exceptions -mmcu=atmega325p -DF_CPU=16000000UL -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -c -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


