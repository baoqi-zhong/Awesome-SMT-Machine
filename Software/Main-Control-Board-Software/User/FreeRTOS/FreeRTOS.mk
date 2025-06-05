FREERTOS_HEAP = 0

# Sources
FreeRTOS_SRC =  \
$(FreeRTOS_DIR)/sources/croutine.c \
$(FreeRTOS_DIR)/sources/event_groups.c \
$(FreeRTOS_DIR)/sources/list.c \
$(FreeRTOS_DIR)/sources/queue.c \
$(FreeRTOS_DIR)/sources/stream_buffer.c \
$(FreeRTOS_DIR)/sources/tasks.c \
$(FreeRTOS_DIR)/sources/timers.c

FreeRTOS_SRC +=  \
$(FreeRTOS_DIR)/portable/GCC/ARM_CM4/port.c

ifneq ($(FREERTOS_HEAP), 0)
FreeRTOS_SRC += $(FreeRTOS_DIR)/portable/MemMang/heap_$(FREERTOS_HEAP).c
endif

C_SOURCES += $(FreeRTOS_SRC)

CPP_SOURCES += $(FreeRTOS_DIR)/sources/os.cpp \
$(FreeRTOS_DIR)/Hook.cpp

C_INCLUDES += -I$(FreeRTOS_DIR)/include
C_INCLUDES += -I$(FreeRTOS_DIR)/portable/GCC/ARM_CM4/