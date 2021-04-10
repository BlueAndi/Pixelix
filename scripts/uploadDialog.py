# MIT License
# 
# Copyright (c) 2019 - 2021 Andreas Merkle <web@blue-andi.de>
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
import json

Import("env")

class UploadModel():
    def __init__(self, fileName):
        self._FILENAME          = fileName
        self._dataJson          = self._load(self._FILENAME)
        self.DEFAULT_IP_ADDRESS = "192.168.x.x"
        self.DEFAULT_PORT       = 3232
        self.DEFAULT_PASSWORD   = "maytheforcebewithyou"

        if (None == self._dataJson):
            self._setupModel()

    def _load(self, fileName):
        dataJson = None

        try:
            with open(fileName) as jsonFile:
                dataJson = json.load(jsonFile)                
        except:
            pass
    
        return dataJson

    def _save(self, fileName):
        try:
            with open(fileName, "w") as jsonFile:
                json.dump(self._dataJson, jsonFile, indent=4)
        except:
            pass
    
    def _setupModel(self):
        self._dataJson = dict()
        self._dataJson["ipAddress"] = self.DEFAULT_IP_ADDRESS
        self._dataJson["port"]      = self.DEFAULT_PORT
        self._dataJson["password"]  = self.DEFAULT_PASSWORD

    def load(self):
        self._load(self._FILENAME)

    def save(self):
        self._save(self._FILENAME)

    def getIPAddress(self):
        ipAddress = self.DEFAULT_IP_ADDRESS

        if (None != self._dataJson):
            if ("ipAddress" in self._dataJson):
                ipAddress = self._dataJson["ipAddress"]
        
        return ipAddress

    def setIPAddress(self, ipAddress):
        self._dataJson["ipAddress"] = ipAddress

    def getPort(self):
        port = self.DEFAULT_PORT

        if (None != self._dataJson):
            if ("port" in self._dataJson):
                port = self._dataJson["port"]
        
        return port

    def setPort(self, port):
        self._dataJson["port"] = port

    def getPassword(self):
        password = self.DEFAULT_PASSWORD

        if (None != self._dataJson):
            if ("password" in self._dataJson):
                password = self._dataJson["password"]
        
        return password

    def setPassword(self, password):
        self._dataJson["password"] = password

class App(tk.Tk):
    def __init__(self, *args, **kwargs):
        tk.Tk.__init__(self, *args, **kwargs)

        self._uploadModel   = UploadModel("upload.json")
        self._ipAddress     = tk.StringVar()
        self._port          = tk.IntVar()
        self._password      = tk.StringVar()
        self._isAborted     = True

    def _updateGUI(self):
        self._ipAddress.set(self._uploadModel.getIPAddress())
        self._port.set(self._uploadModel.getPort())
        self._password.set(self._uploadModel.getPassword())

    def _updateModel(self):
        self._uploadModel.setIPAddress(self._ipAddress.get())
        self._uploadModel.setPort(self._port.get())
        self._uploadModel.setPassword(self._password.get())

    def _setupGUI(self):

        self.title("Upload Utility")

        frame = tk.Frame(self)

        labelIpAddress = tk.Label(frame, text="IP-Address:", anchor="w")
        inputIpAddress = tk.Entry(frame, textvariable=self._ipAddress)

        labelPort = tk.Label(frame, text="Port:", anchor="w")
        inputPort = tk.Entry(frame, textvariable=self._port)

        labelPassword = tk.Label(frame, text="Password:", anchor="w")
        inputPassword = tk.Entry(frame, textvariable=self._password, show="*")

        labelIpAddress.pack(fill="x", expand=False)
        inputIpAddress.pack(fill="x", expand=True)
        labelPort.pack(fill="x", expand=False)
        inputPort.pack(fill="x", expand=True)
        labelPassword.pack(fill="x", expand=False)
        inputPassword.pack(fill="x", expand=True)

        buttonUpload = tk.Button(frame, text="Upload", command=self._onUpload)
        buttonUpload.pack(fill="x", expand=False)

        frame.pack(fill="x", expand=True, padx=20, pady=20)

        self._updateGUI()
        self.update()
        self.minsize(frame.winfo_width(), frame.winfo_height())
        self.protocol("WM_DELETE_WINDOW", lambda: self.quit())

    def run(self):
        self._setupGUI()
        self.mainloop()
        self._updateModel()

        if (False == self._isAborted):
            env.Replace(
                UPLOAD_PORT=self._uploadModel.getIPAddress(),
                UPLOAD_FLAGS=["--port=" + str(self._uploadModel.getPort()), "--auth=" + self._uploadModel.getPassword()]
            )
        else:
            print("Aborted. Using upload parameters from platform.ini")

    def _onUpload(self):
        self._updateModel()
        self._uploadModel.save()

        self._isAborted = False
        self.quit()

def beforeUpload(source, target, env):
    app = App()
    app.run()

env.AddPreAction("upload", beforeUpload)
env.AddPreAction("uploadfs", beforeUpload)
