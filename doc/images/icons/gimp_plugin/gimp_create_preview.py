#!/usr/bin/env python3
"""Generate a pixel-grid preview of an icon in GIMP 3."""

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

################################################################################
# Imports
################################################################################
from __future__ import annotations

import sys
from typing import Any

# These introspection bindings are provided by the GIMP runtime.
# pylint: disable-next=import-error
import gi

gi.require_version("Gimp", "3.0")
gi.require_version("Gegl", "0.4")
# pylint: disable-next=import-error,wrong-import-position
from gi.repository import Gegl, Gimp, GLib  # noqa: E402

################################################################################
# Variables
################################################################################
PROCEDURE_NAME = "python-fu-pixelix-create-preview"
ICON_SIZE = 8
PREVIEW_PIXEL_SIZE = 10
BORDER_SIZE = 1
PREVIEW_SIZE = ICON_SIZE * (PREVIEW_PIXEL_SIZE + BORDER_SIZE) + BORDER_SIZE

################################################################################
# Classes
################################################################################


class PreviewPlugin(Gimp.PlugIn):
    """Register the preview generator with GIMP."""

    def do_query_procedures(self) -> list[str]:
        """List the procedures provided by this plugin.

        Args:
            self: Plugin instance.

        Returns:
            Names of the procedures.
        """
        return [PROCEDURE_NAME]

    def do_create_procedure(self, name: str) -> Gimp.Procedure | None:
        """Create the requested GIMP procedure.

        Args:
            self: Plugin instance.
            name: Name requested by GIMP.

        Returns:
            The registered procedure, or None for an unknown name.
        """
        procedure = None
        if name == PROCEDURE_NAME:
            procedure = Gimp.ImageProcedure.new(self, name, Gimp.PDBProcType.PLUGIN, create_preview, None)
            procedure.set_image_types("*")
            procedure.set_sensitivity_mask(Gimp.ProcedureSensitivityMask.DRAWABLE)
            procedure.set_menu_label("Create Preview")
            procedure.add_menu_path("<Image>/Filters/Pixelix")
            procedure.set_documentation("Preview Creator", "Create a preview image from 8x8 icons.", name)
            procedure.set_attribution("Andreas Merkle", "Andreas Merkle", "2023")
        return procedure

################################################################################
# Functions
################################################################################


def set_pixel(drawable: Gimp.Drawable, x_coord: int, y_coord: int, color: Gegl.Color) -> None:
    """Set one pixel and report an unsuccessful GIMP operation.

    Args:
        drawable: Destination drawable.
        x_coord: Horizontal coordinate.
        y_coord: Vertical coordinate.
        color: Pixel color.

    Returns:
        None.

    Raises:
        RuntimeError: GIMP could not set the pixel.
    """
    if not drawable.set_pixel(x_coord, y_coord, color):
        raise RuntimeError(f"Unable to set preview pixel at ({x_coord}, {y_coord})")


def draw_tile(layer: Gimp.Layer, src_drawable: Gimp.Drawable, src_x: int, src_y: int) -> None:
    """Draw the enlarged pixels of one source tile.

    Args:
        layer: Destination layer.
        src_drawable: Source drawable.
        src_x: Horizontal position of the source tile.
        src_y: Vertical position of the source tile.

    Returns:
        None.
    """
    for x_rel in range(ICON_SIZE):
        for y_rel in range(ICON_SIZE):
            color = src_drawable.get_pixel(src_x + x_rel, src_y + y_rel)
            dst_x = x_rel * (PREVIEW_PIXEL_SIZE + BORDER_SIZE) + BORDER_SIZE
            dst_y = y_rel * (PREVIEW_PIXEL_SIZE + BORDER_SIZE) + BORDER_SIZE
            for x_offset in range(PREVIEW_PIXEL_SIZE):
                for y_offset in range(PREVIEW_PIXEL_SIZE):
                    set_pixel(layer, dst_x + x_offset, dst_y + y_offset, color)


def create_layer(dst_image: Gimp.Image, layer_id: int, src_drawable: Gimp.Drawable, src_x: int, src_y: int) -> None:
    """Create a preview layer for an 8x8 tile of the source drawable.

    Args:
        dst_image: Destination image.
        layer_id: Layer name and stack position.
        src_drawable: Source drawable.
        src_x: Horizontal position of the source tile.
        src_y: Vertical position of the source tile.

    Returns:
        None.

    Raises:
        RuntimeError: GIMP could not create or update the layer.
    """
    layer = Gimp.Layer.new(dst_image, str(layer_id), PREVIEW_SIZE, PREVIEW_SIZE,
                           Gimp.ImageType.RGB_IMAGE, 100.0, Gimp.LayerMode.NORMAL)
    if layer is None or not dst_image.insert_layer(layer, None, layer_id):
        raise RuntimeError(f"Unable to create preview layer {layer_id}")

    draw_tile(layer, src_drawable, src_x, src_y)
    border = Gegl.Color.new("white")
    for coordinate in range(ICON_SIZE + 1):
        position = coordinate * (PREVIEW_PIXEL_SIZE + BORDER_SIZE)
        for offset in range(PREVIEW_SIZE):
            set_pixel(layer, position, offset, border)
            set_pixel(layer, offset, position, border)
    if not layer.update(0, 0, PREVIEW_SIZE, PREVIEW_SIZE):
        raise RuntimeError(f"Unable to update preview layer {layer_id}")


# GIMP's ImageProcedure callback supplies six positional arguments.
# pylint: disable-next=too-many-arguments,too-many-positional-arguments
def create_preview(procedure: Gimp.Procedure, run_mode: Gimp.RunMode, image: Gimp.Image,
                   drawables: list[Gimp.Drawable], config: Gimp.ProcedureConfig, data: Any) -> Any:
    """Create and display a preview layer for each complete 8x8 image tile.

    Args:
        procedure: Invoked GIMP procedure.
        run_mode: Invocation mode.
        image: Source image.
        drawables: Selected source drawables.
        config: Procedure configuration.
        data: User data supplied when registering the procedure.

    Returns:
        GIMP procedure return values indicating success or invalid input.
    """
    del run_mode, config, data
    if len(drawables) != 1 or image.get_width() < ICON_SIZE or image.get_height() < ICON_SIZE:
        return procedure.new_return_values(
            Gimp.PDBStatusType.CALLING_ERROR, GLib.Error("Select one drawable in an image of at least 8x8 pixels."))

    Gegl.init(None)
    preview_image = Gimp.Image.new(PREVIEW_SIZE, PREVIEW_SIZE, Gimp.ImageBaseType.RGB)
    if preview_image is None:
        raise RuntimeError("Unable to create preview image")
    layer_id = 0
    for tile_y in range(image.get_height() // ICON_SIZE - 1, -1, -1):
        for tile_x in range(image.get_width() // ICON_SIZE - 1, -1, -1):
            create_layer(preview_image, layer_id, drawables[0], tile_x * ICON_SIZE, tile_y * ICON_SIZE)
            layer_id += 1
    if Gimp.Display.new(preview_image) is None:
        raise RuntimeError("Unable to display preview image")
    return procedure.new_return_values(Gimp.PDBStatusType.SUCCESS, None)

################################################################################
# Main
################################################################################


if __name__ == "__main__":
    Gimp.main(PreviewPlugin.__gtype__, sys.argv)
