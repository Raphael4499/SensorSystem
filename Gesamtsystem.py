import serial
import csv
import time
from datetime import datetime
import os

# Variablen, um den Doppelmodus und die Konsolenausgabe zu aktivieren
enableDoubleMode = False
printData = True

# Serielle Verbindung zum Arduino herstellen (passen Sie den Port und die Baudrate an)
ser1 = serial.Serial('COM3', 500000, timeout=1)
ser2 = None

if enableDoubleMode:
    ser2 = serial.Serial('COM4', 500000, timeout=1)

save_directory = r"C:\Users\Administrator\Desktop\PyTools\RUNS"  # Change to your save directory
if not os.path.exists(save_directory):
    os.makedirs(save_directory)
filename = os.path.join(save_directory, f"arduino_data_{datetime.now().strftime('%Y%m%d_%H%M%S')}.csv")
print("Dateiname:", filename)

# CSV-Datei zum Schreiben öffnen
with open(filename, mode='w', newline='') as file:
    writer = csv.writer(file)
    writer.writerow(["Timestamp (ms)", "A", "B", "C", "D", "E", "Err"])  # Kopfzeile schreiben

    start_time = time.time()

    # Initiale Werte für jede Spalte
    data = {
        "A": 0.0,
        "B": 0.0,
        "C": 0.0,
        "D": 0.0,
        "E": 0.0,
        "Err": "0"
    }

    received_data = {
        "A": False,
        "B": False,
        "C": False,
        "D": False,
        "E": False,
        "Err": False
    }

    def process_line(line, data, received_data):
        parts = line.split(',')
        if len(parts) == 2:
            sensor = parts[0]
            wert = parts[1]

            # Sensorwert in das Dictionary einfügen und als empfangen markieren
            if sensor in data:
                if sensor == "Err":
                    data[sensor] = wert  # Err kann jetzt auch String-Werte enthalten
                else:
                    try:
                        data[sensor] = float(wert)
                        received_data[sensor] = True
                    except ValueError:
                        print(f"Ungültiger Wert für {sensor}: {wert}")

    try:
        while True:
            # Zeile von der ersten seriellen Verbindung lesen
            line1 = ser1.readline().decode('utf-8').strip()
            if line1:
                process_line(line1, data, received_data)

            # Wenn der Doppelmodus aktiviert ist, Zeile von der zweiten seriellen Verbindung lesen
            if enableDoubleMode and ser2:
                line2 = ser2.readline().decode('utf-8').strip()
                if line2:
                    process_line(line2, data, received_data)

            # Überprüfen, ob alle relevanten Sensorwerte empfangen wurden
            if all(received_data[key] for key in ["B", "C", "D", "E"]):
                # Zeitstempel für die CSV-Datei berechnen
                timestamp = int((time.time() - start_time) * 1000)

                # Daten in die CSV-Datei schreiben
                writer.writerow([timestamp, data["A"], data["B"], data["C"], data["D"], data["E"], data["Err"]])
                if printData:
                    print(f"Timestamp: {timestamp}, A: {data['A']}, B: {data['B']}, C: {data['C']}, D: {data['D']}, E: {data['E']}, Err: {data['Err']}")

                # Empfangsmarkierungen zurücksetzen
                for key in received_data:
                    received_data[key] = False

            time.sleep(0.001)  # Kurz warten, um die CPU-Last zu reduzieren

    except KeyboardInterrupt:
        # Bei Tastaturunterbrechung die Schleife beenden
        print("Beenden der Datenerfassung...")
    finally:
        # Serielle Verbindungen schließen
        ser1.close()
        if enableDoubleMode and ser2:
            ser2.close()
