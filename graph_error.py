import serial
import matplotlib.pyplot as plt

# Set the correct serial port and baud rate
ser = serial.Serial('COM3', 9600)  # Change 'COM3' to your Arduino's port

# Initialize an empty list to store data
data = []

try:
    plt.ion()  # Turn on interactive mode for real-time plotting

    # Create a figure and axis for the plot
    fig, ax = plt.subplots()

    # Set up the plot
    ax.set_title('Sensor Data Plot')
    ax.set_xlabel('Time')
    ax.set_ylabel('Sensor Reading')

    while True:
        # Read a line of serial data
        line = ser.readline().decode('utf-8').rstrip()

        # Convert the data to an integer (replace with your own parsing logic)
        sensor_reading = int(line)

        # Append the data to the list
        data.append(sensor_reading)

        # Plot the data
        ax.plot(data, color='blue')
        plt.pause(0.01)  # Pause to update the plot

except KeyboardInterrupt:
    # Close the serial connection on Ctrl+C
    ser.close()
    print("Serial connection closed.")
finally:
    plt.ioff()  # Turn off interactive mode
    plt.show()  # Display the final plot when the script is closed
