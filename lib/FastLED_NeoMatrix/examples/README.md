FastLED_NeoMatrix_SmartMatrix_LEDMatrix_GFX_Demos is populated from
https://github.com/marcmerlin/FastLED_NeoMatrix_SmartMatrix_LEDMatrix_GFX_Demos/

Please run git submodule update --init --recursive

MatrixGFXDemo shows how to use this drier directly without worrying
about the combined neomatrix_config.h backend.

Other examples under FastLED_NeoMatrix_SmartMatrix_LEDMatrix_GFX_Demos use the
complex looking neomatrix_config.h that allows to move your code back and forth
between the different drivers that use the Framebuffer::GFX backend (like the
FastLED::NeoMatrix, SmartMatrix::GFX, and FastLED_SPITFT::GFX drivers)
