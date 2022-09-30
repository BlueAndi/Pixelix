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
Configure several build and upload options via GUI. It will apply the necessary
changes to the build environment and project environment.

It is important to call the same script as a pre- and post-type script via
platformio.ini

"""

################################################################################
# IMPORTS
################################################################################
import os.path
import tkinter as tk
from config_db import ConfigDb
from config import Config

################################################################################
# Globals
################################################################################

CONFIG_DB_FILE_NAME = "./scripts/config_db.json"
CONFIG_FILE_NAME = "config.json"

################################################################################
# Classes
################################################################################

class App(tk.Tk):
    """The application can setup a GUI to easy select one option per configuration item.
        The GUI will be started by calling the run() method.

        If a available configuration needs to be applied, the apply() method shall be called.
    """
    def __init__(self) -> None:
        super().__init__()

        self._db = ConfigDb()
        self._config = Config()
        self._selected_item_dict = {}

        self._config.load(CONFIG_FILE_NAME)

        if True is self._db.load(CONFIG_DB_FILE_NAME):
            item_names = self._db.get_item_names()
            for item_name in item_names:
                self._selected_item_dict[item_name] = tk.StringVar()
                selected_value_name = self._config.get_value_by_key(item_name)

                if None is selected_value_name:
                    selected_value_name = self._db.get_item_default_name(item_name)

                self._selected_item_dict[item_name].set(selected_value_name)

    def _create_gui(self):
        self.title("Setup")
        self.geometry("400x200")
        self.minsize(200, 100)

        row_index = 0
        item_names = self._db.get_item_names()
        for item_name in item_names:
            label = tk.Label(self, text=self._db.get_item_label(item_name))
            label.grid(column=0, row=row_index, sticky="nse")
            drop_down = tk.OptionMenu(self, self._selected_item_dict[item_name], *self._db.get_item_option_names(item_name))
            drop_down.grid(column=1, row=row_index, sticky="nswe")

            row_index += 1

        b_save = tk.Button(self, text="Save")
        b_save["command"] = self._save

        b_cancel = tk.Button(self, text="Quit")
        b_cancel["command"] = lambda: self.quit() and self.close()

        b_save.grid(column=0, row=row_index, columnspan=2, sticky="nswe")
        row_index += 1

        b_cancel.grid(column=0, row=row_index, columnspan=2, sticky="nswe")

        for col_index in range(2):
            tk.Grid.columnconfigure(self, col_index, weight=1)

        tk.Grid.rowconfigure(self, row_index, weight=1)

        self.protocol("WM_DELETE_WINDOW", lambda: self.quit() and self.close())

    def _save(self):
        item_names = self._db.get_item_names()
        for item_name in item_names:
            value = self._selected_item_dict[item_name].get()
            self._config.set_value_by_key(item_name, value)

        self._config.save(CONFIG_FILE_NAME)

    def run(self):
        """Setup the GUI for easy selecting of the option of every configuration item.
        """
        self._create_gui()
        self.mainloop()

    def apply(self, env):
        """Apply the configuration to the build environment and project environment
            of PlatformIO.

        Args:
            env (dict): Current construction environment

        Raises:
            ValueError: A construction environment is necessary.
        """

        item_names = self._db.get_item_names()
        for item_name in item_names:
            option_name = self._config.get_value_by_key(item_name)

            if option_name is not None:
                option = self._db.get_item_option(item_name, option_name)

                if "envAppend" in option:
                    if len(option["envAppend"].keys()) > 0:
                        env.Append(**option["envAppend"])

                if "envReplace" in option:
                    if len(option["envReplace"].keys()) > 0:
                        env.Replace(**option["envReplace"])

                # Keep it as example in case a script shall be executed:
                # env.SConscript(script, exports="env")

################################################################################
# Functions
################################################################################

################################################################################
# Main
################################################################################

# Import the current working construction environment which is available for pre-/post-type scripts.
# pylint: disable=undefined-variable
Import("env") # type: ignore

# The application GUI will only be started if a configuration file is missing.
# Otherwise the existing configuration will be applied.
app = App()

if False is os.path.isfile(CONFIG_FILE_NAME):
    app.run()
    if False is os.path.isfile(CONFIG_FILE_NAME):
        raise ValueError("No configuration available.")

if True is os.path.isfile(CONFIG_FILE_NAME):
    app.apply(env) # type: ignore

app.destroy()
