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

"""
For debugging purposes in the Python shell:

image = gimp.image_list()[0]
drawable = pdb.gimp_image_get_active_drawable(image)
pdb.python_fu_pixelix_create_preview(image, drawable)    
"""

################################################################################
# Imports
################################################################################
from gimpfu import * # pylint: disable=import-error,wildcard-import

################################################################################
# Variables
################################################################################

ICON_SIZE = 8
PREVIEW_PIXEL_SIZE = 10
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
    for x_rel in range(PREVIEW_PIXEL_SIZE):
        for y_rel in range(PREVIEW_PIXEL_SIZE):
            pdb.gimp_drawable_set_pixel(drawable, x_coord + x_rel, y_coord + y_rel, channels, color) # pylint: disable=undefined-variable

def create_layer(dst_image, layer_id, src_drawable, src_x, src_y):
    """Create layer to the destination image and generate the
        preview from the source image at the given position.

    Args:
        dst_image (obj): Destination image
        layer_id (number): Layer id
        src_drawable (obj): Source drawable
        src_x (number): Source image x position
        src_y (number): Source image y position
    """
    layer = pdb.gimp_layer_new(dst_image, dst_image.width, dst_image.height, RGB_IMAGE, str(layer_id), 100, NORMAL_MODE) # pylint: disable=undefined-variable
    pdb.gimp_image_add_layer(dst_image, layer, layer_id) # pylint: disable=undefined-variable

    for x_rel in range(ICON_SIZE):
        for y_rel in range(ICON_SIZE):
            channels, color = pdb.gimp_drawable_get_pixel(src_drawable, src_x + x_rel, src_y + y_rel) # pylint: disable=undefined-variable

            dst_x = x_rel * (PREVIEW_PIXEL_SIZE + BORDER_SIZE) + 1
            dst_y = y_rel * (PREVIEW_PIXEL_SIZE + BORDER_SIZE) + 1

            plot(layer, dst_x, dst_y, channels, color)

    columns = ICON_SIZE + 1
    for column in range(columns):
        x_coord = column * PREVIEW_PIXEL_SIZE + column
        for y_coord in range(dst_image.height):
            pdb.gimp_drawable_set_pixel(layer, x_coord, y_coord, BORDER_CHANNELS, BORDER_COLOR) # pylint: disable=undefined-variable

    rows = ICON_SIZE + 1
    for row in range(rows):
        y_coord = row * PREVIEW_PIXEL_SIZE + row
        for x_coord in range(dst_image.width):
            pdb.gimp_drawable_set_pixel(layer, x_coord, y_coord, BORDER_CHANNELS, BORDER_COLOR) # pylint: disable=undefined-variable

    pdb.gimp_drawable_update(layer, 0, 0, dst_image.width, dst_image.height) # pylint: disable=undefined-variable

def pixelix_create_preview(image, _):
    """Create preview from given image.

    Args:
        image (obj): Image
        _ (obj): Layer (not used)
    """

    drawable = pdb.gimp_image_get_active_drawable(image) # pylint: disable=undefined-variable

    tiles_x = image.width / ICON_SIZE
    tiles_y = image.height / ICON_SIZE

    preview_image_width = ICON_SIZE * (PREVIEW_PIXEL_SIZE + BORDER_SIZE) + 1
    preview_image_height = preview_image_width

    preview_image = pdb.gimp_image_new(preview_image_width, preview_image_height, RGB) # pylint: disable=undefined-variable

    layer_id = 0
    for tile_y in range(tiles_y - 1, -1, -1):
        for tile_x in range(tiles_x - 1, -1, -1):
            frame_x = tile_x * ICON_SIZE
            frame_y = tile_y * ICON_SIZE

            create_layer(preview_image, layer_id, drawable, frame_x, frame_y)
            layer_id += 1

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
