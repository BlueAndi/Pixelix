"""
This script adds a factory binary to FLASH_EXTRA_IMAGES with the offset defined in the 
partition table. This way the factory binary will be flashed together with the firmware 
binary during the upload process.
"""

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

# pyright: reportUndefinedVariable=false

################################################################################
# Imports
################################################################################

import os
import sys
from typing import Optional
Import("env") # pylint: disable=undefined-variable

################################################################################
# Variables
################################################################################

FACTORY_PARTITION_NAME = "factory"
PROJECT_DIR = env.subst("$PROJECT_DIR") # pylint: disable=undefined-variable
FACTORY_BINARY = env.GetProjectOption("custom_factory_binary", "") # pylint: disable=undefined-variable
ENV_NAME = env["PIOENV"] # pylint: disable=undefined-variable

################################################################################
# Classes
################################################################################

################################################################################
# Functions
################################################################################

def get_partition_table(env): # pylint: disable=undefined-variable
    """
    Get the partition table from the environment.
    
    Args:
        env: The environment object containing project and build directories.
    
    Returns:
        List[Dict[str, str]]: A list of partitions defined in the partition table CSV file.
    """
    partition_table = []
    partition_table_file_name = PROJECT_DIR + "/" + env.BoardConfig().get("build.partitions")

    if partition_table_file_name and os.path.isfile(partition_table_file_name):
        with open(partition_table_file_name, "r", encoding="utf-8") as f:
            for line in f:
                line = line.strip()
                # Skip comments and empty lines
                if not line or line.startswith("#"):
                    continue
                # Partition table CSV format: name, type, subtype, offset, size, flags
                parts = [x.strip() for x in line.split(",")]
                if len(parts) < 5:
                    continue  # Not a valid partition line
                partition = {
                    "name": parts[0],
                    "type": parts[1],
                    "subtype": parts[2],
                    "offset": parts[3],
                    "size": parts[4],
                    "flags": parts[5] if len(parts) > 5 else ""
                }
                partition_table.append(partition)
    else:
        print("No partition table found or PARTITIONS_TABLE_CSV not set.", file=sys.stderr)

    return partition_table

def get_factory_image() -> Optional[str]:
    """
    Get the factory binary file depending on the active environment.
    
    Returns:
        str: A path to a factory binary file.
    """
    factory_image = ""

    if FACTORY_BINARY != "":
        factory_image = os.path.join(PROJECT_DIR, FACTORY_BINARY)

        if os.path.isfile(factory_image):
            return factory_image
        else:
            print(f"Factory binary: {factory_image} does not exist!")
    else:
        print(f"No factory binary specified for environment: {ENV_NAME}!")

    return None

def handle_upload() -> None:
    """
    Handle the upload process by adding the factory binary to FLASH_EXTRA_IMAGES.
    """
    factory_image = get_factory_image()
    partition_table = get_partition_table(env) # pylint: disable=undefined-variable
    factory_offset = 0

    if factory_image is not None:
        # Get the offset for the factory image from the partition table.
        for partition in partition_table:
            if partition["name"] == FACTORY_PARTITION_NAME:
                factory_offset = partition["offset"]

        if factory_offset != 0:
            env.Append( # pylint: disable=undefined-variable
                FLASH_EXTRA_IMAGES=[
                    (f"{factory_offset}", f"{factory_image}")
                ]
            )
        else:
            raise ValueError(f"No offset found for partition: {FACTORY_PARTITION_NAME}!")
    else:
        raise FileNotFoundError("No factory image found!")


################################################################################
# Main
################################################################################

if "upload" in sys.argv:
    handle_upload()
