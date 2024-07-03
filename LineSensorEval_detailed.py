import serial
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import matplotlib.gridspec as gridspec
import time

print("start")

# Set up the serial connection (replace 'COM4' with your actual COM port)
ser = serial.Serial('COM4', 500000)

# Initialize data arrays
data = [0] * 128
faser_counter = 0
width = 0.0

# Set up the plot layout
fig = plt.figure(figsize=(10, 8))
fig.suptitle('LineSensorEval', fontsize=16)
gs = gridspec.GridSpec(2, 2, height_ratios=[1, 1])

ax1 = fig.add_subplot(gs[0, :])  # Plot 1 across the top
ax2 = fig.add_subplot(gs[1, 0])  # Plot 2 on the bottom left
ax3 = fig.add_subplot(gs[1, 1])  # Plot 3 on the bottom right

# Update the plot style for line1 to show individual points clearly
line1, = ax1.plot(data, 'o-', markersize=4, linewidth=1)
line2, = ax2.plot([], [], 'r-')
line3, = ax3.plot([], [], 'b-')

ax1.set_ylim(0, 1050)
ax1.set_xlim(0, 127)
ax1.set_xlabel('Array Index')
ax1.set_ylabel('Analog Value')
ax1.set_title('Live Data from Arduino')

ax2.set_ylim(0, 150)
ax2.set_xlim(0, 10)
ax2.set_ylabel('Faser Counter')
ax2.set_title('Faser Counter')
ax2.tick_params(labelbottom=False)  # Remove x-axis labels
text_faser_counter = ax2.text(0.95, 0.95, '', transform=ax2.transAxes, ha='right', va='top', fontsize=12, bbox=dict(facecolor='white', alpha=0.8))

ax3.set_ylim(0, 5)  # Set y-axis limit from 0 to 5
ax3.set_xlim(0, 10)
ax3.set_ylabel('Width (cm)')
ax3.set_title('Width of Faser Band')
ax3.tick_params(labelbottom=False)  # Remove x-axis labels
text_width = ax3.text(0.95, 0.95, '', transform=ax3.transAxes, ha='right', va='top', fontsize=12, bbox=dict(facecolor='white', alpha=0.8))

fig.tight_layout(pad=2.0)

time_data = []
faser_data = []
width_data = []

def update(frame):
    line1.set_ydata(data)  # Update the data for the array plot

    # Update the data for the faser counter plot
    line2.set_data(time_data, faser_data)
    ax2.set_xlim(max(0, len(time_data) - 10), len(time_data) if len(time_data) > 0 else 10)
    text_faser_counter.set_text(f'Current: {faser_counter}')

    # Update the data for the width plot
    line3.set_data(time_data, width_data)
    ax3.set_xlim(max(0, len(time_data) - 10), len(time_data) if len(time_data) > 0 else 10)
    text_width.set_text(f'Current: {width:.2f} cm')

    ax2.relim()
    ax2.autoscale_view()
    ax3.relim()
    ax3.autoscale_view()

    return line1, line2, line3, text_faser_counter, text_width

def data_gen():
    global data, faser_counter, width, time_data, faser_data, width_data
    while True:
        if ser.in_waiting > 0:
            line = ser.readline().decode('utf-8').strip()
            if line.startswith("Data:"):
                parts = line.split(" FaserCounter: ")
                data_str = parts[0].replace("Data: ", "")
                faser_counter_str, width_str = parts[1].split(" Width: ")

                # Process the data array part
                values = data_str.split(',')
                if len(values) == 128:
                    try:
                        data = [int(v) for v in values]
                    except ValueError:
                        pass

                # Process the faser counter and width
                try:
                    faser_counter = int(faser_counter_str)
                    width = float(width_str)
                except ValueError:
                    pass

                # Update time_data and other variables
                time_data.append(len(time_data))
                faser_data.append(faser_counter)
                width_data.append(width)

                # Limit the time data to the last 100 points to keep the plot window dynamic
                if len(time_data) > 100:
                    time_data.pop(0)
                    faser_data.pop(0)
                    width_data.pop(0)
            time.sleep(0.5)

        yield data

ani = animation.FuncAnimation(fig, update, data_gen, blit=True, interval=100, cache_frame_data=False)
plt.show()
