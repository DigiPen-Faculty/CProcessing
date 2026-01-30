//---------------------------------------------------------
// file:	NoiseDemo.h
// 
// CProcessing features in this demo:
// CP_Image_CreateFromData	- create an image at runtime from raw pixel data
// CP_Image_UpdatePixelData - modify raw pixel data and update the image for drawing
// CP_Random_Noise			- use perlin noise to compute a random color
//---------------------------------------------------------

void NoiseDemoInit(void);
void NoiseDemoUpdate(void);
void NoiseDemoExit(void);