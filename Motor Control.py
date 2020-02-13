# Import the GPIO library
import RPI.GPIO as IO

# Define motor pin on Raspberry Pi
motor_pin = 12

# Set the mode as IO
GPIO.setmode(IO.BOARD)
GPIO.setup(motor_pin, IO.OUT)

# Set PWM frequency
pwm = IO.PWM(motor_pin, 100)


# Define the function that reverses the PWM value for a human readable format.
def true_pwm (num):
    true_pwm = abs(num - 100.0)
    return true_pwm
    
while True:
    try:
        value = float(input("Motor percentage float: "))
    except ValueError:
        print("Error! Not a usable value.")
        # better try again... Return to the start of the loop
        continue
    if(value == 'exit'):
        #age was successfully parsed!
        #we're ready to exit the loop.
        break
        GPIO.cleanup()
    else:
        pwm.start(true_pwm(value))
        print("Motor speed set to {}%".format(true_pwm(value))

