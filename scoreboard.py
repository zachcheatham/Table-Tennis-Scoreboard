import cairo
import gi
import threading

gi.require_version("Gtk", "3.0")

from gi.repository import Gtk, Gdk
from RPi import GPIO

MAX_SCORE = 11
SERVES = 2
LEFT_BUTTON = 7
RIGHT_BUTTON = 11

# Helper function draw draw text centered at a point
def draw_text_at(cr, text, x, y, centered=False, vertical_spacing=5):
    # Single-lined text
    if isinstance(text, str):
        # Get text bounds
        text_extents = cr.text_extents(text)

        if centered:
            # (In order to center vertically, we ADD half tr.he text height because
            # cairo renders from the bottom left of the text)
            cr.move_to(x - text_extents[2] / 2, y + text_extents[3] / 2)
        else:
            cr.move_to(x, y + text_extents[3])

        cr.show_text(text)
        return (text_extents[2], text_extents[3])
    # Multi-lined text
    else:
        # Get height of single line of text
        line_height = cr.text_extents(text[0])[3]

        total_width = 0 # We'll figure this as we measure lines
        total_height = (line_height * len(text)) + (vertical_spacing * (len(text) - 1))

        # Calculate the y for centering
        starting_y = 0
        if centered:
            starting_y = y - total_height / 2
        else:
            starting_y = y

        # Render each line of text
        for i in range(0, len(text)):
            # Get the bounds of the line
            line_extents = cr.text_extents(text[i])

            # Adjust what our total_width is
            if line_extents[2] > total_width:
                total_width = line_extents[2]

            # Calculate the x for centering
            render_x = 0
            if centered:
                render_x = x - (line_extents[2] / 2)
            else:
                render_x = x

            # Move down
            render_y = starting_y + line_height * i + vertical_spacing * i + line_height

            # RENDER
            cr.move_to(render_x, render_y)
            cr.show_text(text[i])

        return (total_width, total_height)


class Scoreboard(Gtk.ApplicationWindow):
    def __init__(self):
        super(Scoreboard, self).__init__(title="Table Tennis Scoreboard")

        self.running = True
        self.left_score = 0
        self.right_score = 0
        self.left_serving = False

        self.game_started = False
        self.game_over = False
        self.left_winner = False

        #self.set_wmclass("Table Tennis Scoreboard", "Table Tennis Scoreboard")
        self.fullscreen_on_monitor(Gdk.Screen.get_default(), 1)
        self.connect("delete-event", Gtk.main_quit)

        drawing_area = Gtk.DrawingArea()
        drawing_area.connect("draw", self.on_draw)
        self.add(drawing_area)

        # Setup buttons
        GPIO.setmode(GPIO.BOARD)
        GPIO.setup(LEFT_BUTTON, GPIO.IN, pull_up_down=GPIO.PUD_UP)
        GPIO.setup(RIGHT_BUTTON, GPIO.IN, pull_up_down=GPIO.PUD_UP)

        self.left_button_thread = threading.Thread(target=self.listen_for_l_button)
        self.left_button_thread.start()
        self.right_button_thread = threading.Thread(target=self.listen_for_r_button)
        self.right_button_thread.start()

    def listen_for_l_button(self):
        while self.running:
            channel = GPIO.wait_for_edge(LEFT_BUTTON, GPIO.RISING)
            if channel == LEFT_BUTTON:
                print ("LEFT_BUTTON")

    def listen_for_r_button(self):
        while self.running:
            channel = GPIO.wait_for_edge(RIGHT_BUTTON, GPIO.RISING)
            if channel == RIGHT_BUTTON:
                print ("RIGHT_BUTTON")

    def on_draw(self, widget, cr):
        width, height = window.get_size()
        x_half = width / 2
        y_half = height / 2

        # Background
        cr.set_source_rgb(0, 0, 0)
        cr.rectangle(0, 0, width, height)
        cr.fill()

        cr.select_font_face("Ozone", cairo.FONT_SLANT_NORMAL, cairo.FONT_WEIGHT_NORMAL)

        # Draw scoreboard
        if self.game_started:
            if not self.game_over and (self.left_score >= MAX_SCORE or self.right_score >= MAX_SCORE):
                cr.set_source_rgb(1, 0, 0)
            else:
                cr.set_source_rgb(1, 0.8, 0)
            # Center Line
            cr.set_line_width(2)
            cr.line_to(x_half, 0)
            cr.line_to(x_half, height)
            cr.stroke()

            # Set score font
            cr.set_font_size(height / 2)

            # Render left score
            if self.game_over and self.left_winner:
                cr.set_source_rgb(0, 1, 0)
            elif not self.game_over and (self.left_score >= MAX_SCORE or self.right_score >= MAX_SCORE):
                cr.set_source_rgb(1, 0, 0)
            else:
                cr.set_source_rgb(1, 0.8, 0)
            score_width, score_height = draw_text_at(cr, '%02d' % self.left_score, x_half / 2, y_half, True)

            # Render right score
            if self.game_over and not self.left_winner:
                cr.set_source_rgb(0, 1, 0)
            elif not self.game_over and (self.left_score >= MAX_SCORE or self.right_score >= MAX_SCORE):
                cr.set_source_rgb(1, 0, 0)
            else:
                cr.set_source_rgb(1, 0.8, 0)
            draw_text_at(cr, '%02d' % self.right_score, x_half + x_half / 2, y_half, True)

            score_bottom = y_half + (score_height / 2)
            if self.game_over:
                # Render serving
                cr.set_font_size(height / 10)
                serving_position = score_bottom + ((height - score_bottom) / 2)
                cr.set_source_rgb(0, 1, 0)
                if self.left_winner:
                    draw_text_at(cr, "WINNER", (x_half / 2), serving_position, True)
                else:
                    draw_text_at(cr, "WINNER", x_half + (x_half / 2), serving_position, True)
            else:
                # Render serving
                cr.set_font_size(height / 10)
                serving_position = score_bottom + ((height - score_bottom) / 2)
                if self.left_serving:
                    draw_text_at(cr, "SERVING", (x_half / 2), serving_position, True)
                else:
                    draw_text_at(cr, "SERVING", x_half + (x_half / 2), serving_position, True)

        # Draw start instructions
        else:
            cr.set_source_rgb(1, 0.8, 0)
            cr.set_font_size(width / 8)

            draw_text_at(cr, ("First server", "press button", "to start"), x_half, y_half, True, width / 14) # width 8 / 2 really means width / 14

window = Scoreboard()
window.show_all()

Gtk.main()
