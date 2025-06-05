IaplhaADCBias = 2039.26
IbetaADCBias = 2020.3894
IalphaADCOnCurrent = 1546.75
IbetaADCOnCurrent = 2521.694
VBusADCValue = 2685.42

VBusVoltageRef = 24.00
IalphaADCRef = 1.382
IbetaADCRef = IalphaADCRef

print("IalphaADCGain = ", str(IalphaADCRef / (IalphaADCOnCurrent - IaplhaADCBias))[:12]+"f")
print("IbetaADCGain = ", str(IbetaADCRef / (IbetaADCOnCurrent - IbetaADCBias))[:12]+"f")
print("VBusADCGain = ", str(VBusVoltageRef / VBusADCValue)[:12]+"f")

print("Estimated R: ", VBusVoltageRef * 10 / IalphaADCRef)