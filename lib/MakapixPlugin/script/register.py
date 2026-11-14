"""Register the Makapix plugin as a player."""

# MIT License
#
# Copyright (c) 2019 - 2026 Andreas Merkle <web@blue-andi.de>
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

###############################################################################
# Imports
###############################################################################
import sys
from requests import post

###############################################################################
# Variables
###############################################################################

###############################################################################
# Classes
###############################################################################

###############################################################################
# Functions
###############################################################################


def register_makapix_player() -> int:
    """Register a Makapix player and obtain a player key.

    Returns:
        Zero on success, one on failure.
    """
    status = 0

    url = "https://makapix.club/api/player/provision"
    headers = {
        "Content-Type": "application/json"
    }
    payload = {
        "device_model": "Pixelix",
        "firmware_version": "9.0.2"
    }

    response = post(url, json=payload, headers=headers, timeout=10)

    if response.status_code == 201:
        data = response.json()
        player_key = data.get("player_key")
        registration_code = data.get("registration_code")
        expires_at = data.get("registration_code_expires_at")
        mqtt_broker = data.get("mqtt_broker", {})
        mqtt_host = mqtt_broker.get("host")
        mqtt_port = mqtt_broker.get("port")

        print(f"Makapix player registered successfully. Player Key: {player_key}")
        print(f"Registration Code: {registration_code}")
        print(f"Expires At: {expires_at}")
        print(f"MQTT Broker Host: {mqtt_host}")
        print(f"MQTT Broker Port: {mqtt_port}")

    else:
        print(f"Failed to register Makapix player. Status Code: {response.status_code}")
        status = 1

    return status


###############################################################################
# Main
###############################################################################

if __name__ == "__main__":
    sys.exit(register_makapix_player())
