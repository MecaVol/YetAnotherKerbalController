""" Yet Another Kerbal Controller v1.0

This script is to be launched after Kerbal Space Program is started and a vessel is under control, while
kRPC server is started on localhost 127.0.0.1.
"""

import serial
import time
import krpc
from enum import Enum

print('Main script starts...')

# Global constants
ARDUINO_PORT = 'COM4'


class SPEED_MODE(Enum):
    ORBIT = 1
    TARGET = 2
    SURFACE = 3


class SAS_MODE(Enum):
    PROGRADE = 1
    RETROGRADE = 2
    NORMAL = 3
    ANTI_NORMAL = 4
    RADIAL = 5
    ANTI_RADIAL = 6
    STABILITY_ASSIST = 7
    MANEUVER = 8
    TO_TARGET = 9
    ANTI_TARGET = 10


# Global variables
arduino = None  # Serial connection to the board
krpc_server = None  # kRPC server connection
vessel = None  # kRPC server connection to the Vessel object

sas = False
rcs = False
speed_mode = SPEED_MODE.SURFACE
sas_mode = SAS_MODE.STABILITY_ASSIST

forward = False
backward = False
up = False
down = False
left = False
right = False


def set_sas(x):
    global sas
    sas = x


def set_rcs(x):
    global rcs
    rcs = x


def set_speed_mode(x):
    global speed_mode
    if x == vessel.control.speed_mode.orbit:
        speed_mode = SPEED_MODE.ORBIT
    if x == vessel.control.speed_mode.target:
        speed_mode = SPEED_MODE.TARGET
    if x == vessel.control.speed_mode.surface:
        speed_mode = SPEED_MODE.SURFACE


def set_sas_mode(x):
    global sas_mode
    if x == vessel.control.sas_mode.prograde:
        sas_mode = SAS_MODE.PROGRADE
    if x == vessel.control.sas_mode.retrograde:
        sas_mode = SAS_MODE.RETROGRADE
    if x == vessel.control.sas_mode.normal:
        sas_mode = SAS_MODE.NORMAL
    if x == vessel.control.sas_mode.anti_normal:
        sas_mode = SAS_MODE.ANTI_NORMAL
    if x == vessel.control.sas_mode.radial:
        sas_mode = SAS_MODE.RADIAL
    if x == vessel.control.sas_mode.anti_radial:
        sas_mode = SAS_MODE.ANTI_RADIAL
    if x == vessel.control.sas_mode.stability_assist:
        sas_mode = SAS_MODE.STABILITY_ASSIST
    if x == vessel.control.sas_mode.maneuver:
        sas_mode = SAS_MODE.MANEUVER
    if x == vessel.control.sas_mode.target:
        sas_mode = SAS_MODE.TO_TARGET
    if x == vessel.control.sas_mode.anti_target:
        sas_mode = SAS_MODE.ANTI_TARGET


def reset_translations():

    global forward
    global backward
    global up
    global down
    global left
    global right

    forward = False
    backward = False
    up = False
    down = False
    left = False
    right = False


def process_controller_data(raw_serial_data):
    """ This function processes data from the controller

    The controller sends data as bytes, ASCII encoded, to share its states, according to the following:
    'A' : SAS on    'B' : SAS off
    'C' : RCS on    'D' : RCS off

    :param raw_serial_data: bytes type
    """
    global sas
    global rcs
    global speed_mode
    global sas_mode

    global forward
    global backward
    global up
    global down
    global left
    global right

    txt = raw_serial_data.decode('utf-8')
    if 'A' in txt:
        sas = True
    if 'B' in txt:
        sas = False
    if 'C' in txt:
        rcs = True
    if 'D' in txt:
        rcs = False
    if 'E' in txt:
        speed_mode = SPEED_MODE.ORBIT
    if 'F' in txt:
        speed_mode = SPEED_MODE.TARGET
    if 'G' in txt:
        speed_mode = SPEED_MODE.SURFACE
    if 'H' in txt:
        sas_mode = SAS_MODE.PROGRADE
    if 'I' in txt:
        sas_mode = SAS_MODE.RETROGRADE
    if 'J' in txt:
        sas_mode = SAS_MODE.NORMAL
    if 'K' in txt:
        sas_mode = SAS_MODE.ANTI_NORMAL
    if 'L' in txt:
        sas_mode = SAS_MODE.RADIAL
    if 'M' in txt:
        sas_mode = SAS_MODE.ANTI_RADIAL
    if 'N' in txt:
        sas_mode = SAS_MODE.STABILITY_ASSIST
    if 'O' in txt:
        sas_mode = SAS_MODE.MANEUVER
    if 'P' in txt:
        sas_mode = SAS_MODE.TO_TARGET
    if 'Q' in txt:
        sas_mode = SAS_MODE.ANTI_TARGET
    if 'R' in txt:
        forward = True
    if 'S' in txt:
        backward = True
    if 'T' in txt:
        up = True
    if 'U' in txt:
        down = True
    if 'V' in txt:
        left = True
    if 'W' in txt:
        right = True


def send_state_to_controller():
    """ Encode state and send it to the controller

    The corresponding ASCII table can be found in process_serial_data docstring
    """
    state_string = ''
    if sas:
        state_string += 'A'
    else:
        state_string += 'B'
    if rcs:
        state_string += 'C'
    else:
        state_string += 'D'

    if speed_mode == SPEED_MODE.ORBIT:
        state_string += 'E'
    if speed_mode == SPEED_MODE.TARGET:
        state_string += 'F'
    if speed_mode == SPEED_MODE.SURFACE:
        state_string += 'G'

    if sas_mode == SAS_MODE.PROGRADE:
        state_string += 'H'
    if sas_mode == SAS_MODE.RETROGRADE:
        state_string += 'I'
    if sas_mode == SAS_MODE.NORMAL:
        state_string += 'J'
    if sas_mode == SAS_MODE.ANTI_NORMAL:
        state_string += 'K'
    if sas_mode == SAS_MODE.RADIAL:
        state_string += 'L'
    if sas_mode == SAS_MODE.ANTI_RADIAL:
        state_string += 'M'
    if sas_mode == SAS_MODE.STABILITY_ASSIST:
        state_string += 'N'
    if sas_mode == SAS_MODE.MANEUVER:
        state_string += 'O'
    if sas_mode == SAS_MODE.TO_TARGET:
        state_string += 'P'
    if sas_mode == SAS_MODE.ANTI_TARGET:
        state_string += 'Q'

    arduino.write(state_string.encode('utf-8'))


def send_state_to_game():
    if sas:
        vessel.control.sas = True
    else:
        vessel.control.sas = False
    if rcs:
        vessel.control.rcs = True
    else:
        vessel.control.rcs = False

    if speed_mode == SPEED_MODE.ORBIT:
        vessel.control.speed_mode = vessel.control.speed_mode.orbit
    elif speed_mode == SPEED_MODE.TARGET:
        vessel.control.speed_mode = vessel.control.speed_mode.target
    elif speed_mode == SPEED_MODE.SURFACE:
        vessel.control.speed_mode = vessel.control.speed_mode.surface

    if sas_mode == SAS_MODE.PROGRADE:
        vessel.control.sas_mode = vessel.control.sas_mode.prograde
    if sas_mode == SAS_MODE.RETROGRADE:
        vessel.control.sas_mode = vessel.control.sas_mode.retrograde
    if sas_mode == SAS_MODE.NORMAL:
        vessel.control.sas_mode = vessel.control.sas_mode.normal
    if sas_mode == SAS_MODE.ANTI_NORMAL:
        vessel.control.sas_mode = vessel.control.sas_mode.anti_normal
    if sas_mode == SAS_MODE.RADIAL:
        vessel.control.sas_mode = vessel.control.sas_mode.radial
    if sas_mode == SAS_MODE.ANTI_RADIAL:
        vessel.control.sas_mode = vessel.control.sas_mode.anti_radial
    if sas_mode == SAS_MODE.STABILITY_ASSIST:
        vessel.control.sas_mode = vessel.control.sas_mode.stability_assist
    if sas_mode == SAS_MODE.MANEUVER:
        vessel.control.sas_mode = vessel.control.sas_mode.maneuver
    if sas_mode == SAS_MODE.TO_TARGET:
        vessel.control.sas_mode = vessel.control.sas_mode.target
    if sas_mode == SAS_MODE.ANTI_TARGET:
        vessel.control.sas_mode = vessel.control.sas_mode.anti_target

    if forward:
        vessel.control.forward = 1
    elif backward:
        vessel.control.forward = -1
    else:
        vessel.control.forward = 0
    if up:
        vessel.control.up = 1
    elif down:
        vessel.control.up = -1
    else:
        vessel.control.up = 0
    if left:
        vessel.control.right = -1
    elif right:
        vessel.control.right = 1
    else:
        vessel.control.right = 0



# Serial connection to Arduino
try:
    arduino = serial.Serial(ARDUINO_PORT, 9600, timeout=0.05, writeTimeout=0.05)
    print('Arduino connected')
except serial.SerialException:
    print('Arduino board cannot be found on port: ' + ARDUINO_PORT)
    exit(0)

# kRPC server connection
try:
    krpc_server = krpc.connect(name='Yet Another Kerbal Controller')
    vessel = krpc_server.space_center.active_vessel
    print("Connected to vessel: " + vessel.name)
except ConnectionError:
    print('Connection Error to Kerbal Space Program.')
    exit(0)

# Setting Callbacks
SAS_stream = krpc_server.add_stream(getattr, vessel.control, 'sas')
RCS_stream = krpc_server.add_stream(getattr, vessel.control, 'rcs')
speed_control_stream = krpc_server.add_stream(getattr, vessel.control, 'speed_mode')
sas_control_stream = krpc_server.add_stream(getattr, vessel.control, 'sas_mode')

SAS_stream.add_callback(set_sas)
RCS_stream.add_callback(set_rcs)
speed_control_stream.add_callback(set_speed_mode)
sas_control_stream.add_callback(set_sas_mode)

SAS_stream.start()
RCS_stream.start()
speed_control_stream.start()
sas_control_stream.start()

while True:
    reset_translations()
    while arduino.in_waiting > 0:
        process_controller_data(arduino.read())
    send_state_to_game()
    # print('SAS:', str(sas), 'RCS:', str(rcs), 'speed mode:', str(speed_mode), 'sas mode:', str(sas_mode))
    send_state_to_controller()
    time.sleep(0.02)


print('Main script stops!')
