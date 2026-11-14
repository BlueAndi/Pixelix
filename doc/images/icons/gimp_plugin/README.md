# Icon preview plugin

The preview plugin requires GIMP 3 and its bundled Python support. On Windows,
run `copyToGimp.bat` to install it into the GIMP 3 user plug-ins directory.
On other platforms, put `gimp_create_preview.py` in a plug-in directory named
`gimp_create_preview` under GIMP's user plug-ins directory and make it
executable. Restart GIMP after installation.

Select one drawable in an image of at least 8×8 pixels, then use
**Filters → Pixelix → Create Preview**. Each complete 8×8 tile becomes a
separate preview layer; incomplete tiles at the right and bottom are ignored.
