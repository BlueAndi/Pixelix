"""
MIT License

Copyright (c) 2019 - 2023 Andreas Merkle <web@blue-andi.de>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

================================================================================
The upload dialog (OTA) is shown after the build process. In the upload dialog
it is possible to setup the configuration for the own pixelix device, like
ip-address and etc.

"""

import tkinter as tk
import json

# pylint: disable=undefined-variable
Import("env") # type: ignore

class UploadModel():
    """The upload model provides access to the configuration data.
    """
    def __init__(self, file_name):
        self._filename              = file_name
        self._data_json             = self._load(self._filename)
        self._default_ip_address    = "192.168.x.x"
        self._default_port          = 3232
        self._default_password      = "maytheforcebewithyou"

        if self._data_json is None:
            self._setup_model()

    def _load(self, file_name):
        data_json = None

        try:
            with open(file_name, encoding="utf-8") as json_file:
                data_json = json.load(json_file)
        except FileNotFoundError:
            pass

        return data_json

    def _save(self, file_name):
        try:
            with open(file_name, "w", encoding="utf-8") as json_file:
                json.dump(self._data_json, json_file, indent=4)
        except FileNotFoundError:
            pass

    def _setup_model(self):
        self._data_json = dict()
        self._data_json["ipAddress"] = self._default_ip_address
        self._data_json["port"]      = self._default_port
        self._data_json["password"]  = self._default_password

    def load(self):
        """Load configuration from filesystem.
        """
        self._load(self._filename)

    def save(self):
        """Store configuration in filesystem.
        """
        self._save(self._filename)

    def get_ip_address(self):
        """Get ip-address of remote device.

        Returns:
            str: IP-address
        """
        ip_address = self._default_ip_address

        if self._data_json is not None:
            if "ipAddress" in self._data_json:
                ip_address = self._data_json["ipAddress"]

        return ip_address

    def set_ip_address(self, ip_address):
        """Set ip-address of remote device.

        Args:
            ip_address (str): IP-Address
        """
        self._data_json["ipAddress"] = ip_address

    def get_port(self):
        """Get port for remote update.

        Returns:
            int: Port number
        """
        port = self._default_port

        if self._data_json is not None:
            if "port" in self._data_json:
                port = self._data_json["port"]

        return port

    def set_port(self, port):
        """Set port for remote update.

        Args:
            port (int): Port number
        """
        self._data_json["port"] = port

    def get_password(self):
        """Get password for remote access.

        Returns:
            str: Password
        """
        password = self._default_password

        if self._data_json is not None:
            if "password" in self._data_json:
                password = self._data_json["password"]

        return password

    def set_password(self, password):
        """Set password for remote access.

        Args:
            password (str): Password
        """
        self._data_json["password"] = password

class App(tk.Tk):
    """The main application class, which represents the main window.
    """
    def __init__(self, *args, **kwargs):
        tk.Tk.__init__(self, *args, **kwargs)

        self._upload_model  = UploadModel("upload.json")
        self._ip_address    = tk.StringVar()
        self._port          = tk.IntVar()
        self._password      = tk.StringVar()
        self._is_aborted    = True

    def _update_gui(self):
        self._ip_address.set(self._upload_model.get_ip_address())
        self._port.set(self._upload_model.get_port())
        self._password.set(self._upload_model.get_password())

    def _update_model(self):
        self._upload_model.set_ip_address(self._ip_address.get())
        self._upload_model.set_port(self._port.get())
        self._upload_model.set_password(self._password.get())

    def _setup_gui(self):

        self.title("Upload Utility")

        frame = tk.Frame(self)

        label_ip_address = tk.Label(frame, text="IP-Address:", anchor="w")
        input_ip_address = tk.Entry(frame, textvariable=self._ip_address)

        label_port = tk.Label(frame, text="Port:", anchor="w")
        input_port = tk.Entry(frame, textvariable=self._port)

        label_password = tk.Label(frame, text="Password:", anchor="w")
        input_password = tk.Entry(frame, textvariable=self._password, show="*")

        label_ip_address.pack(fill="x", expand=False)
        input_ip_address.pack(fill="x", expand=True)
        label_port.pack(fill="x", expand=False)
        input_port.pack(fill="x", expand=True)
        label_password.pack(fill="x", expand=False)
        input_password.pack(fill="x", expand=True)

        button_upload = tk.Button(frame, text="Upload", command=self._on_upload)
        button_upload.pack(fill="x", expand=False)

        frame.pack(fill="x", expand=True, padx=20, pady=20)

        self._update_gui()
        self.update()
        self.minsize(frame.winfo_width(), frame.winfo_height())
        self.protocol("WM_DELETE_WINDOW", lambda: self.quit() and self.close())

    def run(self):
        """Run the  main application.
        """
        self._setup_gui()
        self.mainloop()
        self._update_model()

        if False is self._is_aborted:
            env.Replace( # type: ignore
                UPLOAD_PORT=self._upload_model.get_ip_address(),
                UPLOAD_FLAGS=["--port=" + str(self._upload_model.get_port()), "--auth=" + self._upload_model.get_password()]
            )
        else:
            print("Aborted. Using upload parameters from platform.ini")

    def _on_upload(self):
        self._update_model()
        self._upload_model.save()

        self._is_aborted = False
        self.quit()

def before_upload(source, target, env): # pylint: disable=unused-argument
    """This function is called after build process and before the upload starts.

    Args:
        source (SCons.Node.FS.File): Source file as file object
        target (SCons.Node.Alias.Alias): Alias object
        env (SCons.Script.SConscript.SConsEnvironment): Environment object
    """
    app = App()
    app.run()

# pylint: disable=undefined-variable
env.AddPreAction("upload", before_upload) # type: ignore
# pylint: disable=undefined-variable
env.AddPreAction("uploadfs", before_upload) # type: ignore
