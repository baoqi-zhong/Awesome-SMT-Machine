"""
Author: Baoqi (zzhongas@connect.ust.hk)
"""


import wave
import struct

# 打开WAV文件
wav_file = wave.open(r'./3min.wav', 'rb')

# 获取WAV文件的参数
channels = wav_file.getnchannels()
sample_width = wav_file.getsampwidth()
frame_rate = wav_file.getframerate()
print(frame_rate)
# 读取所有的音频帧
frames = wav_file.readframes(wav_file.getnframes())[frame_rate * 10:]

# 关闭WAV文件
wav_file.close()

length = len(frames)
if length > 44100 * 2:
    length = 44100 * 2

# 将二进制数据转换为int16_t格式的数据
data = []
for i in range(0, length, 2):
    sample = struct.unpack('<h', frames[i:i+2])[0]  # 使用小端模式解包int16_t数据
    data.append(sample)

result = ""
count = 0
for sample in data:
    result += "{:>5}, ".format(sample)
    if count % 441 == 440:
        result += "\n"
    count += 1
print(count)
open(r'./3min.txt', 'w').write(result)
