################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../CEnvir.cpp \
../CGenet.cpp \
../CGrid.cpp \
../CGridEnvir.cpp \
../CObject.cpp \
../CSeed.cpp \
../Cell.cpp \
../IBC-grass.cpp \
../LCG.cpp \
../OutStructs.cpp \
../Plant.cpp \
../RunPara.cpp \
../SPftTraits.cpp 

OBJS += \
./CEnvir.o \
./CGenet.o \
./CGrid.o \
./CGridEnvir.o \
./CObject.o \
./CSeed.o \
./Cell.o \
./IBC-grass.o \
./LCG.o \
./OutStructs.o \
./Plant.o \
./RunPara.o \
./SPftTraits.o 

CPP_DEPS += \
./CEnvir.d \
./CGenet.d \
./CGrid.d \
./CGridEnvir.d \
./CObject.d \
./CSeed.d \
./Cell.d \
./IBC-grass.d \
./LCG.d \
./OutStructs.d \
./Plant.d \
./RunPara.d \
./SPftTraits.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


