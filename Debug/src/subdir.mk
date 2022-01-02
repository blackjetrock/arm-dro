################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
S_SRCS += \
../src/aeabi_romdiv_patch.s 

C_SRCS += \
../src/arm_dro5.c \
../src/cr_startup_lpc11uxx.c \
../src/crp.c \
../src/i2c_functions.c \
../src/max7219.c \
../src/menu.c \
../src/oled096.c \
../src/sysinit.c \
../src/tft_il9341.c 

OBJS += \
./src/aeabi_romdiv_patch.o \
./src/arm_dro5.o \
./src/cr_startup_lpc11uxx.o \
./src/crp.o \
./src/i2c_functions.o \
./src/max7219.o \
./src/menu.o \
./src/oled096.o \
./src/sysinit.o \
./src/tft_il9341.o 

C_DEPS += \
./src/arm_dro5.d \
./src/cr_startup_lpc11uxx.d \
./src/crp.d \
./src/i2c_functions.d \
./src/max7219.d \
./src/menu.d \
./src/oled096.d \
./src/sysinit.d \
./src/tft_il9341.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.s
	@echo 'Building file: $<'
	@echo 'Invoking: MCU Assembler'
	arm-none-eabi-gcc -c -x assembler-with-cpp -DDEBUG -D__CODE_RED -DCORE_M0 -D__USE_LPCOPEN -D__LPC11UXX__ -D__REDLIB__ -I"/home/menadue/LPCXpresso/workspace/nxp_lpcxpresso_11u14_board_lib/inc" -I"/home/menadue/LPCXpresso/workspace/lpc_chip_11uxx_lib/inc" -g3 -mcpu=cortex-m0 -mthumb -specs=redlib.specs -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -DDEBUG -D__CODE_RED -DCORE_M0 -D__USE_LPCOPEN -D__LPC11UXX__ -D__REDLIB__ -I"/home/menadue/LPCXpresso/workspace/nxp_lpcxpresso_11u14_board_lib/inc" -I"/home/menadue/LPCXpresso/workspace/lpc_chip_11uxx_lib/inc" -O0 -fno-common -g3 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -mcpu=cortex-m0 -mthumb -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


