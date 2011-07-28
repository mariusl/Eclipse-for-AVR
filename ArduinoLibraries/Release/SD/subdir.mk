################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../SD/File.cpp \
../SD/SD.cpp \
../SD/Sd2Card.cpp \
../SD/SdFile.cpp \
../SD/SdVolume.cpp 

OBJS += \
./SD/File.o \
./SD/SD.o \
./SD/Sd2Card.o \
./SD/SdFile.o \
./SD/SdVolume.o 

CPP_DEPS += \
./SD/File.d \
./SD/SD.d \
./SD/Sd2Card.d \
./SD/SdFile.d \
./SD/SdVolume.d 


# Each subdirectory must supply rules for building sources it contributes
SD/%.o: ../SD/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: AVR C++ Compiler'
	avr-g++ -I"C:\arduino-0022\hardware\arduino\cores\arduino" -Wall -Os -fpack-struct -fshort-enums -funsigned-char -funsigned-bitfields -fno-exceptions -mmcu=atmega325p -DF_CPU=16000000UL -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -c -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


