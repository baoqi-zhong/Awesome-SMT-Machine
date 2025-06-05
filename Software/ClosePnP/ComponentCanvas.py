import tkinter as tk

import math

class ComponentCanvas:
    def drawComponent(self, Xn, fill="white"):
        offsetX = self.offsetX + 20
        offsetY = self.offsetX + 17.5
        self.canvas.create_rectangle((offsetX + 20 - self._0603_height / 2 + Xn * 40) * self.scale,( offsetY + 35 - self._0603_width / 2) * self.scale, (offsetX + 20 + self._0603_height / 2 + Xn * 40) * self.scale, (offsetY + 35 + self._0603_width / 2) * self.scale, fill=fill, outline="black")
                    

    def drawComponents(self):
        self.canvas.create_rectangle(self.offsetX * self.scale, self.offsetY * self.scale, (self.offsetX + len(self.componentAvailable) * 40 + 40) * self.scale, (self.offsetY + 80) * self.scale, fill="white", outline="black")
        
        offsetX = self.offsetX + 20
        offsetY = self.offsetX + 17.5
        for i in range(len(self.componentAvailable) + 1):
            self.canvas.create_oval((offsetX + -8 + i * 40) * self.scale, (offsetY - 8) * self.scale, (offsetX + 8 + i * 40) * self.scale, (offsetY + 8) * self.scale, outline='black', fill='white')

        for i in range(len(self.componentAvailable)):
            if self.componentAvailable[i]:
                self.drawComponent(i, "black")
            else:
                self.drawComponent(i)

    def drawSelectedBar(self, Xn, color="blue"):
        offsetX = self.offsetX + 20
        offsetY = self.offsetX + 17.5

        barWidth = 20
        self.canvas.create_line((offsetX + 20 - barWidth / 2 + Xn * 40) * self.scale, (offsetY + 45 + self._0603_width / 2) * self.scale, (offsetX + 20 + barWidth / 2 + Xn * 40) * self.scale, (offsetY + 45 + self._0603_width / 2) * self.scale, fill=color, width=2)


    def onMouseMove(self, event):
        if not self.allowModify:
            return
        Xn = (event.x - self.offsetX - 10) // 20
        if(event.y > self.offsetY and event.y < self.height - 2) and (Xn >= 0 and Xn < len(self.componentAvailable)):
            if self.lastSelectedComponentIndex != Xn:
                if self.currentBrush == "add":
                    self.drawSelectedBar(Xn, color="green")
                elif self.currentBrush == "del":
                    self.drawSelectedBar(Xn, color="red")
                else:
                    self.drawSelectedBar(Xn, color="black")

                if self.mousePressed:
                    if self.currentBrush == "del":
                        self.drawComponent(Xn, "white")
                        self.componentAvailable[Xn] = False
                    else:
                        self.drawComponent(Xn, "black")
                        self.componentAvailable[Xn] = True

                if self.lastSelectedComponentIndex != -1:
                    self.drawSelectedBar(self.lastSelectedComponentIndex, color="white")
                
                self.lastSelectedComponentIndex = Xn

        else:
            # 清除上一次选中的元件
            if self.lastSelectedComponentIndex != -1:
                self.drawSelectedBar(self.lastSelectedComponentIndex, color="white")
                self.lastSelectedComponentIndex = -1

    def onMousePress(self, event, brush="del"):
        self.mousePressed = True
        self.currentBrush = brush
        self.lastSelectedComponentIndex = -1
        self.onMouseMove(event)
    
    def onMouseRelease(self, event):
        self.mousePressed = False
        self.currentBrush = "none"

    def getComponentAvailable(self):
        return self.componentAvailable

    def setComponentAvailable(self, componentAvailable):
        self.componentAvailable = componentAvailable
        self.drawComponents()

    def allowModify(self):
        self.allowModify = True

    def disallowModify(self):
        self.allowModify = False

        # 清除上一次选中的元件
        if self.lastSelectedComponentIndex != -1:
            self.drawSelectedBar(self.lastSelectedComponentIndex, color="white")
            self.lastSelectedComponentIndex = -1

    def __init__(self, parent, x, y, width=600, height=50):
        self.parent = parent
        self.x = x
        self.y = y
        self.width = width
        self.height = height
        self.canvas = tk.Canvas(self.parent, width=width, height=height, bg="white")
        self.canvas.place(x=x, y=y)

        self.offsetX = 16
        self.offsetY = 16
        self.scale = 0.5

        self._0603_width = 16
        self._0603_height = 8
        self._0805_width = 20
        self._0805_height = 12
        self.componentAvailable = [True] * 27

        self.allowModify = True
        self.mousePressed = False
        self.currentBrush = "none"
        self.lastSelectedComponentIndex = -1

        self.drawComponents()

        # 绑定鼠标事件
        self.canvas.bind('<Motion>', self.onMouseMove)
        self.canvas.bind('<Leave>', self.onMouseMove)

        self.canvas.bind("<ButtonPress-1>", lambda event: self.onMousePress(event, "add"))
        self.canvas.bind("<ButtonPress-3>", lambda event: self.onMousePress(event, "del"))
        self.canvas.bind("<ButtonRelease-1>", self.onMouseRelease)
        self.canvas.bind("<ButtonRelease-3>", self.onMouseRelease)

