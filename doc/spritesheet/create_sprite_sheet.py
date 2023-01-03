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
Create a sprite sheet from a bitmap (.bmp) file.

"""

import argparse
import json
import sys
from PIL import Image

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Create a sprite sheet data file.")

    parser.add_argument(
        "--frameWidth",
        dest="frame_width",
        type=int,
        help="Frame width in pixels"
    )

    parser.add_argument(
        "--frameHeight",
        dest="frame_height",
        type=int,
        help="Frame height in pixels"
    )

    parser.add_argument(
        "--framesCnt",
        dest="frames_cnt",
        type=int,
        required=False,
        default=0,
        help="Specify number of frames in case the sprite sheet contains gaps. (Default: Derived from texture and frame size.)"
    )

    parser.add_argument(
        "--fps",
        dest="fps",
        type=int,
        default=12,
        help="Frames per second. (Default: 12)"
    )

    parser.add_argument(
        "--pretty",
        dest="is_pretty",
        default=False,
        action='store_true',
        help="Create sprite sheet pretty. (Default: None)"
    )

    parser.add_argument(
        "--repeat",
        dest="repeat",
        type=str,
        required=False,
        default="true",
        help="Repeat animation infinite. If false it will run just once. (Default: true)"
    )

    parser.add_argument(
        "texture_filename",
        metavar="textureFilename",
        type=str,
        help="Texture image filename. Only .bmp is supported."
    )

    parser.add_argument(
        "sprite_sheet_filename",
        metavar="spriteSheetFilename",
        type=str,
        help="Sprite sheet filename, which to create (.sprite)."
    )

    args = parser.parse_args()

    if args.texture_filename.endswith(".bmp") is False:
        print(f'{args.texture_filename} is not supported. Only bitmap files (.bmp) files.')
        sys.exit(1)

    print(f'Loading texture image "{args.texture_filename}".')

    img = Image.open(args.texture_filename)

    texture_width   = img.width
    texture_height  = img.height

    img.close()

    print(f'Texture size is {texture_width} x {texture_height}.')

    frames_x = texture_width // args.frame_width
    frames_y = texture_height // args.frame_height

    print(f'{frames_x} calculated frames on the x-axis.')
    print(f'{frames_y} calculated frames on the y-axis.')

    frames_cnt = args.frames_cnt

    if frames_cnt == 0:
        frames_cnt = frames_x * frames_y

    print(f'{frames_cnt} frames in the sprite sheet.')
    
    repeat = True
    if args.repeat.lower() == "false":
        repeat = False

    doc = {
        "texture": {
            "width": texture_width,
            "height": texture_height,
            "frames": frames_cnt,
            "frame": {
                "width": args.frame_width,
                "height": args.frame_height
            },
            "fps": args.fps,
            "repeat": repeat
        }
    }

    sprite_sheet_filename = args.sprite_sheet_filename
    if sprite_sheet_filename.endswith(".sprite") is False:
        sprite_sheet_filename += ".sprite"

    INDENT = None
    if args.is_pretty is True:
        INDENT = 4

    with open(sprite_sheet_filename, 'w') as outfile:
        json.dump(doc, outfile, indent=INDENT)

    print(f'"{sprite_sheet_filename}" sprite sheet created.')
