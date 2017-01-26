import threading

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
        self.config(bg="black", cursor='none')
        self.bind("<Configure>", self.on_resize)

        self.running = True
        self.left_score = 0
        self.right_score = 0
        self.left_serving = False

        self.game_started = False
        self.game_over = False
        self.left_winner = False

        self.parent.title("Table Tennis Scoreboard")
        self.pack(fill=BOTH, expand=2)

        self.canvas = Canvas(self, highlightthickness=0)
        self.canvas.configure(background="black")
        self.canvas.pack(fill=BOTH, expand=2)

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

root = Tk()
root.attributes("-fullscreen", True)
root.config(bg="black")
window = Scoreboard(root)
root.mainloop()
