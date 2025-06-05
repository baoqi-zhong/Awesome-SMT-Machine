
CPP_SOURCES += \
$(wildcard $(CORE_DIR)/Drivers/*.cpp) \
$(wildcard $(CORE_DIR)/Drivers/MotorDrivers/*.cpp)

C_INCLUDES += \
-I$(CORE_DIR)/Drivers \
-I$(CORE_DIR)/Drivers/MotorDrivers \
-I$(CORE_DIR)/Control

SystemView_PATH = $(CORE_DIR)/Diagnostic/SystemView
include $(SystemView_PATH)/SystemView.mk

FreeRTOS_DIR = $(CORE_DIR)/FreeRTOS
include $(FreeRTOS_DIR)/FreeRTOS.mk