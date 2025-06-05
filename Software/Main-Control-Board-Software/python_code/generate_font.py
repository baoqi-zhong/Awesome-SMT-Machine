from PIL import Image, ImageDraw, ImageFont, ImageFilter

font = ImageFont.truetype("consola.ttf", 35)

result = """#include "font.hpp"

const uint8_t font_LUT[] = {0x00, 0x00, 0x52, 0xaa, 0xad, 0x55, 0xff, 0xff};

const uint8_t fontConsolas[] = {\n"""

for num in range(33, 123):
    img = Image.new('RGB', (16, 32), color = 'white')
    draw = ImageDraw.Draw(img)
    draw.text((-1, 0), chr(num), font=font, fill='black')

    img = img.resize((11, 20))
    width, height = img.size

    # img=img.filter(ImageFilter.GaussianBlur(radius=0.5))  

    if chr(num) != '\\':
        result += "// {}\n".format(chr(num))
    for i in range(height):
        for j in range(width):
            r, g, b = img.getpixel((j, i))
            result += str(r) + ", "
            # result += "0x{:02x}, ".format((r << 3) | (g >> 3))
            # result += "0x{:02x}, ".format(((g & 0x07) << 5) | b)
        result += "\n"
    result += "\n"

result += "};"
with open("./User/Drivers/FONT.cpp", "w") as f:
    f.write(result)


# generate LUT
# result = ""
# for i in range(4):
#     brightness = int(i * 255 / 3)
#     r = brightness >> 3
#     g = brightness >> 2
#     b = brightness >> 3
#     result += "0x{:02x}, ".format((r << 3) | (g >> 3))
#     result += "0x{:02x}, ".format(((g & 0x07) << 5) | b)
# print(result)