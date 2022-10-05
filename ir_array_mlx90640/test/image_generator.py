import numpy as np
import seaborn as sns;
import matplotlib.pylab as plt
import serial
plt.style.use("seaborn")

if __name__ == '__main__':
    com_output = []
    temps = []

    # Replace COM19 with the actual COM port your device is located on
    ser = serial.Serial("COM19", 9600)

    data = np.mat(0, dtype=float)

    plt.ion()
    plt.figure(figsize=(24, 32))

    sns.heatmap(data)
    plt.title("MLX90640 image")
    
    plt.show()
    while True:
        com_output = ser.readline().decode("utf-8").split(",")
        if len(com_output) == 769:
            for temp in com_output:
                if temp == '\n':
                    data = np.mat(temps, dtype=float)
                    data = data.reshape(24, 32)
                    plt.clf()
                    sns.heatmap(data)
                    plt.pause(0.25)
                    com_output = []
                    temps = []
                else:
                    temps.append(float(temp))
