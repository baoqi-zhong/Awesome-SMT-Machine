"""
通信协议:
发送指令后, 阻塞等待一段时间接收反馈信息, 这段时间 Tkinter 界面会卡死

通信协议:
发送指令格式:
    1. [G0/G1] Xxxxxx Yxxxxx Zxxxxx Exxxxx\n 移动到指定位置, XYZ 为坐标, 单位 mm; E 为旋转角度, 单位: 度
        示例: G1 X  -30 Y  -60 Z    0 E    0\n
    2. G28      通过撞开关找原点 
        示例: G28\n
    3. M115     获取固件版本, 用于连通性测试
        示例: M115\n
    
反馈信息格式:
    1. ok Conn [版本信息]      连接成功
        示例: ok Conn FIRMWARE_NAME:ELEC3300_SMT FIRMWARE_VERSION:1.0 
    2. [ok/er] Origin               找到原点成功/失败
        示例: ok Origin
"""

import serial
import serial.tools.list_ports

from threading import Thread
import time

ser = serial.Serial()
ser.timeout = 1

targetPort = []

def refreshSerialPorts():
    global targetPort

    targetPort = []

    # 获取当前系统中可用的串口信息
    available_ports = serial.tools.list_ports.comports()

    for port in available_ports:
        if "Bluetooth" in port.description:
            continue
        
        if "CH340" in port.description:
            targetPort.append(port)

    if len(targetPort) == 0:
        print("\033[31m", end="") # 红色
        print("没有检测到可用的串口.")
        print("\033[30m", end="") # 黑
        # exit()

    elif len(targetPort) > 1:
        print("\033[31m", end="") # 红色
        print("检测到多个CH340串口")
        print("\033[30m", end="") # 黑
        for port in available_ports:
            targetPort.append(port)
            print(f"{port.description}: {port.device}")
        # exit()

    else:
        print("\033[32m", end="") # 绿色
        print("检测到一个CH340串口")
        print("\033[30m", end="") # 黑

        print(f"{targetPort[0].description}: {targetPort[0].device}")


def sendCommand(command):
    ser.write(command.encode("ascii"))
    print("\033[33m[SEND <<]", command, end="") # 黄色
    time.sleep(0.05)

def decodeFeedbackMessage(app, feedbackMessage):
    print("\033[94m[RECV >>]", feedbackMessage, "\033[30m")
    feedbackMessage = feedbackMessage.split()

    # if feedbackMessage[0].lower() != "ok":
    #     print("\033[31m", end="") # 红色
    #     print("Error: ", "".join(feedbackMessage))
    

    if feedbackMessage[1] == "Conn":
        if feedbackMessage[0].lower() == "er":
            print("\033[31m", end="")
            print("连接失败")
            print("\033[30m", end="")
            app.tab1_serial_status_label.config(text="连接失败")
            app.tab1_serial_status_label.config(foreground="red")
            return
        
        print("\033[32m", end="")
        print("已连接")
        print("\033[30m", end="")

        app.tab1_serial_status_label.config(text="已连接")
        app.tab1_serial_status_label.config(foreground="green")
        app.tab1_apply_button.config(text="Close")

        # from MovementManager import findOrigin
        # findOrigin(app)

    elif feedbackMessage[1] == "Origin":
        if feedbackMessage[0].lower() == "er":
            print("\033[31m", end="")
            print("找原点失败(超时)")
            print("\033[30m", end="")
            app.tab1_serial_status_label.config(text="找原点失败(超时)")
            app.tab1_serial_status_label.config(foreground="red")

            from MovementManager import stateMachineStatus, setStateMachineTriggerSignal
            if stateMachineStatus == "Finding Origin":
                setStateMachineTriggerSignal("Finding Origin Failed")
            return
        
        print("\033[32m", end="") # 绿色
        print("已找到原点")
        print("\033[30m", end="")

        # 与主控板内的 TargetPosition 同步
        # 在没有实时解算坐标反馈的情况下有用.
        from MovementManager import setTargetPosition
        # 这里重置的位置需要与下位机保持一致
        setTargetPosition(5, 5, 9, 0)
        time.sleep(0.5)
        from MovementManager import stateMachineStatus, setStateMachineTriggerSignal
        if stateMachineStatus == "Finding Origin":
            setStateMachineTriggerSignal("Finding Origin Success")

    elif feedbackMessage[1] == "Emergency":
        if feedbackMessage[2] == "Stopped":
            print("\033[31m", end="")
            print("紧急停止成功")
            print("\033[30m", end="")

            app.tab2_emergency_stop_img.config(image=app.reRunImgTK)
            app.tab2_emergency_stop_img.config(text="ReRun")
        elif feedbackMessage[2] == "Restarted":
            print("\033[32m", end="")
            print("重新启动成功")
            print("\033[30m", end="")

            app.tab2_emergency_stop_img.config(image=app.emergencyStopImgTK)
            app.tab2_emergency_stop_img.config(text="Emergency Stop")

    elif feedbackMessage[1] == "Move":
        print("\033[32m", end="")
        print("移动完成")
        print("\033[30m", end="")



def openSerial(app, portIndex, baudrate):
    # 打开串口
    global ser
    if ser.is_open:
        print("\033[31m", end="") # 红色
        print(ser.name, "已经打开")
        print("\033[30m", end="") # 黑
        return
    
    
    if portIndex == -1 or portIndex > len(targetPort) - 1:
        print("\033[31m", end="")
        print("串口不存在") # 红色
        print("\033[30m", end="")
        return
    
    ser.port = targetPort[portIndex].device
    ser.baudrate = baudrate
    try:
        ser.open()
    except serial.SerialException as e:
        print("\033[31m", end="")
        print(e)
        print("\033[30m", end="")
        return
    
    print("\033[32m", end="") # 绿色
    print(ser.name, "已打开")
    print("\033[30m", end="") # 黑

    app.tab1.focus_set()

    sendCommand("M115\n")
    ser.timeout = 1
    recv = ser.readline().decode("ascii")

    if not recv:
        print("\033[31m", end="")
        print("未收到版本 ACK")
        print("\033[30m", end="")

        app.tab1_serial_status_label.config(text="串口已打开, 连接失败...")
        app.tab1_serial_status_label.config(foreground="orange")

        app.tab1_apply_button.config(text="Close")
        return
    
    decodeFeedbackMessage(app, recv)

    

def closeSerial():
    global ser
    if ser.is_open:
        ser.close()
        
    print("\033[32m", end="") # 绿色
    if ser.name is None:
        print("串口已关闭")
    else:
        print("串口", ser.name, "已关闭")
    print("\033[30m", end="") # 黑


# 思路是发一个收一个, 由发的函数负责收
