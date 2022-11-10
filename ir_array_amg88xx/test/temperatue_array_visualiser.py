import numpy as np
import seaborn as sns
import serial.tools.list_ports
import matplotlib.pylab as plt

plt.style.use("seaborn")

serialInst = serial.Serial()

def init_uart():
    serialInst.baudrate = 115200
    # setup the serial Port
    serialInst.port = "COM16"
    serialInst.open()

def receive_uart():
    serialInst.write(b'temperature array \n')
    # read back the send package
    serialInst.readline()
    packet = serialInst.readline()
    temperatures_array = str(packet)
    # split the two characters from the begin
    temperatures_array = temperatures_array[2:-1]
    temperatures_array = temperatures_array.split(",")
    # remove the last element because it is a line break
    temperatures_array.pop(64)
    create_thermal_image(temperatures_array)


def create_thermal_image(temps):
    data = np.mat(temps, dtype=float)
    data = data.reshape(8, 8)
    plt.figure(figsize=(8, 8))
    plt.title("HeatMap using Seaborn Method")
    sns.heatmap(data)
    plt.pause(5)
    plt.close()



if __name__ == '__main__':
    init_uart()
    i = 0
    while i<10:
        receive_uart()
        i+=1