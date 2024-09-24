import pandas as pd
import matplotlib.pyplot as plt

# Read the data from the file with specified encoding
file_path = 'output_2024-09-17 21_13_09.txt'
data = pd.read_csv(file_path, encoding='utf-8')

# Convert the 'Timestamp' column to datetime
data['Timestamp'] = pd.to_datetime(data['Timestamp'])

# Plot the data
plt.figure(figsize=(12, 12))

# Plot IAQ
plt.subplot(3, 2, 1)
plt.plot(data['Timestamp'], data['IAQ'], label='IAQ')
plt.xlabel('Timestamp')
plt.ylabel('IAQ')
plt.title('IAQ over Time')
plt.legend()

# Plot IAQ Accuracy
plt.subplot(3, 2, 2)
plt.plot(data['Timestamp'], data['IAQ accuracy'], label='IAQ Accuracy', color='orange')
plt.xlabel('Timestamp')
plt.ylabel('IAQ Accuracy')
plt.title('IAQ Accuracy over Time')
plt.legend()

# Plot Temperature
plt.subplot(3, 2, 3)
plt.plot(data['Timestamp'], data['Temperature [C]'], label='Temperature [C]')
plt.xlabel('Timestamp')
plt.ylabel('Temperature [C]')
plt.title('Temperature over Time')
plt.legend()

# Plot Pressure
plt.subplot(3, 2, 4)
plt.plot(data['Timestamp'], data['Pressure [hPa]'], label='Pressure [hPa]', color='orange')
plt.xlabel('Timestamp')
plt.ylabel('Pressure [hPa]')
plt.title('Pressure over Time')
plt.legend()

# Plot Humidity
plt.subplot(3, 2, 5)
plt.plot(data['Timestamp'], data['Humidity [%]'], label='Humidity [%]', color='green')
plt.xlabel('Timestamp')
plt.ylabel('Humidity [%]')
plt.title('Humidity over Time')
plt.legend()

# Plot Gas
plt.subplot(3, 2, 6)
plt.plot(data['Timestamp'], data['Gas [Ohm]'], label='Gas [Ohm]', color='red')
plt.xlabel('Timestamp')
plt.ylabel('Gas [Ohm]')
plt.title('Gas over Time')
plt.legend()

plt.tight_layout()
plt.show()