import time
from machine import UART
"""  NOTE:- if UART_no is 2 then TX pin will be at pin17 of esp32 dev module and Rx is pin16
            if UART_no is 1 then RX, Tx pin is standard pin of esp32 board.
            
            # connect Rx pin of arduino with Tx pin of esp32 dev module (pin17)
            # connect Tx pin of arduino with Rx pin of es32 dev module (Pin16)
            # connect ground pin of arduino with ground pin of esp32 """

"""  Commands:- To calibrate the load cell press-----> y or Y.
                To skip calibration press any alphanumeric key
                For unit conversion press------------> k or ."""
baudrate = 57600
UART_no = 2

# Define the UART settings
uart = UART(UART_no, baudrate)  # UART2, baud rate 57600
time.sleep(2)  # Wait for the UART to initialize

# Send a command to the Arduino and read the response
def send_command_to_arduino(command):
    uart.write(command)
    while True:
        if uart.any():
            data = uart.read()
            if data == b'A':
                print("Calibration aborted due to no valid weight entered.")
                break
            elif data == b'B':
                print("Load cell is not found")
                break
            elif data == b'D':
                print("Place the weight on the Load cell")
                time.sleep(3)
                while True:
                    user_input = input("Enter the weight that you have placed on the load cell  ").strip()
                    try:
                        user_input = float(user_input)
                        uart.write(str(user_input).encode())
                        print("Remove weight from the load cell")
                        time.sleep(2)
                        return
                    except ValueError:
                        print("Invalid input. Please enter a valid floating-point number for calibration (in kg).")
                        time.sleep(1)
                        
            else:
                print("Received unknown data:", data.decode())
        

# Prompt the user to enter 'Y' or 'y'
user_input = input("Do you want to calibrate? (Y/N): ").strip()

if user_input.lower() == 'y':
    # Send 'Y' to the Arduino to initiate calibration
    send_command_to_arduino('Y'.encode())
    user_input = input("To view weight in kg, press K press N to skip ")
    uart.write(user_input.encode())
    while True:
        if uart.any():
            data = uart.read()
            print(data.decode())
                
else:
    uart.write('N'.encode())
    user_input = input("To view weight in kg, press K press N to skip  ")
    uart.write(user_input.encode())
    while True:
        if uart.any():
            data = uart.read()
            print(data.decode())
                
