import urequests
import ujson
from machine import UART
import utime

"""
This Micropython code is intended to be flashed onto the cellular module.

It reads an incoming CAN message from the cellular module's UART, packages it
into a JSON, and makes an HTTP POST request to a URL of your choice.
"""

server_name = ""
read_delay = 500       # time delay between consecutive UART reads (in ms)

# Initialising UART
# TODO: test this error handling more
try:
    uart = UART(1, 9600)
    uart.init(9600, bits=8, parity=None, stop=1)
except Exception as exc:
    raise RuntimeError("UART cannot be initialised") from exc

while True:
    # TODO: surround with try-except statement
    # returns None if no data is sent
    # reads len of one CAN message into 'message' buffer
    message = uart.read(26)

    if message is None:
        continue
    if len(message) != 26:
        continue

    # converts byte object into hex string
    can_hex_string = "0x" + message.decode()

    data_array = []
    for i in range(0, 16, 2):
        # populates data_array
        data_array.append(int(can_hex_string[10 + i: 12 + i], 16))

    # stores the CAN message which will be converted to a JSON string
    can_msg = dict()

    # separates the first four hex digits as timestamp
    can_msg["timestamp"] = int(can_hex_string[2:6], 16)
    # continues separating the hex string into JSON key-value pairs
    can_msg["id"] = int(can_hex_string[6:10], 16)
    can_msg["data"] = data_array
    can_msg["len"] = int(can_hex_string[26:28], 16)

    # converts CAN message into JSON string
    can_msg_json = ujson.dumps(can_msg)

    # TODO: add error-handling for this line
    response = urequests.post(server_name, json=can_msg_json)
    print("CAN message JSON:", can_msg_json)

    utime.sleep_ms(read_delay)
