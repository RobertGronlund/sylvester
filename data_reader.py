import serial
import keyboard
import time

def read_serial_and_write_to_file(serial_port, baud_rate, output_file):
    # Open the serial port
    ser = serial.Serial(serial_port, baud_rate)
    
    # Open the file in write mode
    with open(output_file, 'w') as file:
        print("Press 'q' to quit.")
        first_line = True
        while True:
            # Read a line of data from the serial port
            if ser.in_waiting > 0:
                line = ser.readline().decode('utf-8', errors='ignore').strip()
                
                if first_line:
                    # Find the position of the timestamp in the first line
                    timestamp_pos = line.find('Timestamp')
                    if timestamp_pos != -1:
                        line = line[timestamp_pos:]
                    first_line = False
                    line = f"Timestamp,{line}"
                else:
                    # Add a timestamp to the data
                    timestamp = time.strftime('%Y-%m-%d %H:%M:%S')
                    line = f"{timestamp},{line}"
                
                # Write the data to the file
                print(line)
                file.write(line + '\n')
            
            # Check if 'q' is pressed to quit
            if keyboard.is_pressed('q'):
                print("Quitting...")
                break
    
    # Close the serial port
    ser.close()

# Example usage
if __name__ == "__main__":
    timestamp = time.strftime('%Y-%m-%d %H_%M_%S')
    read_serial_and_write_to_file('COM3', 9600, f'data/output_{timestamp}.txt')