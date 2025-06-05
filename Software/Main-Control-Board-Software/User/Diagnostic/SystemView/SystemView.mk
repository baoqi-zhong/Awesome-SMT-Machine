#######################################
# RM2023 SystemView makefile
#######################################

FREERTOS_HEAP = 0

# Sources
SystemView_SRC =  \
$(SystemView_PATH)/FreeRTOS/Config/Cortex-M/SEGGER_SYSVIEW_Config_FreeRTOS.c \
$(SystemView_PATH)/FreeRTOS/SEGGER_SYSVIEW_FreeRTOS.c \
$(SystemView_PATH)/SEGGER/Syscalls/SEGGER_RTT_Syscalls_GCC.c \
$(SystemView_PATH)/SEGGER/SEGGER_RTT_printf.c \
$(SystemView_PATH)/SEGGER/SEGGER_RTT.c \
$(SystemView_PATH)/SEGGER/SEGGER_SYSVIEW.c

SystemView_AS_SRC =  \
$(SystemView_PATH)/SEGGER/SEGGER_RTT_ASM_ARMv7M.s

# Includes
SystemView_INC =  \
-I$(SystemView_PATH)/Config  \
-I$(SystemView_PATH)/FreeRTOS \
-I$(SystemView_PATH)/SEGGER

C_SOURCES += $(SystemView_SRC)
ASM_SOURCES += $(SystemView_AS_SRC)
C_DEFS += -DAPP_NAME=$(TARGET)
C_INCLUDES += $(SystemView_INC)