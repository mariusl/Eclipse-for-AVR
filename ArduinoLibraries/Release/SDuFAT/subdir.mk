################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../SDuFAT/SDuFAT.cpp \
../SDuFAT/microfat.cpp \
../SDuFAT/mmc.cpp 

OBJS += \
./SDuFAT/SDuFAT.o \
./SDuFAT/microfat.o \
./SDuFAT/mmc.o 

CPP_DEPS += \
./SDuFAT/SDuFAT.d \
./SDuFAT/microfat.d \
./SDuFAT/mmc.d 


# Each subdirectory must supply rules for building sources it contributes
SDuFAT/%.o: ../SDuFAT/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: AVR C++ Compiler'
	avr-g++ -I"C:\arduino-0022\hardware\arduino\cores\arduino" -Wall -Os -fpack-struct -fshort-enums -funsigned-char -funsigned-bitfields -fno-exceptions -mmcu=atmega325p -DF_CPU=16000000UL -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -c -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


