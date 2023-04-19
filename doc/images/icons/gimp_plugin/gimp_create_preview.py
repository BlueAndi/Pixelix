#!/usr/bin/python

# MIT License
#
# Copyright (c) 2019 - 2023 Andreas Merkle (web@blue-andi.de)
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

################################################################################
# Imports
################################################################################
from gimpfu import * # pylint: disable=import-error,wildcard-import

################################################################################
# Variables
################################################################################

PIXEL_SIZE = 10
BORDER_SIZE = 1
BORDER_CHANNELS = 3
BORDER_COLOR = (255, 255, 255)

################################################################################
# Classes
################################################################################

################################################################################
# Functions
################################################################################

def plot(drawable, x_coord, y_coord, channels, color):
    """Plot a pixel in the preview.

    Args:
        drawable (obj): Preview drawable
        x_coord (number): x-coordinate of upper left pixel corner
        y_coord (number): y-coordinate of upper left pixel corner
        channels (number): Number of color channels
        color ((number, number, number))): Color
    """
    x_rel = 0
    y_rel = 0
    for x_rel in range(PIXEL_SIZE):
        for y_rel in range(PIXEL_SIZE):
            pdb.gimp_drawable_set_pixel(drawable, x_coord + x_rel, y_coord + y_rel, channels, color) # pylint: disable=undefined-variable

def pixelix_create_preview(image, _):
    """Create preview from given image.

    Args:
        image (obj): Image
        _ (obj): Layer (not used)
    """

    drawable = pdb.gimp_image_get_active_drawable(image) # pylint: disable=undefined-variable

    preview_image_width = image.width * (PIXEL_SIZE + BORDER_SIZE) + 1
    preview_image_height = image.height * (PIXEL_SIZE + BORDER_SIZE) + 1

    preview_image = pdb.gimp_image_new(preview_image_width, preview_image_height, RGB) # pylint: disable=undefined-variable
    preview_image_layer = pdb.gimp_layer_new(preview_image, preview_image_width, preview_image_height, RGB_IMAGE, "base", 100, NORMAL_MODE) # pylint: disable=undefined-variable
    pdb.gimp_image_add_layer(preview_image, preview_image_layer, 0) # pylint: disable=undefined-variable

    preview_image_active_drawable = pdb.gimp_image_get_active_drawable(preview_image) # pylint: disable=undefined-variable

    for icon_image_x in range(image.width):
        for icon_image_y in range(image.height):
            channels, color = pdb.gimp_drawable_get_pixel(drawable, icon_image_x, icon_image_y) # pylint: disable=undefined-variable
            preview_image_x = icon_image_x * (PIXEL_SIZE + BORDER_SIZE) + 1
            preview_image_y = icon_image_y * (PIXEL_SIZE + BORDER_SIZE) + 1
            plot(preview_image_active_drawable, preview_image_x, preview_image_y, channels, color)

    columns = image.width + 1
    for column in range(columns):
        x_coord = column * PIXEL_SIZE + column
        for y_coord in range(preview_image_height):
            pdb.gimp_drawable_set_pixel(preview_image_active_drawable, x_coord, y_coord, BORDER_CHANNELS, BORDER_COLOR) # pylint: disable=undefined-variable

    rows = image.height + 1
    for row in range(rows):
        y_coord = row * PIXEL_SIZE + row
        for x_coord in range(preview_image_width):
            pdb.gimp_drawable_set_pixel(preview_image_active_drawable, x_coord, y_coord, BORDER_CHANNELS, BORDER_COLOR) # pylint: disable=undefined-variable

    pdb.gimp_drawable_update(preview_image_active_drawable, 0, 0, preview_image_width, preview_image_height) # pylint: disable=undefined-variable
    pdb.gimp_display_new(preview_image) # pylint: disable=undefined-variable

################################################################################
# Main
################################################################################

register( # pylint: disable=undefined-variable
	"python_fu_pixelix_create_preview",
    "Preview Creator",
	"Create a preview image from a 8x8 icon.",
	"Andreas Merkle",
	"MIT License",
	"2023",
	"<Image>/Filters/Pixelix/Create_Preview",
	"*",
	[],
	[],
	pixelix_create_preview)

main() # pylint: disable=undefined-variable
