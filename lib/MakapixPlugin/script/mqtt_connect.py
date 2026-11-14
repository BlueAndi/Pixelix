"""Connect to the Makapix MQTT broker for a short diagnostic session."""

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
from __future__ import annotations

import time
from typing import Any

import paho.mqtt.client as mqtt

###############################################################################
# Variables
###############################################################################

# MQTT broker configuration
BROKER_URL = "dev.makapix.club"
BROKER_PORT = 8883
CLIENT_ID = "99c57cf3-0796-4bb6-9ff8-e87a2152bcb3"
DURATION = 300  # 5 minutes in seconds

###############################################################################
# Classes
###############################################################################

###############################################################################
# Functions
###############################################################################


def on_connect(_client: mqtt.Client, _userdata: Any, _flags: dict[str, int], rc: int) -> None:
    """Report the broker's connection result.

    Args:
        _client: MQTT client.
        _userdata: User data passed to the MQTT client.
        _flags: Broker response flags.
        rc: MQTT connection return code.

    Returns:
        None.
    """
    if rc == 0:
        print("Connected to MQTT broker")
    else:
        print(f"Connection failed with code {rc}")


def on_disconnect(_client: mqtt.Client, _userdata: Any, _rc: int) -> None:
    """Report that the broker disconnected the client.

    Args:
        _client: MQTT client.
        _userdata: User data passed to the MQTT client.
        _rc: MQTT disconnection return code.

    Returns:
        None.
    """
    print("Disconnected from MQTT broker")


def main() -> None:
    """Connect securely, wait for the configured duration, and disconnect.

    Returns:
        None.
    """
    client = mqtt.Client(client_id=CLIENT_ID)
    client.on_connect = on_connect
    client.on_disconnect = on_disconnect

    # Use TLS for secure connection
    client.tls_set()

    try:
        client.connect(BROKER_URL, BROKER_PORT, keepalive=60)
        client.loop_start()

        time.sleep(DURATION)

    finally:
        client.loop_stop()
        client.disconnect()


###############################################################################
# Main
###############################################################################

if __name__ == "__main__":
    main()
