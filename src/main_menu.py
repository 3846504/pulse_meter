import os
import RPi.GPIO as GPIO
import time
import sys
import atexit

disp = 0
flag = 0

SHUTDOWN_PIN = 3
SELECT_PIN = 4
SAVE_PIN = 17
ROTE_L_PIN = 27
ROTE_R_PIN = 22

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
    rote_r_pin = ROTE_R_PIN
    rote_l_pin = ROTE_L_PIN
    if(GPIO.input(rote_r_pin) == 0 and GPIO.input(rote_l_pin) == 1):
        if(disp < 2):
            disp += 1
    elif(GPIO.input(rote_r_pin) == 0 and GPIO.input(rote_l_pin) == 0):
        if(disp > 0):
            disp -= 1

    os.system(command[disp])

def shutdown(gpio_pin):
    button_state = 1
    for _ in range(15):
        time.sleep(0.2)
        button_state = GPIO.input(gpio_pin)
        if(button_state == 1):
            break
    if(button_state == 0):
        sys.stdout.write("-1")
        sys.exit()

def update(gpio_pin):
    button1 = 1
    button2 = 1
    button3 = 1
    for _ in range(15):
        time.sleep(0.2)
        button1 = GPIO.input(SHUTDOWN_PIN)
        button2 = GPIO.input(SELECT_PIN)
        button3 = GPIO.input(SAVE_PIN)
        if(button1 == 1 or button2 == 1 or button3 == 1):
            break
    if(button1 == 0 and button2 == 0 and button3 == 0):
        os.system("sudo git -C /opt/sakulab_ex_module pull")
        os.system("sudo reboot")

if(__name__ == "__main__"):
    select_pin = SELECT_PIN
    shutdown_pin = SHUTDOWN_PIN

    rotA_pin = ROTE_R_PIN
    rotB_pin = ROTE_L_PIN

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