import os
import json
import struct
import serial
import serial.tools.list_ports

class SerialCommunicater:
    def __init__(self,config_json, print_log = False):
        self.print_log = print_log
        
        with open(os.path.join(os.getcwd(), config_json), "r") as config:
            self.serial_info = json.load(config)
        self.serial = serial.Serial()
        
        if "port" in self.serial_info:
            self.serial.port = self.serial_info["port"]
        else:
            device = [port.device for port in serial.tools.list_ports.comports() if port.serial_number == self.serial_info.get("seiralnumber","")]
            if len(device):
                self.serial.port = device[0]
            else:
                ...
        self.serial.baudrate = self.serial_info["baudrate"]
        if "timeout" in self.serial_info:
            self.serial.timeout = self.serial_info["timeout"]
        
        if "send_data" in self.serial_info:
            self.serial_info["send_format"] = "<"+"".join(self.serial_info["send_datatype"].values())
        
        if "receive_data" in self.serial_info:
            self.serial_info["receive_format"] = "<"+"".join(self.serial_info["receive_datatype"].values())
        
    def begin(self):
        try:
            self.serial.open()
            print(self.serial.read(310))#trash from esp
        except serial.SerialException as e:
            print(f"error:{e}")
    
    def send_dict(self):
        if "send_data" in self.serial_info:
            self.serial.write(data:=struct.pack(
                    self.serial_info["send_format"],
                    *list(map(int,self.serial_info["send_data"].values()))
            ))
        else:
            return False
        
    def receive_dict(self):
        if "receive_data" in self.serial_info:
            data_bytes = self.serial.read(struct.calcsize(self.serial_info["receive_format"]))
            print("receive",data_bytes)
            self.serial_info["receive_data"] = dict(zip(self.serial_info["receive_data"].keys(),
                struct.unpack(self.serial_info["receive_format"],data_bytes)
            ))
        else:
            return False

    def receive_dict_(self):
        if not "receive_data" in self.serial_info:return False
        if self.serial.in_waiting >= 14:
            if (head:=self.serial.read(2)) == b'\xAA\xBB':
                data_bytes = self.serial.read(struct.calcsize(self.serial_info["receive_format"]))
                print("receive",data_bytes)
                self.serial_info["receive_data"] = dict(zip(self.serial_info["receive_data"].keys(),
                    struct.unpack(self.serial_info["receive_format"],data_bytes)
                ))
            else:
                self.serial.read(1)
            print(head)
            

import pygame,sys

def main():
    pygame.init()
    pygame.display.set_mode((200,200))
    ser = SerialCommunicater("ESP32.json",True)
    ser.begin()
    while ser.serial.is_open:
        ser.send_dict()
        ser.receive_dict()
        for event in pygame.event.get():
            if event.type == pygame.KEYDOWN:
                ser.serial_info["send_data"]["angle"] = 1
            if event.type == pygame.QUIT:
                ser.serial.close()
                pygame.quit()
                sys.exit()
        pygame.display.update()


if __name__ == "__main__":
    main()