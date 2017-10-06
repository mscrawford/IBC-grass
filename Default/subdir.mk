################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/Cell.cpp \
../src/Environment.cpp \
../src/Genet.cpp \
../src/Grid.cpp \
../src/GridEnvir.cpp \
../src/IBC-grass.cpp \
../src/Output.cpp \
../src/Parameters.cpp \
../src/Plant.cpp \
../src/RandomGenerator.cpp \
../src/Seed.cpp \
../src/Traits.cpp 

OBJS += \
./src/Cell.o \
./src/Environment.o \
./src/Genet.o \
./src/Grid.o \
./src/GridEnvir.o \
./src/IBC-grass.o \
./src/Output.o \
./src/Parameters.o \
./src/Plant.o \
./src/RandomGenerator.o \
./src/Seed.o \
./src/Traits.o 

CPP_DEPS += \
./src/Cell.d \
./src/Environment.d \
./src/Genet.d \
./src/Grid.d \
./src/GridEnvir.d \
./src/IBC-grass.d \
./src/Output.d \
./src/Parameters.d \
./src/Plant.d \
./src/RandomGenerator.d \
./src/Seed.d \
./src/Traits.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -std=c++14 -O2 -g -Wall -pedantic -pedantic-errors -Wextra -Wcast-align -Wcast-qual -Wdisabled-optimization -Wfloat-equal -Wformat=2 -Wformat-nonliteral -Wformat-security -Wformat-y2k -Wimport -Winit-self -Winline -Winvalid-pch -Wlong-long -Wmissing-field-initializers -Wmissing-format-attribute -Wmissing-include-dirs -Wmissing-noreturn -Wpacked -Wpointer-arith -Wredundant-decls -Wshadow -Wstack-protector -Wstrict-aliasing=2 -Wswitch-default -Wswitch-enum -Wunreachable-code -Wunused -Wunused-parameter -Wvariadic-macros -Wwrite-strings -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


