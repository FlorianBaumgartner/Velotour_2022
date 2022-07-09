import os
import sys
import time
import threading
try:
    import serial
    import serial.tools.list_ports
except ModuleNotFoundError:
    print("USB Modules not found, try to install them...")
    import pip
    def install(package):
        if hasattr(pip, 'main'):
            pip.main(['install', package])
        else:
            pip._internal.main(['install', package])

    install("serial")
    install("pyserial")
    import serial
    import serial.tools.list_ports
    print("USB Modules successfully installed and imported!")


class Port:
    def __init__(self, port, vid, pid, ser):
        self.port = port
        self.vid = vid
        self.pid = pid
        self.ser = ser
    
    def __str__(self):
        return f"{self.port} (VID: {self.vid:04X}, PID: {self.pid:04X}, SER: {self.ser})"


def getPorts():
    ports = []
    for port, desc, hwid in sorted(serial.tools.list_ports.comports()):
        try:
            vid = int(hwid.split("PID=")[1].split(":")[0], base=16)
            pid = int(hwid.split("PID=")[1].split(":")[1].split(" ")[0], base=16)
            ser = hwid.split("SER=")[1].split(" ")[0]
            ports.append(Port(port, vid, pid, ser))
        except:
            pass
    return ports

ports = getPorts()
for p in ports:
    print(p)

print(f"Process ID: {os.getpid()}")

if(len(sys.argv) <= 1):
    print("Please specifiy a COM-Port, for example: COM34")
else:
    comPort = sys.argv[1]
    print(f"Start Console on port {comPort}\n")
    
    class MainTHread(threading.Thread):
        def __init__(self):
            threading.Thread.__init__(self)
            self._data = ""
            self.ser = None
            self.runThread = True
            
        def run(self):
            while(self.runThread):
                try:
                    time.sleep(1)
                    ser = serial.Serial(comPort,9600,timeout=2)
    
                except serial.SerialException:
                    try:
                        ser.close()
                    except UnboundLocalError:
                        pass
                    continue
                except KeyboardInterrupt:
                    self.runThread = False
                    
                while self.runThread:
                    try:
                        data_json = ser.readline()
                        self._data =data_json.decode('UTF-8');
                        if(not self._data):
                            break
                        print(self._data, end="")
                        
                    except serial.SerialException:
                        ser.close()
                        break
                    except UnicodeDecodeError:  # Ignore characters that cannot be printed
                        pass
                    except KeyboardInterrupt:
                        self.runThread = False
    
        def getData(self):
            return self._data
        
        def terminate(self):
            self.runThread = False
    
    
    ports = serial.tools.list_ports.comports()
    for port, desc, hwid in sorted(ports):
            print("{}: {} [{}]".format(port, desc, hwid))
    
    thread = MainTHread()
    thread.start()
    
    try:
        while True:
            time.sleep(0.1)
    except KeyboardInterrupt:
        thread.terminate()