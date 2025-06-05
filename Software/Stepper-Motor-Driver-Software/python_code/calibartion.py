import os
from tqdm import tqdm
import time

working_dir = os.path.split(os.path.realpath(__file__))[0]

files_ = []
for _, _, files in os.walk(working_dir):
    for file in files:
        if file.endswith(".csv"):
            files_.append(file)

files = files_
if len(files) == 0:
    print("\033[31m", end="") # 红色
    print("No csv file found.")
    print("\033[30m", end="") # 黑
    exit()
elif len(files) > 1:
    print("\033[31m", end="") # 红色
    print("Multiple csv files found.")
    print("\033[30m", end="") # 黑
    exit()

print("\033[32m", end="") # 绿色
csv_file = files[0]
print("Found csv file: " + csv_file)
# print("\033[30m", end="") # 黑


data = open(os.path.join(working_dir, csv_file), encoding="ascii", errors='ignore').read().split("\n")[:-1]
colomn = data[0].split(",")
title = data[1].split(",")
data = data[2:]

expressionColomn = None
# find "Expression" colomn
for expressionColomn in range(len(colomn)):
    if "Expression" in colomn[expressionColomn]:
        break


expressionWatching = title[expressionColomn]
if expressionWatching != "encoderCalibrationResult":
    print("\033[31m", end="") # 红色
    print("Not a calibration csv file.")
    print("\033[30m", end="") # 黑
    exit()


print("Decoding expression: ", expressionWatching)

if len(data) != 256:
    print("\033[31m", end="") # 红色
    print("Data length is not 256.")
    print("\033[30m", end="") # 黑
    exit()

print("\033[30m", end="") # 黑

result = "int8_t ENCODER_BIAS[257] = {"
# 进度条
for i in tqdm(range(len(data))):
    if i % 16 == 0:
        result += "\n    "
    bias = int(data[i].split(",")[expressionColomn + 2].split(" ")[0])
    result += str(bias).rjust(3, " ") + ", "

result += "\n    " + data[0].split(",")[expressionColomn + 2].split(" ")[0].rjust(3, " ")
result += "\n};"

print(result)