import sys
import time
import serial
import threading

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
                    except KeyboardInterrupt:
                        self.runThread = False
    
        def getData(self):
            return self._data
        
        def terminate(self):
            self.runThread = False
    
    
    thread = MainTHread()
    thread.start()
    
    try:
        while True:
            time.sleep(0.1)
    except KeyboardInterrupt:
        thread.terminate()