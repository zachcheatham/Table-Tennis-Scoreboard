import math
import threading
import time

from RPi import GPIO
from tkinter import Tk, Canvas, Frame, font, BOTH, CENTER

MAX_SCORE = 11
SERVES = 2

LEFT_BUTTON = 11
RIGHT_BUTTON = 13

DEFAULT_COLOR = "#FFC900"
OVERTIME_COLOR = "#FF0000"
WINNING_COLOR = "#00FF00"

class Scoreboard(Frame):
    def __init__(self, parent):
        super(Scoreboard, self).__init__(parent)

        self.parent = parent
        self.run = True
        self.config(bg="black", cursor='none')
        self.bind("<Configure>", self.on_resize)

        self.running = True
        self.left_score = 0
        self.right_score = 0
        self.left_serving = False
        self.left_first_serving = False

        self.game_started = False
        self.game_over = False
        self.left_winner = False

        self.parent.title("Table Tennis Scoreboard")
        self.pack(fill=BOTH, expand=2)

        self.canvas = Canvas(self, highlightthickness=0)
        self.canvas.configure(background="black")
        self.canvas.pack(fill=BOTH, expand=2)

        self.draw_canvas()

        # Setup buttons
        GPIO.setmode(GPIO.BOARD)
        GPIO.setup(LEFT_BUTTON, GPIO.IN, pull_up_down=GPIO.PUD_UP)
        GPIO.setup(RIGHT_BUTTON, GPIO.IN, pull_up_down=GPIO.PUD_UP)

        self.left_button_thread = threading.Thread(target=self.listen_to_left_button)
        self.left_button_thread.start()

    def listen_to_left_button(self):
        while self.run:
            press_time = 0
            while self.run:
                c = GPIO.wait_for_edge(LEFT_BUTTON, GPIO.RISING, timeout=500)
                if not (c is None):
                    print ("L BUTTON DOWN")
                    press_time = int(time.time())
                    break

            waiting = True
            while self.run:
                c = GPIO.wait_for_edge(LEFT_BUTTON, GPIO.FALLING, timeout=500)
                if not (c is None):
                    print ("L BUTTON UP")
                    press_length = int(time.time()) - press_time
                    self.on_button_press(True, press_length >= 3)
                    break

    def listen_to_right_button(self):
        while self.run:
            press_time = 0
            while self.run:
                c = GPIO.wait_for_edge(RIGHT_BUTTON, GPIO.RISING, timeout=500)
                if not (c is None):
                    print ("R BUTTON DOWN")
                    press_time = int(time.time())
                    break

            waiting = True
            while self.run:
                c = GPIO.wait_for_edge(RIGHT_BUTTON, GPIO.FALLING, timeout=500)
                if not (c is None):
                    print ("R BUTTON UP")
                    press_length = int(time.time()) - press_time
                    self.on_button_press(False, press_length >= 3)
                    break

    def on_button_press(self, left_side, is_long):
        if not self.game_started:
            self.game_started = True
            self.left_serving = left_side
            self.left_first_serving = left_side
        elif self.game_over:
            if is_long:
                if left_side and self.left_winner:
                    self.left_score = self.left_score - 1
                    self.game_over = False
                elif not left_side and not self.left_winner:
                    self.right_score = self.right_score - 1
                    self.game_over = False
            else:
                self.game_over = False
                self.left_score = 0
                self.right_score = 0
                self.left_serving = left_side
                self.left_first_serving = left_side
        else:
            if left_side:
                if is_long:
                    self.left_score = self.left_score - 1
                    if self.left_score < 0:
                        self.left_score = 0
                else:
                    self.left_score = self.left_score + 1
                    if self.left_score >= MAX_SCORE:
                        if self.left_score - self.right_score > 1:
                            self.game_over = True
                            self.left_winner = True
            else:
                if is_long:
                    self.right_score = self.right_score - 1
                    if self.right_score < 0:
                        self.right_score = 0
                else:
                    self.right_score = self.right_score + 1
                    if self.right_score >= MAX_SCORE:
                        if self.right_score - self.left_score > 1:
                            self.game_over = True
                            self.left_winner = False

            total_points = self.right_score + self.left_score
            if self.left_first_serving:
                self.left_serving = (math.floor(total_points / 2) % 2 == 0)
            else:
                self.left_serving = (math.floor(total_points / 2) % 2 != 0)

        self.draw_canvas()

    def draw_canvas(self):
        self.canvas.delete("all")

        width = self.winfo_width()
        height = self.winfo_height()
        x_half = width / 2
        y_half = height / 2

        if self.game_started:
            main_color = DEFAULT_COLOR
            if not self.game_over and (self.left_score >= MAX_SCORE or self.right_score >= MAX_SCORE):
                main_color = OVERTIME_COLOR

            self.canvas.create_line(x_half, 0, x_half, height, width=2, fill=main_color)

            score_font = font.Font(family="Ozone", size=round(height/2.5))
            label_font = font.Font(family="Ozone", size=round(height/10))

            # Render left score
            color = main_color
            if self.game_over and self.left_winner:
                color = WINNING_COLOR
            self.canvas.create_text(x_half / 2, y_half, text='%02d' % self.left_score, fill=color, font=score_font)

            # Render right score
            color = main_color
            if self.game_over and not self.left_winner:
                color = WINNING_COLOR
            self.canvas.create_text(x_half + x_half / 2, y_half, text='%02d' % self.right_score, fill=color, font=score_font)

            score_bottom = y_half + (round(height/2.5) / 2)
            if self.game_over:
                # Render winner
                serving_position = score_bottom + ((height - score_bottom) / 2)
                if self.left_winner:
                    self.canvas.create_text(x_half / 2, serving_position, text="WINNER", fill=WINNING_COLOR, font=label_font)
                else:
                    self.canvas.create_text(x_half + x_half / 2, serving_position, text="WINNER", fill=WINNING_COLOR, font=label_font)
            else:
                # Render serving label
                serving_position = score_bottom + ((height - score_bottom) / 2)
                if self.left_serving:
                    self.canvas.create_text(x_half / 2, serving_position, text="SERVING", fill=color, font=label_font)
                else:
                    self.canvas.create_text(x_half + x_half / 2, serving_position, text="SERVING", fill=color, font=label_font)
        else:
            label_font = font.Font(family="Ozone", size=round(height/6))
            self.canvas.create_text(x_half, y_half, text="First server\npress button\nto start", fill=DEFAULT_COLOR, font=label_font, justify=CENTER)

    def on_resize(self, event):
        self.draw_canvas()

    def clean_up(self):
        self.run = False
        GPIO.cleanup()
        root.destroy()

root = Tk()
root.attributes("-fullscreen", True)
root.config(bg="black")

window = Scoreboard(root)

root.protocol("WM_DELETE_WINDOW", window.clean_up)
root.mainloop()
