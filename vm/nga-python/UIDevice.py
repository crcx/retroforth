# UIDevice
# Copyright (c) 2020, 2021 Charles Childers
# Copyright (c) 2020, 2021 Arland Childers
#
# This implements a virtual device for creating user
# interfaces on Pythonista (a Python 3 system for
# iOS).
#
# The layout system as designed by Arland uses a grid
# system (defaulting to 10x10). We hope to offer a
# stack based (ala SwiftUI) option in the future.
#
# It's intended that we will port this to other systems
# as well. On iOS, Pyto is another good target (though
# we have less experience with it), and there are a
# number of options on desktop OSes.
#
# http://omz-software.com/pythonista/docs/ios/ui.html

import ui

class UI:
    def __init__(self, rows=10, cols=10):
        self.rows = rows
        self.cols = cols

    def create_view(self, name, id):
        pass

    def add_view(self, name):
        pass

    def set_size(self, name, h, w):
        pass

    def set_coords(self, name, x, y):
        pass

    def remove_view(self, name):
        pass

    def set_text(self, name, text):
        pass

    def set_title(self, name, text):
        pass

    def get_text(self, name):
        pass

    def get_title(self, name):
        pass

    def handle_action(self, sender):
        pass

    def add_action(self, name, handler):
        pass
