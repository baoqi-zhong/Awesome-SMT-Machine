import SerialManager
from ZoomDragCanvas import ZoomDragCanvas
from ComponentCanvas import ComponentCanvas
import MovementManager

import tkinter as tk
from tkinter import ttk
from tkinter import filedialog

from PIL import ImageTk, Image

import time
import os
import math
import json


pyFilePath = os.path.dirname(os.path.abspath(__file__))

app = None





class ClosePnp(tk.Tk):
    def __init__(self):
        self.config = {}
        self.ctrlPressed = False

        tk.Tk.__init__(self)
        self.title("ClosePnP @ ELEC3300 G4")

        self.notebook = ttk.Notebook(self)

        # 创建标签页1
        self.tab1 = ttk.Frame(self.notebook)
        self.notebook.add(self.tab1, text="参数设置")


        # COM 口设置
        self.tab1_com_label = ttk.Label(self.tab1, text="COM 口:")
        self.tab1_com_label.place(x=10, y=10)
        self.tab1_com_combo = ttk.Combobox(self.tab1, values=[d.description for d in SerialManager.targetPort], width=25)
        if len(SerialManager.targetPort) > 0:
            self.tab1_com_combo.current(0)
        self.tab1_com_combo.place(x=100, y=10)

        # 波特率设置
        self.tab1_baud_rate_label = ttk.Label(self.tab1, text="波特率:")
        self.tab1_baud_rate_label.place(x=10, y=40)
        self.tab1_baud_rate_entry = ttk.Combobox(self.tab1, values=["9600", "115200"], width=10)
        self.tab1_baud_rate_entry.current(1)
        self.tab1_baud_rate_entry.place(x=100, y=40)

        # 刷新串口按钮
        self.tab1_refresh_button = ttk.Button(self.tab1, text="Refresh", command=self.onRefreshSerialButtonPress)
        self.tab1_refresh_button.place(x=310, y=9)

        # 串口状态
        self.tab1_serial_status_label = ttk.Label(self.tab1, text="串口未打开", foreground="red")
        self.tab1_serial_status_label.place(x=10, y=73)

        # 创建应用按钮
        self.tab1_apply_button = ttk.Button(self.tab1, text="Open", command=lambda: self.onopenCloseSerialButtonPress())
        self.tab1_apply_button.place(x=310, y=70)

        self.targetPositionData = []
        self.selectedComponentData = None

        # 创建标签页2
        self.tab2 = ttk.Frame(self.notebook)
        self.notebook.add(self.tab2, text="贴片")

        # 选择目标文件按钮
        self.tab2_select_target_positions_file_button = ttk.Button(self.tab2, text="打开文件...", command=lambda: self.openTargetPositionsFile())
        self.tab2_select_target_positions_file_button.place(x=10, y=10)

        # 选择目标文件 label
        self.tab2_target_positions_label = ttk.Label(self.tab2, text="坐标文件:")
        self.tab2_target_positions_label.place(x=120, y=13)


        # 选择目标文件 label
        self.tab2_filtter_components_label = ttk.Label(self.tab2, text="筛选元件")
        self.tab2_filtter_components_label.place(x=10, y=53)

        # 筛选元件
        self.filter_resisters_var = tk.BooleanVar()
        self.filter_resisters_var.set("1")
        self.tab2_show_resisters_checkbutton = ttk.Checkbutton(self.tab2, text="电阻", onvalue=True, offvalue=False, variable=self.filter_resisters_var, command=self.onComponentListChange)
        self.tab2_show_resisters_checkbutton.place(x=90, y=50)

        self.filter_capacitors_var = tk.BooleanVar()
        self.filter_capacitors_var.set("1")
        self.tab2_show_capacitors_checkbutton = ttk.Checkbutton(self.tab2, text="电容", onvalue=True, offvalue=False, variable=self.filter_capacitors_var, command=self.onComponentListChange)
        self.tab2_show_capacitors_checkbutton.place(x=150, y=50)

        self.layer_var = tk.IntVar()
        self.layer_var.set(0)
        self.tab2_layer_btn1 = ttk.Radiobutton(self.tab2, text="Top", variable=self.layer_var, value=0, command=self.onComponentListChange)
        self.tab2_layer_btn1.place(x=210, y=50)
        self.tab2_layer_btn2 = ttk.Radiobutton(self.tab2, text="Bottom", variable=self.layer_var, value=1, command=self.onComponentListChange)
        self.tab2_layer_btn2.place(x=270, y=50)



        # 元件列表
        self.tab2_target_positions_list = ttk.Treeview(self.tab2)
        self.tab2_target_positions_list["columns"] = ("Name", "X", "Y", "R", "Layer")
        self.tab2_target_positions_list.column("#0", width=10)
        self.tab2_target_positions_list.column("Name", width=40)
        self.tab2_target_positions_list.column("X", width=40)
        self.tab2_target_positions_list.column("Y", width=40)
        self.tab2_target_positions_list.column("R", width=40)
        self.tab2_target_positions_list.column("Layer", width=50)
        self.tab2_target_positions_list.heading("#0", text="#0")
        self.tab2_target_positions_list.heading("Name", text="Name")
        self.tab2_target_positions_list.heading("X", text="X")
        self.tab2_target_positions_list.heading("Y", text="Y")
        self.tab2_target_positions_list.heading("R", text="R")
        self.tab2_target_positions_list.heading("Layer", text="Layer")
        self.tab2_target_positions_list.bind("<<TreeviewSelect>>", self.onComponentSelectedOnList)
        self.tab2_target_positions_list.place(x=10, y=90, width=400, height=460)

        self.tab2_scrollbar = ttk.Scrollbar(self.tab2, orient="vertical", command=self.tab2_target_positions_list.yview)
        self.tab2_target_positions_list.config(yscrollcommand = self.tab2_scrollbar.set) 
        self.tab2_scrollbar.place(x=410, y=90, width=40, height=460)

        self.tab2_canvas = ZoomDragCanvas(self.tab2, x=450, y=90, width=460, height=460)

        # focus 按钮
        self.tab2_focus_button = ttk.Button(self.tab2, text="捕获键盘", command=self.onFocusButtonPress)
        self.tab2_focus_button.place(x=760, y=600)

        # 控制按钮
        self.tab2_forward_button = ttk.Button(self.tab2, text="前")
        self.tab2_backward_button = ttk.Button(self.tab2, text="后")
        self.tab2_left_button = ttk.Button(self.tab2, text="左")
        self.tab2_right_button = ttk.Button(self.tab2, text="右")

        self.tab2_up_button = ttk.Button(self.tab2, text="上")
        self.tab2_down_button = ttk.Button(self.tab2, text="下")
        self.tab2_CW_button = ttk.Button(self.tab2, text="顺")
        self.tab2_CCW_button = ttk.Button(self.tab2, text="逆")

        self.tab2_forward_button.bind("<ButtonPress>", lambda event: MovementManager.moveForwardOneStep())
        self.tab2_backward_button.bind("<ButtonPress>", lambda event: MovementManager.moveBackwardOneStep())
        self.tab2_left_button.bind("<ButtonPress>", lambda event: MovementManager.moveLeftOneStep())
        self.tab2_right_button.bind("<ButtonPress>", lambda event: MovementManager.moveRightOneStep())

        self.tab2_up_button.bind("<ButtonPress>", lambda event: MovementManager.moveUpOneStep())
        self.tab2_down_button.bind("<ButtonPress>", lambda event: MovementManager.moveDownOneStep())
        self.tab2_CW_button.bind("<ButtonPress>", lambda event: MovementManager.rotateClockwiseOneStep())
        self.tab2_CCW_button.bind("<ButtonPress>", lambda event: MovementManager.rotateCounterClockwiseOneStep())

        self.tab2_forward_button.place(x=50, y=620, width=40, height=40)
        self.tab2_backward_button.place(x=50, y=700, width=40, height=40)
        self.tab2_left_button.place(x=10, y=660, width=40, height=40)
        self.tab2_right_button.place(x=90, y=660, width=40, height=40)

        self.tab2_up_button.place(x=200, y=620, width=40, height=40)
        self.tab2_down_button.place(x=200, y=700, width=40, height=40)
        self.tab2_CW_button.place(x=240, y=660, width=40, height=40)
        self.tab2_CCW_button.place(x=160, y=660, width=40, height=40)


        self.tab2_back_to_origin_button = ttk.Button(self.tab2, text="零点校准")
        self.tab2_back_to_origin_button.place(x=350, y=620)
        self.tab2_back_to_origin_button.bind("<ButtonPress>", lambda event: MovementManager.findOrigin())

        self.tab2_back_to_origin_button = ttk.Button(self.tab2, text="回到原点")
        self.tab2_back_to_origin_button.place(x=350, y=650)
        self.tab2_back_to_origin_button.bind("<ButtonPress>", lambda event: MovementManager.returnToOrigin())

        self.tab2_pump_on_button = ttk.Button(self.tab2, text="打开泵")
        self.tab2_pump_on_button.place(x=350, y=680)
        self.tab2_pump_on_button.bind("<ButtonPress>", lambda event: MovementManager.pumpOn())

        self.tab2_pump_off_button = ttk.Button(self.tab2, text="关闭泵")
        self.tab2_pump_off_button.place(x=350, y=710)
        self.tab2_pump_off_button.bind("<ButtonPress>", lambda event: MovementManager.pumpOff())

        self.tab2_start_SMT_button = ttk.Button(self.tab2, text="开始 SMT")
        self.tab2_start_SMT_button.place(x=450, y=720)
        self.tab2_start_SMT_button.bind("<ButtonPress>", lambda event: MovementManager.startSMT())


        self.tab2_speed_label = ttk.Label(self.tab2, text="进给")
        self.tab2_speed_label.place(x=450, y=620)
        self.tab2_speed_scale = tk.Scale(self.tab2, from_=0, to=100, orient='horizontal', command=lambda value: MovementManager.setFeed(int(value) / 100))
        self.tab2_speed_scale.set(100)
        self.tab2_speed_scale.place(x=500, y=600)

        # 放外面, 不然会被回收
        self.emergencyStopImgPIL = Image.open("./Image/emergencyStop.jpg").resize((150, 150))
        self.reRunImgPIL = Image.open("./Image/reRun.png").resize((150, 150))
        self.emergencyStopImgTK = ImageTk.PhotoImage(self.emergencyStopImgPIL)
        self.reRunImgTK = ImageTk.PhotoImage(self.reRunImgPIL)
        self.tab2_emergency_stop_img = tk.Label(self.tab2, image=self.emergencyStopImgTK, text="Emergency Stop")
        self.tab2_emergency_stop_img.place(x=750, y=630, width=150, height=150)
        self.tab2_emergency_stop_img.bind("<ButtonPress>", lambda event: self.onEmrgencyStopButtonPressed())

        self.tab2.bind("<KeyPress>", self.onKeyPress)
        self.tab2.bind("<KeyRelease>", self.onKeyRelease)
        self.tab2.bind("<FocusOut>", self.onTabLooseFocus)

        # 创建标签页
        self.tab3 = ttk.Frame(self.notebook)
        self.notebook.add(self.tab3, text="校准")

        self.tab3_feeder_calibration_label = ttk.Label(self.tab3, text="飞达校准")
        self.tab3_feeder_calibration_label.place(x=10, y=10)

        # 飞达槽位 1
        self.tab3_feeder1_offsetY = 50
        self.tab3_feeder1_label = ttk.Label(self.tab3, text="槽位 1")
        self.tab3_feeder1_label.place(x=10, y=self.tab3_feeder1_offsetY)

        # 元件类型设置
        self.tab3_feeder1_component_type_label = ttk.Label(self.tab3, text="元件类型")
        self.tab3_feeder1_component_type_label.place(x=70, y=self.tab3_feeder1_offsetY)
        self.tab3_feeder1_component_type_entry = ttk.Combobox(self.tab3, values=["C", "R", "None"], width=10)
        self.tab3_feeder1_component_type_entry.current(1)
        self.tab3_feeder1_component_type_entry.place(x=150, y=self.tab3_feeder1_offsetY)

        self.tab3_feeder1_canvas = ComponentCanvas(self.tab3, x=260, y=self.tab3_feeder1_offsetY - 15, width=580, height=self.tab3_feeder1_offsetY)

        # 飞达槽位 2
        self.tab3_feeder2_offsetY = 150
        self.tab3_feeder2_label = ttk.Label(self.tab3, text="槽位 2")
        self.tab3_feeder2_label.place(x=10, y=self.tab3_feeder2_offsetY)

        self.tab3_feeder2_component_type_label = ttk.Label(self.tab3, text="元件类型")
        self.tab3_feeder2_component_type_label.place(x=70, y=self.tab3_feeder2_offsetY)
        self.tab3_feeder2_component_type_entry = ttk.Combobox(self.tab3, values=["C", "R", "None"], width=10)
        self.tab3_feeder2_component_type_entry.current(1)
        self.tab3_feeder2_component_type_entry.place(x=150, y=self.tab3_feeder2_offsetY)

        self.tab3_feeder2_canvas = ComponentCanvas(self.tab3, x=260, y=self.tab3_feeder2_offsetY - 15, width=580, height=50)

        # 飞达槽位 3
        self.tab3_feeder3_offsetY = 250
        self.tab3_feeder3_label = ttk.Label(self.tab3, text="槽位 3")
        self.tab3_feeder3_label.place(x=10, y=self.tab3_feeder3_offsetY)

        self.tab3_feeder3_component_type_label = ttk.Label(self.tab3, text="元件类型")
        self.tab3_feeder3_component_type_label.place(x=70, y=self.tab3_feeder3_offsetY)
        self.tab3_feeder3_component_type_entry = ttk.Combobox(self.tab3, values=["C", "R", "None"], width=10)
        self.tab3_feeder3_component_type_entry.current(1)
        self.tab3_feeder3_component_type_entry.place(x=150, y=self.tab3_feeder3_offsetY)

        self.tab3_feeder3_canvas = ComponentCanvas(self.tab3, x=260, y=self.tab3_feeder3_offsetY - 15, width=580, height=50)

        # 保存飞达设置
        self.tab3_save_feeder_setting_button = ttk.Button(self.tab3, text="保存设置", command=self.saveFeederConfig)
        self.tab3_save_feeder_setting_button.place(x=10, y=400)

        # 开始校准步骤
        self.tab3_start_calibration_step_button = ttk.Button(self.tab3, text="开始校准步骤", command=lambda: MovementManager.onCalibrationButtonClicked())
        self.tab3_start_calibration_step_button.place(x=10, y=430)

        # 放弃校准
        self.tab3_abort_calibration_step_button = ttk.Button(self.tab3, text="退出校准", command=lambda: MovementManager.onCalibrationAbortButtonClicked())
        self.tab3_abort_calibration_step_button.place(x=10, y=460)

        self.notebook.bind("<<NotebookTabChanged>>", self.onNotebookTabChanged)
        self.notebook.pack(expand=1, fill="both")


    def onComponentListChange(self):
        self.selectedComponentData = None

        self.tab2_target_positions_list.delete(*self.tab2_target_positions_list.get_children())
        self.tab2_canvas.canvas.delete("all")
        layer = self.layer_var.get()
        for line in self.targetPositionData:
            if ((line[4] == "top") and (layer == 0)) or ((line[4] == "bottom") and (layer == 1)):
                if (self.filter_resisters_var.get() and line[0][0] == "R") or (self.filter_capacitors_var.get() and line[0][0] == "C"):
                    self.tab2_target_positions_list.insert("", "end", values=(line[0], line[1], line[2], line[3], line[4]))
                    # 负号是坐标系的问题, 画布的坐标系和元件的坐标系是相反的
                    self.tab2_canvas.add_0603_components(float(line[1]), -float(line[2]), float(line[3]), color="black", fill="white")

    def openTargetPositionsFile(self, fromConfig=False):
        target_positions_file_dir = self.config["positionFileDir"]

        if not fromConfig:
            python_dir = os.path.dirname(os.path.abspath(__file__))
            target_positions_file_dir = filedialog.askopenfilename(initialdir=python_dir + "/production", title="选择坐标文件", filetypes=(("position files", "*.csv"), ("all files", "*.*")))
            if target_positions_file_dir == "":
                return
        
        if not os.path.exists(target_positions_file_dir):
            print("\033[31m", end="")
            print("文件不存在")
            print("\033[30m", end="")
            return
        
        print(target_positions_file_dir)
        target_positions_file_dir_show = target_positions_file_dir
        if len(target_positions_file_dir) > 50:
            target_positions_file_dir_show =  "..." + target_positions_file_dir[-50:]


        self.tab2_target_positions_label.config(text="坐标文件: " + target_positions_file_dir_show)
        self.targetPositionData = []
        with open(target_positions_file_dir, "r") as f:
            lines = f.readlines()[1:]
            for line in lines:
                line = line.strip().split(",")
                if line == "":
                    continue
                self.targetPositionData.append(line)
            
            centerOfMass = [0, 0]
            for line in self.targetPositionData:
                centerOfMass[0] += float(line[1])
                centerOfMass[1] += float(line[2])
            centerOfMass[0] /= len(self.targetPositionData)
            centerOfMass[1] /= len(self.targetPositionData)

            centerOfMass[0] = int(centerOfMass[0]) # 取整, 防止出现怪小数
            centerOfMass[1] = int(centerOfMass[1])

            for i in range(len(self.targetPositionData)):
                self.targetPositionData[i][1] = float(self.targetPositionData[i][1]) - centerOfMass[0]
                self.targetPositionData[i][2] = float(self.targetPositionData[i][2]) - centerOfMass[1]

            self.targetPositionData[i][3] = float(self.targetPositionData[i][3])
        self.selectedComponentData = None
        
        # 保存配置
        self.config["positionFileDir"] = os.path.relpath(path=target_positions_file_dir, start=pyFilePath)
        self.saveConfig()
        
        print("\033[32m", end="")
        print("坐标文件读取成功")
        print("找到", len(self.targetPositionData), "个元件")
        print("\033[30m", end="")

        self.onComponentListChange()


    def onComponentSelectedOnList(self, event):
        selected = self.tab2_target_positions_list.selection()

        if len(selected) == 0:
            return
        
        # 清除之前的选择
        if(self.selectedComponentData):
            for item in self.selectedComponentData:
                self.tab2_canvas.add_0603_components(float(item[1]), -float(item[2]), float(item[3]), color="black", fill="white")

        self.selectedComponentData = [self.tab2_target_positions_list.item(selected[i])["values"]  for i in range(len(selected))]
        
        # 重新绘制选择
        for item in self.selectedComponentData:
            self.tab2_canvas.add_0603_components(float(item[1]), -float(item[2]), float(item[3]), "red", fill="red")

    # Config
    def updateFilterFromConfig(self):
        self.filter_resisters_var.set(self.config["filterResisters"])
        self.filter_capacitors_var.set(self.config["filterCapacitors"])
        self.layer_var.set(self.config["layer"])

    def updatefeederFromConfig(self):
        if "feeder1" in self.config:
            self.tab3_feeder1_component_type_entry.set(self.config["feeder1"]["componentType"])
            self.tab3_feeder1_canvas.setComponentAvailable(self.config["feeder1"]["componentAvailableStatus"])

        if "feeder2" in self.config:
            self.tab3_feeder2_component_type_entry.set(self.config["feeder2"]["componentType"])
            self.tab3_feeder2_canvas.setComponentAvailable(self.config["feeder2"]["componentAvailableStatus"])

        if "feeder3" in self.config:
            self.tab3_feeder3_component_type_entry.set(self.config["feeder3"]["componentType"])
            self.tab3_feeder3_canvas.setComponentAvailable(self.config["feeder3"]["componentAvailableStatus"])

    def saveFeederConfig(self):
        if "feeder1" not in self.config:
            self.config["feeder1"] = {}
        self.config["feeder1"]["componentType"] = self.tab3_feeder1_component_type_entry.get()
        self.config["feeder1"]["componentAvailableStatus"] = self.tab3_feeder1_canvas.getComponentAvailable()

        if "feeder2" not in self.config:
            self.config["feeder2"] = {}
        self.config["feeder2"]["componentType"] = self.tab3_feeder2_component_type_entry.get()
        self.config["feeder2"]["componentAvailableStatus"] = self.tab3_feeder2_canvas.getComponentAvailable()

        if "feeder3" not in self.config:
            self.config["feeder3"] = {}
        self.config["feeder3"]["componentType"] = self.tab3_feeder3_component_type_entry.get()
        self.config["feeder3"]["componentAvailableStatus"] = self.tab3_feeder3_canvas.getComponentAvailable()

        self.saveConfig()

    def readConfig(self, configFileDir = "./config.json"):
        # 读取配置
        if os.path.exists(configFileDir):
            with open(configFileDir, "r") as configFile:
                self.config = json.load(configFile)
            print("\033[32m", end="")
            print("配置读取成功")
            print("\033[30m", end="")
        else:
            print("\033[31m", end="")
            print("配置文件不存在, 使用默认配置")
            print("\033[30m", end="")
            self.creatDefaultConfigFile()

    def saveConfig(self, configFileDir = "./config.json"):
        with open(configFileDir, "w") as configFile:
            json.dump(self.config, configFile)
        
        print("\033[32m", end="")
        print("配置保存成功")
        print("\033[30m", end="")

    def creatDefaultConfigFile(self):
        self.config = json.loads(r'{}')
        self.config["baudRate"] = 115200
        self.config["positionFileDir"] = ""

        self.config["filterResisters"] = True
        self.config["filterCapacitors"] = True
        self.config["layer"] = 0

        self.config["feeder1"] = {}
        self.config["feeder1"]["componentType"] = "None"
        self.config["feeder1"]["componentAvailableStatus"] = [False] * 27

        self.config["feeder2"] = {}
        self.config["feeder2"]["componentType"] = "None"
        self.config["feeder2"]["componentAvailableStatus"] = [False] * 27

        self.config["feeder3"] = {}
        self.config["feeder3"]["componentType"] = "None"
        self.config["feeder3"]["componentAvailableStatus"] = [False] * 27

        self.config["feederZeroPosition"] = [[0, 0]] * 3         # 第一个元件的坐标
        self.config["fedderComponentDistance"] = [[0, 0]] * 3    # 相邻元件之间的距离

        self.config["PCBLeftUpComponentPosition"] = [0, 0]        # PCB 左上角元件的坐标
        self.config["PCBRightDownComponentPosition"] = [0, 0]     # PCB 右下角元件的坐标
        self.saveConfig()

    # button handler
    def onRefreshSerialButtonPress(self):
        SerialManager.refreshSerialPorts()
        self.tab1_com_combo["values"] = [d.description for d in SerialManager.targetPort]
        if SerialManager.targetPort:
            self.tab1_com_combo.current(0)
        else:
            self.tab1_com_combo.set("")

    def onopenCloseSerialButtonPress(self):
        if self.tab1_apply_button["text"] == "Open":
            portIndex = self.tab1_com_combo.current()
            baudrate = int(self.tab1_baud_rate_entry.get())
            SerialManager.openSerial(self, portIndex, baudrate)
            self.config["baudRate"] = baudrate
            self.saveConfig()
        else:
            SerialManager.closeSerial()
            self.tab1_apply_button.config(text="Open")
            self.tab1_serial_status_label.config(text="串口未打开", foreground="red")

    def onNotebookTabChanged(self, event):
        pass
        # if self.notebook.index("current") == 1:
        #     self.tab2.focus_set()

    def onFocusButtonPress(self):
        # self.tab2_focus_button.config(state="disabled")
        self.tab2.focus_set()

    def onTabLooseFocus(self, event):
        pass
        # self.tab2_focus_button.config(state="normal")


    def onEmrgencyStopButtonPressed(self):
        if(self.tab2_emergency_stop_img["text"] == "Emergency Stop"):
            MovementManager.emrgencyStop(self)
        else:
            MovementManager.recoverFromEmergencyStop(self)

    # key handler
    def onKeyPress(self, event):
        key = event.keysym
        if key == "Control_L":
            self.ctrlPressed = True
            return

        elif key == "space":
            MovementManager.emrgencyStop(app)
            return

        if not self.ctrlPressed:
            if key == "Up":
                self.tab2_forward_button.state(['pressed'])
                MovementManager.moveForwardOneStep()
            elif key == "Down":
                self.tab2_backward_button.state(['pressed'])
                MovementManager.moveBackwardOneStep()
            elif key == "Left":
                self.tab2_left_button.state(['pressed'])
                MovementManager.moveLeftOneStep()
            elif key == "Right":
                self.tab2_right_button.state(['pressed'])
                MovementManager.moveRightOneStep()
        else:
            if key == "Up":
                self.tab2_up_button.state(['pressed'])
                MovementManager.moveUpOneStep()
            elif key == "Down":
                self.tab2_down_button.state(['pressed'])
                MovementManager.moveDownOneStep()
            elif key == "Left":
                self.tab2_CCW_button.state(['pressed'])
                MovementManager.rotateCounterClockwiseOneStep()
            elif key == "Right":
                self.tab2_CW_button.state(['pressed'])
                MovementManager.rotateClockwiseOneStep()


    def onKeyRelease(self, event):

        key = event.keysym
        if key == "Control_L":
            self.ctrlPressed = False
            return

        if not self.ctrlPressed:
            if key == "Up":
                self.tab2_forward_button.state(['!pressed'])
            elif key == "Down":
                self.tab2_backward_button.state(['!pressed'])
            elif key == "Left":
                self.tab2_left_button.state(['!pressed'])
            elif key == "Right":
                self.tab2_right_button.state(['!pressed'])
        else:
            if key == "Up":
                self.tab2_up_button.state(['!pressed'])
            elif key == "Down":
                self.tab2_down_button.state(['!pressed'])
            elif key == "Left":
                self.tab2_CCW_button.state(['!pressed'])
            elif key == "Right":
                self.tab2_CW_button.state(['!pressed'])


if __name__ == "__main__":
    
    SerialManager.refreshSerialPorts()

    app = ClosePnp()
    app.iconbitmap('./Image/ENTERPRIZE_LOGO.ico')
    app.geometry("940x800")
    from MovementManager import init
    init(app)

    app.readConfig()
    app.openTargetPositionsFile(fromConfig=True)
    app.updatefeederFromConfig()
    app.updateFilterFromConfig()
    from MovementManager import getMostLeftUpComponentIndexOnPCB
    getMostLeftUpComponentIndexOnPCB()
    
    app.mainloop()
    SerialManager.closeSerial()
