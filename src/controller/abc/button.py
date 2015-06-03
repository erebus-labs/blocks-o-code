import Adafruit_BBIO.GPIO as GPIO

class ABCButtonManager(object):
    '''
    Manages controller buttons and calls the button's callbacks
    when neccessary
    '''

    def __init__(self, buttons):
        '''
        :param buttons: A list of ABCButton instances
        '''
        self.buttons = buttons

    def poll(self):
        '''
        Check for button presses. If one, and only one, button is
        pressed, call the button's callback

        :returns: True if a callback was called, otherwise false
        '''
        active_buttons = filter(lambda x: x.check(), buttons)
        if len(active_buttons) != 1:
            return False
        else:
            if active_buttons[0].callback:
                active_buttons[0].callback()
                return True
            else:
                return False

class ABCButton(object):
    '''
    Represents a button on the controller cape
    '''

    def __init__(self, port=None, callback=None, exec_on_up=True):
        '''
        :param port: the BBB port to check
        :param callback: called if the button is presed
        :param exec_on_up: the button is 'pressed' if the pin is HIGH
            and exec_on_up is true, or if the pin is low and exec_on_up
            is false. The button is not pressed in any other case
        '''
        self.port = port
        if self.port:
            GPIO.setup(self.port, GPIO.IN)
        if callback:
            self.callback = callback
        self.exec_on_up = exec_on_up

    def check(self):
        '''
        Check if the button is pressed

        :returns: True if the button is pressed
        '''
        if self.port:
            if GPIO.input(self.port) and self.exec_on_up:
                return True
            elif (not GPIO.input(self.port)) and (not self.exec_on_up):
                return True
            else:
                return False
        else:
            return False

    def callback(self):
        '''
        Called when the button is pressed
        '''
        pass
