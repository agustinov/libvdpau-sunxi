#ifndef CSC_NEON_H
#define CSC_NEON_H
void NV12ToRGBA(unsigned char* yuv, unsigned char *puv, int w, int h, int* rgba);
void I420ToRGBA(unsigned char* yuv, unsigned char *pu, unsigned char *pv,int w, int h, int* rgba);
#endif
