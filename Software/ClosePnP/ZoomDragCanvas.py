import tkinter as tk
from tkinter import ttk

import math

class ZoomDragCanvas:
    def __init__(self, parent, x, y, width=400, height=400):
        self.parent = parent
        self.canvas = tk.Canvas(self.parent, width=width, height=height, bg="white")
        self.canvas.place(x=x, y=y)

        self.canvas.bind("<ButtonPress-1>", self.onMousePress)
        self.canvas.bind("<B1-Motion>", self.onMouseMovde)
        self.canvas.bind("<MouseWheel>", self.onMouseWheel)

        self.canvasX = 0  # 画布原点坐标
        self.canvasY = 0
        self.componentX = self.canvasX + width // 2  # 元件原点坐标
        self.componentY = self.canvasY + height // 2
        self.onPressDeltaX = 0
        self.onPressDeltaY = 0
        self.scaleFactor = 5

        self._0603_width = 1.6
        self._0603_height = 0.8
        self._0805_width = 2.0
        self._0805_height = 1.2

    def onMousePress(self, event):
        self.onPressDeltaX = event.x - self.canvasX
        self.onPressDeltaY = event.y - self.canvasY

    def onMouseMovde(self, event):
        self.canvasX = event.x - self.onPressDeltaX
        self.canvasY = event.y - self.onPressDeltaY
        self.canvas.scan_dragto(self.canvasX, self.canvasY, gain=1)

    def onMouseWheel(self, event):
        scale = 1.1 if event.delta > 0 else 0.9
        self.scaleFactor *= scale
        # self.canvasX = (self.canvasX - event.x) * scale + event.x
        # self.canvasY = (self.canvasY - event.y) * scale + event.y
        scaleCenterX = event.x - self.canvasX
        scaleCenterY = event.y - self.canvasY
        self.canvas.scale("all", scaleCenterX, scaleCenterY, scale, scale)

        # 在调用 canvas.scale 的时候缩放的是所有的元素, 但是画布的原点坐标并没有变化
        # 所以要计算缩放之后元件的原点坐标, 以便在后续添加元件的时候按照元件缩放后的坐标添加
        self.componentX = (self.componentX - scaleCenterX) * scale + scaleCenterX
        self.componentY = (self.componentY - scaleCenterY) * scale + scaleCenterY

    def draw_rectangle(self, x1, y1, x2, y2, color="black", fill="white"):
        x1 *= self.scaleFactor
        y1 *= self.scaleFactor
        x2 *= self.scaleFactor
        y2 *= self.scaleFactor
        x1 += self.componentX
        y1 += self.componentY
        x2 += self.componentX
        y2 += self.componentY
        self.canvas.create_rectangle(x1, y1, x2, y2, outline=color, width=2, fill=fill)

    def add_component(self, x, y, width, height, rotation, color="black", fill="white"):
        # 计算四个角的坐标
        if rotation == 0 or rotation == 180:
            x1 = x - width / 2
            y1 = y - height / 2
            x2 = x + width / 2
            y2 = y + height / 2
        elif rotation == 90 or rotation == 270:
            x1 = x - height / 2
            y1 = y - width / 2
            x2 = x + height / 2
            y2 = y + width / 2
        else:
            
            x1 = x - width / 2 * math.cos(math.radians(rotation)) - height / 2 * math.sin(math.radians(rotation))
            y1 = y - width / 2 * math.sin(math.radians(rotation)) + height / 2 * math.cos(math.radians(rotation))
            x2 = x + width / 2 * math.cos(math.radians(rotation)) + height / 2 * math.sin(math.radians(rotation))
            y2 = y + width / 2 * math.sin(math.radians(rotation)) - height / 2 * math.cos(math.radians(rotation))
        self.draw_rectangle(x1, y1, x2, y2, color, fill)

    def add_0603_components(self, x, y, rotation, color="black", fill="black"):
        self.add_component(x, y, self._0603_width, self._0603_height, rotation, color=color, fill=fill)
    
    def add_0805_components(self, x, y, rotation, color="black", fill="black"):
        self.add_component(x, y, self._0805_width, self._0805_height, rotation, color=color, fill=fill)
