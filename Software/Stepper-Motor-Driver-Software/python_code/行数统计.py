import os


working_dir = os.path.split(os.path.realpath(__file__))[0]
root_dir = os.path.abspath(os.path.join(working_dir, ".."))

filesToCount = [
    "Core/Inc/GeneralConfig.h",

    "Core/Src/Calibrator.c",
    "Core/Inc/Calibrator.h",

    "Core/Src/CordicHelper.c",
    "Core/Inc/CordicHelper.h",

    "Core/Src/ErrorHandler.c",
    "Core/Inc/ErrorHandler.h",

    "Core/Src/FDCANManager.c",
    "Core/Inc/FDCANManager.h",

    "Core/Src/IncrementalPID.c",
    "Core/Inc/IncrementalPID.h",

    "Core/Src/InterBoard.c",
    "Core/Inc/InterBoard.h",

    "Core/Src/KalmanFilter.c",
    "Core/Inc/KalmanFilter.h",

    "Core/Src/LPF.c",
    "Core/Inc/LPF.h",

    "Core/Src/MA732.c",
    "Core/Inc/MA732.h",

    "Core/Src/Matrix.c",
    "Core/Inc/Matrix.h",

    "Core/Src/MusicManager.c",
    "Core/Inc/MusicManager.h",

    "Core/Src/PositionalPID.c",
    "Core/Inc/PositionalPID.h",

    "Core/Src/statisticsCalculator.c",
    "Core/Inc/statisticsCalculator.h",

    "Core/Src/tick.c",
    "Core/Inc/tick.h",

    "Core/Src/ws2812.c",
    "Core/Inc/ws2812.h",
]

total_lines = 0
for file in filesToCount:
    with open(os.path.join(root_dir, file), "r", encoding="ascii", errors="ignore") as f:
        lines = f.readlines()
        total_lines += len(lines)

print("Total lines: ", total_lines)