import RPi.GPIO as GPIO
import time

heartbeat_pin = 18

def main():
    GPIO.setmode(GPIO.BCM)
    GPIO.setup(heartbeat_pin, GPIO.OUT, initial=GPIO.HIGH)
    heartbeat = GPIO.HIGH
    try:
        while True:
            time.sleep(1)
            GPIO.output(heartbeat_pin, heartbeat)
            heartbeat ^= GPIO.HIGH
    finally:
        GPIO.cleanup()

if __name__ == '__main__':
    main()
