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
    def __init__(self, title, rows=10, cols=10):
        self.view = ui.View(frame=(0, 0, 300, 300))
        self.view.name = title
        self.view.background_color = 'white'
        self.rows = rows
        self.cols = cols
        self.w = (self.view.frame[2]/cols)
        self.h = (self.view.frame[3]/rows)
        self.subviews = dict()
        self.actions = dict()

    def create_view(self, name, which, xywh=[0,0,1,1], flex=None, bgColor='FFFFFF', border_width=0):
        self.subviews[name] = which
        self.subviews[name].name = name
        if flex:
            self.subviews[name].flex = flex
        else:
            self.subviews[name].flex = 'WRHLTB'
        self.subviews[name].background_color=bgColor
        self.subviews[name].frame = [xywh[0]*self.w,
                                                                 xywh[1]*self.h,
                                                                 xywh[2]*self.w,
                                                                 xywh[3]*self.h]
        self.subviews[name].border_width = border_width

    def add_view(self, view):
        self.view.add_subview(self.subviews[view])

    def set_size(self, name, w, h):
        xywh = list(self.subviews[name].frame)
        xywh[2] = self.w*w
        xywh[3] = self.h*h
        self.subviews[name].frame = xywh

    def set_coords(self, name, x, y):
        xywh = list(self.subviews[name].frame)
        xywh[0] = self.w*x
        xywh[1] = self.h*y
        self.subviews[name].frame = xywh
    
    def get_coords(self, name):
        xywh = list(self.subviews[name].frame)
        return (xywh[0], xywh[1])
    
    def get_size(self, name):
        xywh = list(self.subviews[name].frame)
        return (xywh[3], xywh[2])
  
    def remove_view(self, name):
         self.view.remove_subview(self.subviews[name])

    def set_text(self, name, text):
        self.subviews[name].text = text

    def set_title(self, name, text):
        self.subviews[name].title = text

    def get_text(self, name):
        return self.subviews[name].text

    def get_title(self, name):
        return self.subviews[name].title

    def handle_action(self, sender):
        handler, inst = self.actions[sender.name]
        inst.execute(handler)

    def add_action(self, name, handler, inst):
        self.actions[name] = (handler, inst)
        self.subviews[name].action = self.handle_action
    
    def get_action(self, name, handler, inst):
        return self.actions[name][0]
    
    def present(self):
        self.view.present('sheet')
		