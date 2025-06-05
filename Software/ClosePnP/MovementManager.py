import time
import SerialManager

from threading import Thread
import math


# X, Y, Z, Rotation
# 单位: mm, mm, mm, degree
targetPosition = [0, 0, 9, 0]
targetPositionLowerLimit = [0, 0, 0, -9999]
targetPositionUpperLimit = [120, 160, 9, 9999]

# # X, Y, Z, Rotation
# # 单位: mm, mm, mm, degree
# positionFeedback = [0, 0, 0, 0]

# 这个速度其实是假的速度, 因为速度是下位机控制的, 上位机不直接控制下位机速度.
# 这个的意义是单次移动的距离, 类似于进给速度
# X, Y, Z, Rotation
# 单位: mm/s, mm/s, mm/s, degree/s
maxTargetSpeed = [2, 2, 2, 45]
targetSpeed = [2, 2, 2, 45]

pumpStatus = False

componentOnFeederDeltaZ = -8.1
componentOnPCBDeltaZ = -7.9

closePnPapp = None
availableStateMachineStatus = [
    "None",

    # 校准状态
    "Finding Origin",
    "Waiting for Feeder1 Component 1",
    "Waiting for Feeder1 Component 2",

    "Waiting for Feeder2 Component 1",
    "Waiting for Feeder2 Component 2",

    "Waiting for Feeder3 Component 1",
    "Waiting for Feeder3 Component 2",

    "Waiting for PCB Component 1",

    # 贴片状态
    "Waiting for Current SMT Job Done",
]
stateMachineStatus = "None"      

availableStateMachineTriggerSignal = [
    "None",

    # 校准状态
    "StartCalibration",
    "Finding Origin Success",
    "Finding Origin Failed",
    "Feeder1 Component 1 Done",
    "Feeder1 Component 2 Done",
    "Feeder2 Component 1 Done",
    "Feeder2 Component 2 Done",
    "Feeder3 Component 1 Done",
    "Feeder3 Component 2 Done",
    "PCB Component 1 Done",
    "AbortCalibration",

    # 贴片状态
    "StartSMT",
    "Current SMT Job Done",
    "AbortSMT",
]
stateMachineTriggerSignal = "None"

# 状态机设计: 由 calibrationTriggerSignal 触发状态机的状态转换

# 外部调用
def setTargetPosition(x, y, z, rotation):
    global targetPosition
    targetPosition = [x, y, z, rotation]

def setTargetSpeed(dx, dy, dz, dRotation):
    global targetSpeed
    targetSpeed = [dx, dy, dz, dRotation]

def setFeed(feed):
    global targetSpeed
    if closePnPapp:
        closePnPapp.tab2_speed_scale.set(int(feed * 100))
    targetSpeed = [maxTargetSpeed[0] * feed, maxTargetSpeed[1] * feed, maxTargetSpeed[2] * feed, maxTargetSpeed[3] * feed]

def setStateMachineTriggerSignal(signal):
    global stateMachineTriggerSignal
    stateMachineTriggerSignal = signal

def emrgencyStop(app):
    if not SerialManager.ser.is_open:
        print("\033[31m", end="")
        print("串口未打开")
        print("\033[30m", end="")
        return

    print("\033[31m", end="")
    print("紧急停止")
    print("\033[30m", end="")
    SerialManager.sendCommand("M112\n")

    SerialManager.ser.timeout = 1
    recv = SerialManager.ser.readline().decode("ascii")

    if not recv:
        print("\033[31m", end="")
        print("紧急停止超时失败")
        print("\033[30m", end="")
        return
    
    SerialManager.decodeFeedbackMessage(app, recv)

def recoverFromEmergencyStop(app):
    if not SerialManager.ser.is_open:
        print("\033[31m", end="")
        print("串口未打开")
        print("\033[30m", end="")
        return

    print("\033[32m", end="")
    print("恢复运动")
    print("\033[30m", end="")
    
    SerialManager.sendCommand("M999\n")


    SerialManager.ser.timeout = 1
    recv = SerialManager.ser.readline().decode("ascii")

    if not recv:
        print("\033[31m", end="")
        print("紧急停止超时失败")
        print("\033[30m", end="")
        return
    
    SerialManager.decodeFeedbackMessage(app, recv)

def move(dx, dy, dz, dRotation):
    global targetPosition

    if not SerialManager.ser.is_open:
        print("\033[31m", end="")
        print("串口未打开")
        print("\033[30m", end="")
        return

    targetPosition = [targetPosition[0] + dx, targetPosition[1] + dy, targetPosition[2] + dz, targetPosition[3] + dRotation]
    # 限制位置
    for i in range(4):
        if targetPosition[i] < targetPositionLowerLimit[i]:
            targetPosition[i] = targetPositionLowerLimit[i]
        if targetPosition[i] > targetPositionUpperLimit[i]:
            targetPosition[i] = targetPositionUpperLimit[i]

    if pumpStatus:
        SerialManager.sendCommand("G1 X{} Y{} Z{} E{}\n".format(str(int(targetPosition[0] * 100)).rjust(5, " "), str(int(targetPosition[1] * 100)).rjust(5, " "), str(int(targetPosition[2] * 100)).rjust(5, " "), str(int(targetPosition[3] * 100)).rjust(5, " ")))
    else:
        SerialManager.sendCommand("G0 X{} Y{} Z{} E{}\n".format(str(int(targetPosition[0] * 100)).rjust(5, " "), str(int(targetPosition[1] * 100)).rjust(5, " "), str(int(targetPosition[2] * 100)).rjust(5, " "), str(int(targetPosition[3] * 100)).rjust(5, " ")))

    SerialManager.ser.timeout = 3
    recv = SerialManager.ser.readline().decode("ascii")

    if not recv:
        print("\033[31m", end="")
        print("移动超时失败")
        print("\033[30m", end="")
        return
    
    SerialManager.decodeFeedbackMessage(closePnPapp, recv)
    
def moveForwardOneStep():
    move(0, targetSpeed[1], 0, 0)

def moveBackwardOneStep():
    move(0, -targetSpeed[1], 0, 0)

def moveLeftOneStep():
    move(-targetSpeed[0], 0, 0, 0)

def moveRightOneStep():
    move(targetSpeed[0], 0, 0, 0)

def moveUpOneStep():
    move(0, 0, targetSpeed[2], 0)

def moveDownOneStep():
    move(0, 0, -targetSpeed[2], 0)

def rotateClockwiseOneStep():
    move(0, 0, 0, targetSpeed[3])

def rotateCounterClockwiseOneStep():
    move(0, 0, 0, -targetSpeed[3])

def stopAllMovement():
    global targetSpeed
    targetSpeed = (0, 0, 0, 0)

def pumpOn():
    global pumpStatus
    pumpStatus = True
    move(0, 0, 0, 0)

def pumpOff():
    global pumpStatus
    pumpStatus = False
    move(0, 0, 0, 0)

def findOrigin():
    if not SerialManager.ser.is_open:
        print("\033[31m", end="")
        print("串口未打开")
        print("\033[30m", end="")
        return
    
    if pumpStatus:
        pumpOff()
        time.sleep(0.2)
    SerialManager.sendCommand("G28\n")

    SerialManager.ser.timeout = 10
    recv = SerialManager.ser.readline().decode("ascii")
    SerialManager.ser.timeout = 1

    if not recv:
        print("\033[31m", end="")
        print("回零超时失败...")
        print("\033[30m", end="")

        closePnPapp.tab1_serial_status_label.config(text="回零超时失败...")
        closePnPapp.tab1_serial_status_label.config(foreground="orange")
        return
    
    SerialManager.decodeFeedbackMessage(closePnPapp, recv)

def returnToOrigin():
    if not SerialManager.ser.is_open:
        print("\033[31m", end="")
        print("串口未打开")
        print("\033[30m", end="")
        return

    # 移动前先抬升
    move(0, 0, 9, 0)
    time.sleep(0.6)
    setTargetPosition(0, 0, 9, 0)
    move(0, 0, 0, 0)

def closest_multiple_of_4(num):
    # 判断是否是 4 的整数倍
    num = int(num)
    if num % 4 <= 2:
        return num // 4
    else:
        return num // 4 + 1


def recordComponentPosition(feederIndex, index):
    if index == 0:
        # 最左边的元件
        closePnPapp.config["feederZeroPosition"][feederIndex] = [targetPosition[0], targetPosition[1]]
        closePnPapp.saveConfig()
        
    else:
        # 判断间隔是多少个元件
        # dXn = closest_multiple_of_4(abs(targetPosition[0] - closePnPapp.config["feederZeroPosition"][feederIndex][0]))
        dXn = 16 # 和上面一样, 写死 16
        # distance = math.sqrt((targetPosition[0] - config["feederZeroPosition"][feederIndex][0]) ** 2 + (targetPosition[1] - config["feederZeroPosition"][feederIndex][1]) ** 2) / dXn
        # print("dXn", dXn)
        # print("distance", distance)
        closePnPapp.config["fedderComponentDistance"][feederIndex][0] = (targetPosition[0] - closePnPapp.config["feederZeroPosition"][feederIndex][0]) / dXn
        closePnPapp.config["fedderComponentDistance"][feederIndex][1] = (targetPosition[1] - closePnPapp.config["feederZeroPosition"][feederIndex][1]) / dXn
        closePnPapp.saveConfig()

def getMostLeftUpComponentIndexOnPCB():
    targetPositionData = closePnPapp.targetPositionData
    leftUpIndex = -1
    # rightDownIndex = -1
    maxYPositionMinusXPosition = -9999   # 最大的 y - x, 用于找到左上角的元件
    # minYPositionMinusXPosition = 9999    # 最小的 y - x, 用于找到右下角的元件
    for i in range(len(targetPositionData)):
        x = targetPositionData[i][1]
        y = targetPositionData[i][2]
        if y - x > maxYPositionMinusXPosition:
            maxYPositionMinusXPosition = y - x
            leftUpIndex = i
        # if y - x < minYPositionMinusXPosition:
        #     minYPositionMinusXPosition = y - x
        #     rightDownIndex = i
    
    return leftUpIndex  #, rightDownIndex


def recordPCBComponentPosition(index):
    if index == 0:
        # 最左上角的元件
        closePnPapp.config["PCBLeftUpComponentPosition"] = [targetPosition[0], targetPosition[1]]
    else:
        # 右下角的元件
        closePnPapp.config["PCBRightDownPosition"] = [targetPosition[0], targetPosition[1]]
        
    closePnPapp.saveConfig()

calibrationComponentIndex = 16      # 用第 16 个元件做校准
def moveToComponent(feederIndex, Xn):
    # Xn 从 0 开始
    if not SerialManager.ser.is_open:
        print("\033[31m", end="")
        print("串口未打开")
        print("\033[30m", end="")
        return


    print("移动到 config 中的元件位置")
    print(closePnPapp.config["feederZeroPosition"][feederIndex])
    
    # 移动前先抬升
    move(0, 0, 9, 0)
    time.sleep(0.4)
    setTargetPosition(closePnPapp.config["feederZeroPosition"][feederIndex][0] + closePnPapp.config["fedderComponentDistance"][feederIndex][0] * Xn , closePnPapp.config["feederZeroPosition"][feederIndex][1] + closePnPapp.config["fedderComponentDistance"][feederIndex][1] * Xn, 9, 0)
    move(0, 0, 0, 0)
    time.sleep(1.5)
    move(0, 0, componentOnFeederDeltaZ, 0)

def moveToComponentOnPCB(index):
    targetComponentPositionOnPCB = closePnPapp.targetPositionData[index]
    leftUpComponentPositionOnPCB = closePnPapp.targetPositionData[getMostLeftUpComponentIndexOnPCB()]

    print("移动到 config 中 PCB 上元件位置")
    print("leftUpComponentPositionOnPCB", leftUpComponentPositionOnPCB)
    print("targetComponentPositionOnPCB", targetComponentPositionOnPCB)
    
    targetPositionX = closePnPapp.config["PCBLeftUpComponentPosition"][0] - leftUpComponentPositionOnPCB[1] + targetComponentPositionOnPCB[1]
    targetPositionY = closePnPapp.config["PCBLeftUpComponentPosition"][1] - leftUpComponentPositionOnPCB[2] + targetComponentPositionOnPCB[2]

    
    # 移动前先抬升
    move(0, 0, 9, 0)
    time.sleep(0.4)

    setTargetPosition(targetPositionX, targetPositionY, 9, 0)
    move(0, 0, 0, 0)
    time.sleep(1.5)
    move(0, 0, componentOnPCBDeltaZ, 0)

currentSMTComponentIndex = 0        # 当前贴片的元件索引
smtReverse = False
def getNextComponentIndexOnFeeder(componentType):
    for i in range(3): 
        if closePnPapp.config["feeder" + str(i + 1)]["componentType"] == componentType:
            for j in range(len(closePnPapp.config["feeder" + str(i + 1)]["componentAvailableStatus"])):
                if closePnPapp.config["feeder" + str(i + 1)]["componentAvailableStatus"][j] == True:
                    return i, j
    return -1, -1

def doSMTJob(index):
    if smtReverse:
        doSMTJobReverse(index)
        return

    global stateMachineTriggerSignal
    # 只要是同类型的元件就可以, 不需要判断阻值容值
    print("doSMTJob", index)
    componentType = closePnPapp.targetPositionData[index][0][0]
    print("componentType", componentType)
    feederIndex, componentIndex = getNextComponentIndexOnFeeder(componentType)
    print("feederIndex", feederIndex)
    print("componentIndex", componentIndex)
    if feederIndex == -1:
        print("\033[31m", end="")
        print("没有找到合适的元件")
        print("\033[30m", end="")
        stateMachineTriggerSignal = "Current SMT Job Done"
        return
    
    moveToComponent(feederIndex, componentIndex)
    pumpOn()
    time.sleep(0.4)
    moveToComponentOnPCB(index)
    time.sleep(0.4)
    pumpOff()
    time.sleep(0.4)

    closePnPapp.config["feeder" + str(feederIndex + 1)]["componentAvailableStatus"][componentIndex] = False
    stateMachineTriggerSignal = "Current SMT Job Done"

def doSMTJobReverse(index):
    global stateMachineTriggerSignal
    # 只要是同类型的元件就可以, 不需要判断阻值容值
    print("doSMTJob", index)
    componentType = closePnPapp.targetPositionData[index][0][0]
    print("componentType", componentType)
    feederIndex, componentIndex = getNextComponentIndexOnFeeder(componentType)
    print("feederIndex", feederIndex)
    print("componentIndex", componentIndex)
    if feederIndex == -1:
        print("\033[31m", end="")
        print("没有找到合适的元件")
        print("\033[30m", end="")
        stateMachineTriggerSignal = "Current SMT Job Done"
        return
    
    moveToComponentOnPCB(index)
    pumpOn()
    time.sleep(0.5)
    moveToComponent(feederIndex, componentIndex)
    pumpOff()
    time.sleep(0.5)
    closePnPapp.config["feeder" + str(feederIndex + 1)]["componentAvailableStatus"][componentIndex] = False
    stateMachineTriggerSignal = "Current SMT Job Done"


def startSMT():
    global stateMachineStatus, stateMachineTriggerSignal
    if stateMachineStatus == "None":
        stateMachineTriggerSignal = "StartSMT"
    else:
        stateMachineTriggerSignal = "AbortSMT"

def onCalibrationButtonClicked():
    global stateMachineTriggerSignal
    print("calibrationButtonClicked when calibrationStatus is", stateMachineStatus)
    if stateMachineStatus == "None":
        stateMachineTriggerSignal = "StartCalibration"
    elif stateMachineStatus == "Waiting for Feeder1 Component 1":
        stateMachineTriggerSignal = "Feeder1 Component 1 Done"
    elif stateMachineStatus == "Waiting for Feeder1 Component 2":
        stateMachineTriggerSignal = "Feeder1 Component 2 Done"
    elif stateMachineStatus == "Waiting for Feeder2 Component 1":
        stateMachineTriggerSignal = "Feeder2 Component 1 Done"
    elif stateMachineStatus == "Waiting for Feeder2 Component 2":
        stateMachineTriggerSignal = "Feeder2 Component 2 Done"
    elif stateMachineStatus == "Waiting for Feeder3 Component 1":
        stateMachineTriggerSignal = "Feeder3 Component 1 Done"
    elif stateMachineStatus == "Waiting for Feeder3 Component 2":
        stateMachineTriggerSignal = "Feeder3 Component 2 Done"
    elif stateMachineStatus == "Waiting for PCB Component 1":
        stateMachineTriggerSignal = "PCB Component 1 Done"


def onCalibrationAbortButtonClicked():
    global stateMachineTriggerSignal
    stateMachineTriggerSignal = "AbortCalibration"


def autoMovementTask():
    global stateMachineStatus, stateMachineTriggerSignal, currentSMTComponentIndex
    while True:
        # Safety Protection 防止傻逼
        if not stateMachineStatus in availableStateMachineStatus:
            print("\033[31m", end="")
            print("calibrationStatus 无效")
            print("stateMachineStatus", stateMachineStatus)
            print("\033[30m", end="")
            stateMachineStatus = "None"
            continue

        if not stateMachineTriggerSignal in availableStateMachineTriggerSignal:
            print("\033[31m", end="")
            print("calibrationTriggerSignal 无效")
            print("stateMachineTriggerSignal", stateMachineTriggerSignal)
            print("\033[30m", end="")
            stateMachineTriggerSignal = "None"
            continue


        # 状态机转换
        if stateMachineStatus == "None" and stateMachineTriggerSignal == "StartCalibration":
            stateMachineTriggerSignal = "None"

            if not SerialManager.ser.is_open:
                print("\033[31m", end="")
                print("串口未打开")
                print("\033[30m", end="")

                # 等待下次触发
                continue
    
            stateMachineStatus = "Finding Origin"
            findOrigin()
        
        elif stateMachineStatus == "Finding Origin" and stateMachineTriggerSignal == "Finding Origin Success":
            stateMachineTriggerSignal = "None"

            if closePnPapp.tab3_feeder1_component_type_entry.get() != "None":
                moveToComponent(0, 0)
                setFeed(0.3)        # 降低速度
                stateMachineStatus = "Waiting for Feeder1 Component 1"
                closePnPapp.tab3_start_calibration_step_button.config(text="Feeder1 Component 1 Done")
            elif closePnPapp.tab3_feeder2_component_type_entry.get() != "None":
                moveToComponent(1, 0)
                setFeed(0.3)
                stateMachineStatus = "Waiting for Feeder2 Component 1"
                closePnPapp.tab3_start_calibration_step_button.config(text="Feeder2 Component 1 Done")
            elif closePnPapp.tab3_feeder3_component_type_entry.get() != "None":
                moveToComponent(2, 0)
                setFeed(0.3)
                stateMachineStatus = "Waiting for Feeder3 Component 1"
                closePnPapp.tab3_start_calibration_step_button.config(text="Feeder3 Component 1 Done")
            else:
                print("\033[31m", end="")
                print("没有元件")
                print("\033[30m", end="")
                stateMachineTriggerSignal = "AbortCalibration"

        elif stateMachineStatus == "Finding Origin" and stateMachineTriggerSignal == "Finding Origin Failed":
            stateMachineTriggerSignal = "AbortCalibration"

        elif stateMachineStatus == "Waiting for Feeder1 Component 1" and stateMachineTriggerSignal == "Feeder1 Component 1 Done":
            stateMachineTriggerSignal = "None"

            recordComponentPosition(0, 0)
            moveToComponent(0, calibrationComponentIndex)
            setFeed(0.3)
            stateMachineStatus = "Waiting for Feeder1 Component 2"
            closePnPapp.tab3_start_calibration_step_button.config(text="Feeder1 Component 2 Done")

        elif stateMachineStatus == "Waiting for Feeder1 Component 2" and stateMachineTriggerSignal == "Feeder1 Component 2 Done":
            stateMachineTriggerSignal = "None"

            recordComponentPosition(0, 1)

            if closePnPapp.tab3_feeder2_component_type_entry.get() != "None":
                moveToComponent(1, 0)
                setFeed(0.3)
                stateMachineStatus = "Waiting for Feeder2 Component 1"
                closePnPapp.tab3_start_calibration_step_button.config(text="Feeder2 Component 1 Done")
            elif closePnPapp.tab3_feeder3_component_type_entry.get() != "None":
                moveToComponent(2, 0)
                setFeed(0.3)
                stateMachineStatus = "Waiting for Feeder3 Component 1"
                closePnPapp.tab3_start_calibration_step_button.config(text="Feeder3 Component 1 Done")
            else:
                moveToComponentOnPCB(getMostLeftUpComponentIndexOnPCB())
                setFeed(0.3)
                stateMachineStatus = "Waiting for PCB Component 1"
                closePnPapp.tab3_start_calibration_step_button.config(text="PCB Component 1 Done")


        elif stateMachineStatus == "Waiting for Feeder2 Component 1" and stateMachineTriggerSignal == "Feeder2 Component 1 Done":
            stateMachineTriggerSignal = "None"

            recordComponentPosition(1, 0)
            moveToComponent(1, calibrationComponentIndex)
            setFeed(0.3)
            stateMachineStatus = "Waiting for Feeder2 Component 2"
            closePnPapp.tab3_start_calibration_step_button.config(text="Feeder2 Component 2 Done")

        
        elif stateMachineStatus == "Waiting for Feeder2 Component 2" and stateMachineTriggerSignal == "Feeder2 Component 2 Done":
            stateMachineTriggerSignal = "None"

            recordComponentPosition(1, 1)

            if closePnPapp.tab3_feeder3_component_type_entry.get() != "None":
                moveToComponent(2, 0)
                setFeed(0.3)
                stateMachineStatus = "Waiting for Feeder3 Component 1"
                closePnPapp.tab3_start_calibration_step_button.config(text="Feeder3 Component 1 Done")
            else:
                moveToComponentOnPCB(getMostLeftUpComponentIndexOnPCB())
                setFeed(0.3)
                stateMachineStatus = "Waiting for PCB Component 1"
                closePnPapp.tab3_start_calibration_step_button.config(text="PCB Component 1 Done")

        
        elif stateMachineStatus == "Waiting for Feeder3 Component 1" and stateMachineTriggerSignal == "Feeder3 Component 1 Done":
            stateMachineTriggerSignal = "None"

            recordComponentPosition(2, 0)
            moveToComponent(2, calibrationComponentIndex)
            setFeed(0.3)
            stateMachineStatus = "Waiting for Feeder3 Component 2"
            closePnPapp.tab3_start_calibration_step_button.config(text="Feeder3 Component 2 Done")
        
        elif stateMachineStatus == "Waiting for Feeder3 Component 2" and stateMachineTriggerSignal == "Feeder3 Component 2 Done":
            stateMachineTriggerSignal = "None"

            recordComponentPosition(2, 1)
            moveToComponentOnPCB(getMostLeftUpComponentIndexOnPCB())
            setFeed(0.3)
            stateMachineStatus = "Waiting for PCB Component 1"
            closePnPapp.tab3_start_calibration_step_button.config(text="PCB Component 1 Done")
        
        elif stateMachineStatus == "Waiting for PCB Component 1" and stateMachineTriggerSignal == "PCB Component 1 Done":
            stateMachineTriggerSignal = "None"

            recordPCBComponentPosition(0)
            closePnPapp.saveConfig()
            returnToOrigin()
            setFeed(1)
            stateMachineStatus = "None"
            closePnPapp.tab3_start_calibration_step_button.config(text="重新校准")

        elif stateMachineTriggerSignal == "AbortCalibration":
            stateMachineTriggerSignal = "None"
            SerialManager.sendCommand("M997 I\n")   # 回到 Idle

            returnToOrigin()
            setFeed(1)
            stateMachineStatus = "None"
            closePnPapp.tab3_start_calibration_step_button.config(text="开始校准步骤")

        elif stateMachineStatus == "None" and stateMachineTriggerSignal == "StartSMT":
            stateMachineTriggerSignal = "None"
            if not SerialManager.ser.is_open:
                print("\033[31m", end="")
                print("串口未打开")
                print("\033[30m", end="")

                # 等待下次触发
                continue

            SerialManager.sendCommand("M997 S\n")   # SMT 开始
            time.sleep(0.1)
            SerialManager.sendCommand("M998 " + closePnPapp.targetPositionData[currentSMTComponentIndex][0].rjust(3, " ") + " " + str(currentSMTComponentIndex) +"\n")
            
            currentSMTComponentIndex = 0
            doSMTJob(currentSMTComponentIndex)
            stateMachineStatus = "Waiting for Current SMT Job Done"

            closePnPapp.tab2_start_SMT_button.config(text="停止贴片")

        elif stateMachineStatus == "Waiting for Current SMT Job Done" and stateMachineTriggerSignal == "Current SMT Job Done":
            stateMachineTriggerSignal = "None"

            SerialManager.sendCommand("M998 " + closePnPapp.targetPositionData[currentSMTComponentIndex][0].rjust(3, " ") + " " + str(currentSMTComponentIndex) +"\n")
            
            currentSMTComponentIndex += 1
            if currentSMTComponentIndex < len(closePnPapp.targetPositionData):
                doSMTJob(currentSMTComponentIndex)
                stateMachineStatus = "Waiting for Current SMT Job Done"
            else:
                # 结束 SMT
                stateMachineStatus = "None"
                SerialManager.sendCommand("M997 F\n")   # SMT 结束
                returnToOrigin()
                closePnPapp.tab2_start_SMT_button.config(text="开始贴片")
        
        elif stateMachineTriggerSignal == "AbortSMT":
            stateMachineTriggerSignal = "None"
            stateMachineStatus = "None"
            SerialManager.sendCommand("M997 I\n")   # 回到 Idle

            returnToOrigin()
            closePnPapp.tab2_start_SMT_button.config(text="开始贴片")
        

        time.sleep(0.1)

def init(app):
    global closePnPapp
    closePnPapp = app
    Thread(target=autoMovementTask).start()
