from tkinter import *
from PIL import ImageTk, Image
 
root = Tk()
root.title("展示图片")
 
# 打开图片
image = Image.open("1.jpg")
# 调整图片大小

# 创建图像对象
img = ImageTk.PhotoImage(image)
 
# 创建标签并展示图片
label = Label(root, image=img)
label.pack()
 
root.mainloop()