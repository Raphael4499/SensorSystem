import serial
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import time

print("start")

# Set up the serial connection (replace 'COM4' with your actual COM port)
ser = serial.Serial('COM4', 500000)

# Initialize data array
data = [0] * 128

# Set up the plot
fig, ax = plt.subplots(figsize=(12, 6))
line, = ax.plot(data, 'o-', markersize=4, linewidth=1)

ax.set_ylim(0, 1000)
ax.set_xlim(0, 127)
ax.set_xlabel('Array Index')
ax.set_ylabel('Analog Value')
ax.set_title('Live Data from Arduino')

def update(frame):
    line.set_ydata(data)  # Update the data for the array plot
    print(data)  # Print the array data
    return line,

def data_gen():
    global data
    while True:
        if ser.in_waiting > 0:
            try:
                # Read line from serial port and decode with 'latin1' to avoid decode errors
                line = ser.readline().decode('latin1').strip()
                if line.startswith("Data:"):
                    parts = line.split(" FaserCounter: ")
                    data_str = parts[0].replace("Data: ", "")

                    # Process the data array part
                    values = data_str.split(',')
                    if len(values) == 128:
                        try:
                            data = [int(v) for v in values]
                        except ValueError:
                            pass

            except Exception as e:
                print(f"Error reading from serial port: {e}")

        yield data

ani = animation.FuncAnimation(fig, update, data_gen, blit=True, interval=100, cache_frame_data=False)
plt.show()
