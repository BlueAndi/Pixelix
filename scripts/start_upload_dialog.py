"""
MIT License

Copyright (c) 2019 - 2022 Andreas Merkle <web@blue-andi.de>

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

################################################################################
# IMPORTS
################################################################################
import tkinter as tk
import os
from config import Config

################################################################################
# Globals
################################################################################

CONFIG_FILE_NAME = "config.json"
UPLOAD_CONFIG_FILE_NAME = "upload.json"

################################################################################
# Classes
################################################################################

class App(tk.Tk):
    """The main application class, which represents the main window.
    """
    def __init__(self, *args, **kwargs):
        tk.Tk.__init__(self, *args, **kwargs)

        self._upload_config = Config()
        self._ip_address    = tk.StringVar()
        self._port          = tk.IntVar()
        self._password      = tk.StringVar()
        self._is_aborted    = True

        self._upload_config.load(UPLOAD_CONFIG_FILE_NAME)

        # If no upload configuration is available or it doesn't contain the required
        # information, the default values will be set.
        ip_address = self._get_ip_address_from_model()
        port = self._get_port_from_model()
        password = self._get_password_from_model()

        if ip_address is None:
            ip_address = "192.168.x.x"

        if port is None:
            port = 3232

        if password is None:
            password = "maytheforcebewithyou"

        self._ip_address.set(ip_address)
        self._port.set(port)
        self._password.set(password)

    def _get_ip_address_from_model(self):
        return self._upload_config.get_value_by_key("ipAddress")

    def _set_password_to_model(self, password):
        self._upload_config.set_value_by_key("password", password)

    def _get_port_from_model(self):
        return self._upload_config.get_value_by_key("port")

    def _set_port_to_model(self, port):
        self._upload_config.set_value_by_key("port", port)

    def _get_password_from_model(self):
        return self._upload_config.get_value_by_key("password")

    def _set_ip_address_to_model(self, ip_address):
        self._upload_config.set_value_by_key("ipAddress", ip_address)

    def _update_gui(self):
        self._ip_address.set(self._get_ip_address_from_model())
        self._port.set(self._get_port_from_model())
        self._password.set(self._get_password_from_model())

    def _update_model(self):
        self._set_ip_address_to_model(self._ip_address.get())
        self._set_port_to_model(self._port.get())
        self._set_password_to_model(self._password.get())

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

        self.update()
        self.minsize(frame.winfo_width(), frame.winfo_height())
        self.protocol("WM_DELETE_WINDOW", lambda: self.quit() and self.close())

    def run(self, env):
        """Run the  main application.

            env (obj): Construction environment
        """
        self._setup_gui()
        self.mainloop()
        self._update_model()

        if False is self._is_aborted:
            env.Replace( # type: ignore
                UPLOAD_PORT=self._get_ip_address_from_model(),
                UPLOAD_FLAGS=["--port=" + str(self._get_port_from_model()), "--auth=" + self._get_password_from_model()]
            )
        else:
            print("Aborted. Using upload parameters from platform.ini")

    def _on_upload(self):
        self._update_model()
        self._upload_config.save()

        self._is_aborted = False
        self.quit()

################################################################################
# Functions
################################################################################

def before_upload(source, target, env): # pylint: disable=unused-argument
    """This function is called after build process and before the upload starts.

    Args:
        source (SCons.Node.FS.File): Source file as file object
        target (SCons.Node.Alias.Alias): Alias object
        env (SCons.Script.SConscript.SConsEnvironment): Construction environment
    """
    app = App()
    app.run(env)
    app.destroy()

################################################################################
# Main
################################################################################

if False is os.path.isfile(CONFIG_FILE_NAME):
    raise ValueError("Configuration  not found.")

config = Config()

if config.load(CONFIG_FILE_NAME) is False:
    raise ValueError("Configuration is corrupt.")

upload_type = config.get_value_by_key("uploadType")

if upload_type is None:
    raise ValueError("Upload type is missing in configuration.")

if upload_type == "OTA":
    # Import the current working construction environment which is available for pre-/post-type scripts.
    # pylint: disable=undefined-variable
    Import("env") # type: ignore

    # pylint: disable=undefined-variable
    env.AddPreAction("upload", before_upload) # type: ignore
    # pylint: disable=undefined-variable
    env.AddPreAction("uploadfs", before_upload) # type: ignore
