#!/usr/bin/env python
#-*- coding: utf-8 -*-

import sys
import socket

import msgpack

# PyGI for GTK+ 3
from gi.repository import Gtk, GLib, Gdk, GdkPixbuf

class Monitor(object):
    width = 640
    height = 480

    def __init__(self, host, port):
        self.window = Gtk.Window()
        self.window.connect("destroy", self.destroy)

        # drawing area
        self.drawing = Gtk.DrawingArea()
        self.drawing.set_size_request(self.width, self.height)
        self.drawing.connect("draw", self.on_draw)
        self.window.add(self.drawing)

        #elf.GdkPixbuf

        # TCP socket
        self.sock = socket.create_connection((host, port), 10)
        self.fsock = self.sock.makefile()

        # msgpack
        self.packer = msgpack.Packer()
        self.unpacker = msgpack.Unpacker()

        self.watchtag = list()
        self.canvas = dict()

        self.window.show_all()

    def on_draw(self, widget, cr):
        for name, data in self.unpacker:
            if name == "DRAW":
                x, y, color = data
                self.canvas[(x, y)] = color
            else:
                print "[Error] unknown method:", name

        for position, color in self.canvas.iteritems():
            x, y = position
            r = ((color >> 11) & 0x1f) / float(0x1f)
            g = ((color >> 5) & 0x3f) / float(0x3f)
            b = (color & 0x1f) / float(0x1f)
            
            cr.set_source_rgb(r, g, b)
            cr.rectangle(x, y, 1, 1)
            cr.fill()

    def method_recv(self, source, condition):
        self.unpacker.feed(self.sock.recv(1024 * 1024))
        self.drawing.queue_draw()
        return True

    def method_hup(self, source, condition):
        print "HUP"
        exit()

    def main(self):
        # watch socket
        recv = GLib.io_add_watch(self.fsock, GLib.IO_IN, self.method_recv)
        hup = GLib.io_add_watch(self.fsock, GLib.IO_HUP, self.method_hup)
        self.watchtag.append(recv)
        self.watchtag.append(hup)

        msg = self.packer.pack(["CONNECT"])
        self.sock.send(msg)

        Gtk.main()

    def destroy(self, window):
        for i in self.watchtag:
            GLib.source_remove(i)

        self.fsock.close()
        self.sock.close()

        Gtk.main_quit()

if __name__ == "__main__":
    host = sys.argv[1]
    port = 1032

    print "Connect to", host, port

    monitor = Monitor(host, port)
    monitor.main()
