import math
import threading
import time

RASPI = True
try:
    from RPi import GPIO
except ImportError:
    RASPI = False

from tkinter import Tk, Canvas, Frame, font, BOTH, CENTER, ALL

MAX_SCORE = 11
SERVES = 2

LEFT_BUTTON = 11
RIGHT_BUTTON = 12

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
        self.game_start_time = 0
        self.game_over = False

        self.parent.title("Table Tennis Scoreboard")
        self.pack(fill=BOTH, expand=2)

        self.canvas = Canvas(self, highlightthickness=0)
        self.canvas.configure(background="black")
        self.canvas.pack(fill=BOTH, expand=2)
        self.setup_canvas()

        if RASPI:
            GPIO.setmode(GPIO.BOARD)
            GPIO.setup(LEFT_BUTTON, GPIO.IN, pull_up_down=GPIO.PUD_UP)
            GPIO.setup(RIGHT_BUTTON, GPIO.IN, pull_up_down=GPIO.PUD_UP)

            GPIO.add_event_detect(LEFT_BUTTON, GPIO.FALLING, callback=self.button_event, bouncetime=1000)
            GPIO.add_event_detect(RIGHT_BUTTON, GPIO.FALLING, callback=self.button_event, bouncetime=1000)
        else:
            self.parent.bind("<Button-1>", self.left_click)
            self.parent.bind("<Button-3>", self.right_click)

        self.parent.bind("<Escape>", self.clean_up)

    def left_click(self, event):
        self.on_button_press(True, False)

    def right_click(self, event):
        self.on_button_press(False, False)

    def button_event(self, channel):
            self.on_button_press(channel == LEFT_BUTTON, False)

    def on_button_press(self, left_side, is_long):
        if not self.game_started:
            self.new_game(left_side)
        elif self.game_over:
            if is_long:
                if left_side and self.left_score > self.right_score:
                    self.set_left_score(self.left_score - 1)
                elif not left_side and self.left_score < self.right_score:
                    self.set_right_score(self.right_score - 1)
            else:
                self.new_game(left_side)
        else:
            if left_side:
                if is_long:
                    if self.left_score > 0:
                        self.set_left_score(self.left_score - 1)
                else:
                    self.set_left_score(self.left_score + 1)
            else:
                if is_long:
                    if self.right_score > 0:
                        self.set_right_score(self.right_score - 1)
                else:
                    self.set_right_score(self.right_score + 1)

            if not self.game_over:
                total_points = self.right_score + self.left_score
                first_serving = math.floor(total_points / 2) % 2 == 0
                self.set_left_serving((self.left_first_serving and first_serving) or (not self.left_first_serving and first_serving))


    def setup_canvas(self):
        self.canvas.delete(ALL)

        width = self.winfo_width()
        height = self.winfo_height()
        x_half = width / 2
        y_half = height / 2

        self.left_indicator_pos = x_half / 2
        self.right_indicator_pos = x_half + x_half / 2

        biglabel_font = font.Font(family="Ozone", size=round(height/6))
        score_font = font.Font(family="Ozone", size=round(height/2.5))
        label_font = font.Font(family="Ozone", size=round(height/10))
        timer_font = font.Font(family="Ozone", size=round(height/7))

        self.canvas.create_text(x_half, y_half, text="First server\npress button\nto start", fill=DEFAULT_COLOR, font=biglabel_font, justify=CENTER, tags="instructions")
        self.canvas.create_text(x_half / 2, y_half, text='%02d' % self.left_score, fill=DEFAULT_COLOR, font=score_font, tags=("left_score", "overtime_color", "game"))
        self.canvas.create_text(x_half + x_half / 2, y_half, text='%02d' % self.right_score, fill=DEFAULT_COLOR, font=score_font, tags=("right_score", "overtime_color", "game"))

        timer_y = timer_font.cget("size") / 2 + height / 60
        self.canvas.create_text(x_half, timer_y, text="00:00", fill=DEFAULT_COLOR, font=timer_font, tags=("timer", "game"))
        line_pos = timer_y + timer_font.cget("size") / 2 + height / 30
        self.canvas.create_line(x_half, line_pos, x_half, height, width=2, fill=DEFAULT_COLOR, tags=("overtime_color", "game"))
        self.canvas.create_line(0, line_pos, width, line_pos, width=2, fill=DEFAULT_COLOR, tags=("overtime_color", "game"))

        score_bottom = y_half + (score_font.cget("size") / 2)
        self.label_y = score_bottom + ((height - score_bottom) / 2)

        self.canvas.create_text(x_half / 2, self.label_y, text="LABEL", fill=DEFAULT_COLOR, font=label_font, tags=("indicator_label", "overtime_color", "game"))

        # Reapply logic
        self.set_game_started(self.game_started)
        self.set_left_serving(self.left_serving)
        if self.left_score >= self.right_score:
            self.set_right_score(self.right_score)
            self.set_left_score(self.left_score)
        else:
            self.set_right_score(self.right_score)
            self.set_left_score(self.left_score)

    def new_game(self, left_serving_first):
        self.set_game_started(True)
        self.set_game_over(False)
        self.set_left_score(0)
        self.set_right_score(0)
        self.game_start_time = time.time()
        self.set_left_serving(left_serving_first)
        self.left_serving_first = left_serving_first
        self.canvas.itemconfig("overtime_color", fill=DEFAULT_COLOR)
        threading.Thread(target=self.update_timer).start()

    def set_game_started(self, started):
        self.game_started = started
        if started:
            self.canvas.itemconfig("instructions", state="hidden")
            self.canvas.itemconfig("game", state="normal")
        else:
            self.canvas.itemconfig("instructions", state="normal")
            self.canvas.itemconfig("game", state="hidden")

    def set_left_serving(self, left_serving):
        self.left_serving = left_serving
        if left_serving:
            self.canvas.coords("indicator_label", self.left_indicator_pos, self.label_y)
        else:
            self.canvas.coords("indicator_label", self.right_indicator_pos, self.label_y)

        self.canvas.itemconfig("indicator_label", text="SERVING")

    def set_left_score(self, score):
        self.left_score = score
        self.canvas.itemconfig("left_score", text="%02d" % score)

        if score >= MAX_SCORE:
            if score - self.right_score > 1:
                self.set_game_over(True)
            else:
                self.canvas.itemconfig("overtime_color", fill=OVERTIME_COLOR)
                if self.game_over:
                    self.set_game_over(False)
        else:
            self.canvas.itemconfig("overtime_color", fill=DEFAULT_COLOR)
            if self.game_over:
                self.set_game_over(False)

    def set_right_score(self, score):
        self.right_score = score
        self.canvas.itemconfig("right_score", text="%02d" % score)

        if score >= MAX_SCORE:
            if score - self.left_score > 1:
                self.set_game_over(True)
            else:
                self.canvas.itemconfig("overtime_color", fill=OVERTIME_COLOR)
                if self.game_over:
                    self.set_game_over(False)
        else:
            self.canvas.itemconfig("overtime_color", fill=DEFAULT_COLOR)
            if self.game_over:
                self.set_game_over(False)

    def set_game_over(self, game_over):
        self.game_over = game_over
        if game_over:
            self.canvas.itemconfig("indicator_label", text="WINNER", fill=WINNING_COLOR)
            if self.left_score > self.right_score:
                self.canvas.itemconfig("left_score", fill=WINNING_COLOR)
                self.canvas.coords("indicator_label", self.left_indicator_pos, self.label_y)
            else:
                self.canvas.itemconfig("right_score", fill=WINNING_COLOR)
                self.canvas.coords("indicator_label", self.right_indicator_pos, self.label_y)
        else:
            # Reset label position and coloring
            self.set_left_serving(self.left_serving)

    def update_timer(self):
        while not self.game_over and self.game_started:
            seconds = time.time() - self.game_start_time
            minutes = math.floor(seconds / 60)
            seconds = seconds % 60
            self.canvas.itemconfig("timer", text="%02d:%02d" % (minutes, seconds))

    def on_resize(self, event):
        self.setup_canvas()

    def clean_up(self, event=None):
        self.run = False
        if RASPI:
            GPIO.cleanup()
        root.destroy()

root = Tk()
root.attributes("-fullscreen", True)
root.config(bg="black")

window = Scoreboard(root)
root.protocol("WM_DELETE_WINDOW", window.clean_up)

root.mainloop()
