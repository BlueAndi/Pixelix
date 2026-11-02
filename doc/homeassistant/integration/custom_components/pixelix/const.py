"""Constants of the Pixelix integration."""

# MIT License
#
# Copyright (c) 2019 - 2026 Andreas Merkle (web@blue-andi.de)
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
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

###############################################################################
# Variables
###############################################################################
DOMAIN = "pixelix"

MANUFACTURER = "BlueAndi & Friends"
MODEL = "Pixelix"

SERVICE_NOTIFY = "notify"

# Fields of the notify action.
ATTR_MESSAGE = "message"
ATTR_DURATION = "duration"
ATTR_ICON_FILE_NAME = "icon_file_name"
ATTR_MELODY = "melody"
ATTR_VOLUME = "volume"

# Limits of the Pixelix firmware, see Topics.cpp and RTTTL.h. The text and
# the melody are checked by the firmware in bytes (strlen).
MAX_TEXT_BYTES = 256
MAX_MELODY_BYTES = 256
MAX_VOLUME = 10
MAX_UINT32 = 4294967295

# Default of the web server port. A device may use another one.
DEFAULT_PORT = 80

# Default web login of a Pixelix device.
DEFAULT_USERNAME = "luke"
DEFAULT_PASSWORD = "skywalker"

REST_API_PATH = "/rest/api/v1"

# Timeout of a REST request in seconds.
REQUEST_TIMEOUT = 10

###############################################################################
# Classes
###############################################################################

###############################################################################
# Functions
###############################################################################
