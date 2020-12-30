# MIT License
# 
# Copyright (c) 2019 - 2020 Andreas Merkle <web@blue-andi.de>
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

import tkinter as tk
from tkinter import ttk

Import("env")

class App(tk.Frame):
    def __init__(self, parent=None):
        super(App, self).__init__(parent)

        self._parent    = parent
        self._ipAddress = tk.StringVar()
        self._port      = tk.IntVar()
        self._password  = tk.StringVar()
        self._isAborted = True

        self._ipAddress.set("192.168.2.166")
        self._port.set(3232)
        self._password.set("maytheforcebewithyou")

        labelIpAddress = tk.Label(self, text="IP-Address:", anchor="w")
        inputIpAddress = tk.Entry(self, textvariable=self._ipAddress)

        labelPort = tk.Label(self, text="Port:", anchor="w")
        inputPort = tk.Entry(self, textvariable=self._port)

        labelPassword = tk.Label(self, text="Password:", anchor="w")
        inputPassword = tk.Entry(self, textvariable=self._password)

        labelIpAddress.pack(fill="x", expand=False)
        inputIpAddress.pack(fill="x", expand=True)
        labelPort.pack(fill="x", expand=False)
        inputPort.pack(fill="x", expand=True)
        labelPassword.pack(fill="x", expand=False)
        inputPassword.pack(fill="x", expand=True)

        buttonUpload = tk.Button(self, text="Upload", command=self._upload)
        buttonUpload.pack(fill="x", expand=False)

    def _upload(self):
        self._isAborted = False
        self._parent.quit()

    def isAborted(self):
        return self._isAborted

    def getIPAddress(self):
        return self._ipAddress.get()

    def getPort(self):
        return self._port.get()

    def getPassword(self):
        return self._password.get()

def beforeUpload(source, target, env):
    root = tk.Tk()
    main = App(root)
    root.title("Upload Utility")
    main.pack(fill="x", expand=True)
    root.update()
    root.minsize(root.winfo_width() * 2, root.winfo_height())
    root.protocol("WM_DELETE_WINDOW", lambda: root.quit())
    root.mainloop()
    root.destroy()
    if (False == main.isAborted()):
        env.Replace(
            UPLOAD_PORT=main.getIPAddress(),
            UPLOAD_FLAGS=["--port=" + str(main.getPort()), "--auth=" + main.getPassword()]
        )
    else:
        print("Aborted. Using upload parameters from platform.ini")

env.AddPreAction("upload", beforeUpload)
