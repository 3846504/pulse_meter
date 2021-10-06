import os
import RPi.GPIO as GPIO
import time
import sys
import atexit

disp = 0
flag = 0

command = ['sudo fbi -T 1 -d /dev/fb0 -noverbose /opt/sakulab_ex_module/img/main_menu_select_1.png', 
            'sudo fbi -T 1 -d /dev/fb0 -noverbose /opt/sakulab_ex_module/img/main_menu_select_2.png',
            'sudo fbi -T 1 -d /dev/fb0 -noverbose /opt/sakulab_ex_module/img/main_menu_select_3.png']
    
os.system(command[0])

def select_mode(gpio_pin):
    global disp
    sys.stdout.write(str(disp))
    sys.exit()

def check_rote(gpio_pin):
    global disp
    pinA = 20
    pinB = 21
    if(GPIO.input(pinA) == 0 and GPIO.input(pinB) == 1):
        if(disp < 2):
            disp += 1
    elif(GPIO.input(pinA) == 0 and GPIO.input(pinB) == 0):
        if(disp > 0):
            disp -= 1

    os.system(command[disp])

def shutdown(gpio_pin):
    button = 1
    for _ in range(15):
        time.sleep(0.2)
        button = GPIO.input(gpio_pin)
        if(button == 1):
            break
    if(button == 0):
        sys.stdout.write("-1")
        sys.exit()

if(__name__ == "__main__"):
    select_pin = 12
    shutdown_pin = 3

    rotA_pin = 20
    rotB_pin = 21

    GPIO.setmode(GPIO.BCM)
    GPIO.setup(select_pin, GPIO.IN, pull_up_down=GPIO.PUD_UP)
    GPIO.setup(shutdown_pin, GPIO.IN, pull_up_down=GPIO.PUD_UP)
    GPIO.setup(rotA_pin, GPIO.IN, pull_up_down=GPIO.PUD_UP)
    GPIO.setup(rotB_pin, GPIO.IN, pull_up_down=GPIO.PUD_UP)
    
    GPIO.add_event_detect(select_pin, GPIO.FALLING, callback=select_mode, bouncetime=200)
    GPIO.add_event_detect(shutdown_pin, GPIO.FALLING, callback=shutdown, bouncetime=200)
    GPIO.add_event_detect(rotA_pin, GPIO.FALLING, callback=check_rote, bouncetime=200)

    while(True):
        time.sleep(1)