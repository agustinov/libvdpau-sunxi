#ifdef HAS_NEON
#include <arm_neon.h>
#endif

void NV12ToRGBA(unsigned char* yuv, unsigned char *puv, int w, int h, int* rgba)
{
    for (int i=0; i<h; ++i)
    {
        unsigned char* dst = (unsigned char*)(rgba + w*i);
        unsigned char* y = yuv + w*i;
        unsigned char* uv = yuv + w*h + w*(i/2);
        int count = w;

        if (puv) uv = puv + (w >> 1)*(i >> 1);
#ifdef HAS_NEON
        int c = count/16;
        asm volatile(
                "movs r4, %[c]\t\n"
                "beq 2f\t\n"
                "vmov.u8 d7, #255\t\n"//Alpha
                "vmov.u8 d3, #255\t\n"//Alpha
                "vmov.s16 q11, #90\t\n"
                "vmov.s16 q12, #128\t\n"
                "vmov.s16 q13, #21\t\n"
                "vmov.s16 q14, #46\t\n"
                "vmov.s16 q15, #113\t\n"
                "1:\t\n"
                /* Y1 Y2 lod */
                "vld2.8 {d8, d9}, [%[y]]!\t\n"//Y1, Y2
                /* UV load */
                "vld2.8 {d0, d1}, [%[uv]]!\t\n"//u, v
                "vmovl.u8  q5, d0\t\n"
                "vmovl.u8  q6, d1\t\n"
                "vsub.i16 q5,q5, q12\t\n"//U
                "vsub.i16 q6,q6, q12\t\n"//V
                //First RGBA
                "vshll.u8 q7, d8, #6\t\n"
                "vshll.u8 q8, d8, #6\t\n"
                "vshll.u8 q9, d8, #6\t\n"
                "vmla.i16 q7, q6, q11\t\n"
                "vmls.i16 q8, q5, q13\t\n"
                "vmls.i16 q8, q6, q14\t\n"
                "vmla.i16 q9, q5, q15\t\n"
 
                "vshr.s16 q7, q7, #6\t\n"
                "vshr.s16 q8, q8, #6\t\n"
                "vshr.s16 q9, q9, #6\t\n"
                "vmov.s16 q10, #0\t\n"
                "vmax.s16 q7, q7, q10\t\n"
                "vmax.s16 q8, q8, q10\t\n"
                "vmax.s16 q9, q9, q10\t\n"
                "vmov.u16 q10, #255\t\n"
                "vmin.u16 q7, q7, q10\t\n"
                "vmin.u16 q8, q8, q10\t\n"
                "vmin.u16 q9, q9, q10\t\n"
                "vmovn.s16 d2, q7\t\n"
                "vmovn.s16 d1, q8\t\n"
                "vmovn.s16 d0, q9\t\n"
 
                //Second RGBA
                "vshll.u8 q7, d9, #6\t\n"
                "vshll.u8 q8, d9, #6\t\n"
                "vshll.u8 q9, d9, #6\t\n"
                "vmla.i16 q7, q6, q11\t\n"
                "vmls.i16 q8, q5, q13\t\n"
                "vmls.i16 q8, q6, q14\t\n"
                "vmla.i16 q9, q5, q15\t\n"
                "vshr.s16 q7, q7, #6\t\n"
                "vshr.s16 q8, q8, #6\t\n"
                "vshr.s16 q9, q9, #6\t\n"
                "vmov.s16 q10, #0\t\n"
                "vmax.s16 q7, q7, q10\t\n"
                "vmax.s16 q8, q8, q10\t\n"
                "vmax.s16 q9, q9, q10\t\n"
                "vmov.u16 q10, #255\t\n"
                "vmin.u16 q7, q7, q10\t\n"
                "vmin.u16 q8, q8, q10\t\n"
                "vmin.u16 q9, q9, q10\t\n"
                "vmovn.s16 d6, q7\t\n"
                "vmovn.s16 d5, q8\t\n"
                "vmovn.s16 d4, q9\t\n"
 
                "vtrn.8 d2,d6\t\n"
                "vtrn.16 d2,d6\t\n"
                "vtrn.32 d2,d6\t\n"
 
                "vtrn.8 d1,d5\t\n"
                "vtrn.16 d1,d5\t\n"
                "vtrn.32 d1,d5\t\n"
 
                "vtrn.8 d0,d4\t\n"
                "vtrn.16 d0,d4\t\n"
                "vtrn.32 d0,d4\t\n"
 
                "vst4.8 {d0-d3}, [%[dst]]!\t\n"
                "vst4.8 {d4-d7}, [%[dst]]!\t\n"
 
                "subs r4, r4, #1\t\n"
                "bne 1b\t\n"
                "2:\t\n"
                : [dst] "+r" (dst), [y] "+r" (y), [uv] "+r" (uv), [c] "+r" (c)
                :
                : "r4", "cc","memory", "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7","d8","d9","d10","d11","d12","d13","d14","d15","d16","d17","d18","d19","d20","d21","d22","d23","d24","d25","d26","d27","d28","d29","d30","d31"
                );
        count%=16;
#endif
 
        /* rest */
        int r, g, b;
        while (count > 1) {
            unsigned char _y = y[0];
            unsigned char _u = uv[0];
            unsigned char _v = uv[1];
            r = _y + ((179*(_v-128))>>7);
            g = _y - ((43*(_u-128) - 91*(_v-128))>>7);
            b = _y + ((227*(_u-128))>>7);
            r = r<0?0:r;r=r>255?255:r;
            g = g<0?0:g;g=g>255?255:g;
            b = b<0?0:b;b=b>255?255:b;
            dst[0] = b;
            dst[1] = g;
            dst[2] = r;
            dst[3] = 0xFF;
 
            y++;
            dst+=4;
            _y = y[0];
 
            r = _y + ((179*(_v-128))>>7);
            g = _y - ((43*(_u-128) - 91*(_v-128))>>7);
            b = _y + ((227*(_u-128))>>7);
            r = r<0?0:r;r=r>255?255:r;
            g = g<0?0:g;g=g>255?255:g;
            b = b<0?0:b;b=b>255?255:b;
 
            dst[0] = b;
            dst[1] = g;
            dst[2] = r;
            dst[3] = 0xFF;
            y++;
            uv+=2;
            dst+=4;
 
            count-=2;
        }

        if (count > 0) {
            unsigned char _y = y[0];
            unsigned char _u = uv[0];
            unsigned char _v = uv[1];
            r = _y + ((179*(_v-128))>>7);
            g = _y - ((43*(_u-128) - 91*(_v-128))>>7);
            b = _y + ((227*(_u-128))>>7);
            r = r<0?0:r;r=r>255?255:r;
            g = g<0?0:g;g=g>255?255:g;
            b = b<0?0:b;b=b>255?255:b;
            dst[0] = b;
            dst[1] = g;
            dst[2] = r;
            dst[3] = 0xFF;
        }
    }
}

/*
 BGRA
*/
void I420ToRGBA(unsigned char* yuv, unsigned char *pu, unsigned char *pv,
                int w, int h, int* rgba) {

    for (int i=0; i<h; ++i) {
        int y_off, v_off, u_off;
        y_off = 0;
        v_off = (w*h);
        u_off = v_off + (v_off >> 2);

        unsigned char* dst = (unsigned char*)(rgba + w*i);
        unsigned char* y = yuv + y_off + w*i;
        unsigned char* u = yuv + v_off + (w >> 1)*(i >> 1);
        unsigned char* v = yuv + u_off + (w >> 1)*(i >> 1);

        if (pu) u = pu + (w >> 1)*(i >> 1);
        if (pv) v = pv + (w >> 1)*(i >> 1);

        int count = w;
#ifdef HAS_NEON
        int c = count/16;
        asm volatile(
                "movs r4, %[c]\t\n"
                "beq 2f\t\n"
                "vmov.u8 d7, #255\t\n"//Alpha
                "vmov.u8 d3, #255\t\n"//Alpha
                "vmov.s16 q11, #90\t\n"
                "vmov.s16 q12, #128\t\n"
                "vmov.s16 q13, #21\t\n"
                "vmov.s16 q14, #46\t\n"
                "vmov.s16 q15, #113\t\n"
                "1:\t\n"
                /*Y1 Y2 load */
                "vld2.8 {d8, d9}, [%[y]]!\t\n"//Y1, Y2
                /*u load */
                "vld1.8 {d0}, [%[u]]!\t\n"//u
                /*v load */
                "vld1.8 {d1}, [%[v]]!\t\n"//v
                "vmovl.u8  q5, d0\t\n"
                "vmovl.u8  q6, d1\t\n"
                "vsub.i16 q5,q5, q12\t\n"//U
                "vsub.i16 q6,q6, q12\t\n"//V
                //First RGBA
                "vshll.u8 q7, d8, #6\t\n"
                "vshll.u8 q8, d8, #6\t\n"
                "vshll.u8 q9, d8, #6\t\n"
                "vmla.i16 q7, q6, q11\t\n"
                "vmls.i16 q8, q5, q13\t\n"
                "vmls.i16 q8, q6, q14\t\n"
                "vmla.i16 q9, q5, q15\t\n"
 
                "vshr.s16 q7, q7, #6\t\n"
                "vshr.s16 q8, q8, #6\t\n"
                "vshr.s16 q9, q9, #6\t\n"
                "vmov.s16 q10, #0\t\n"
                "vmax.s16 q7, q7, q10\t\n"
                "vmax.s16 q8, q8, q10\t\n"
                "vmax.s16 q9, q9, q10\t\n"
                "vmov.u16 q10, #255\t\n"
                "vmin.u16 q7, q7, q10\t\n"
                "vmin.u16 q8, q8, q10\t\n"
                "vmin.u16 q9, q9, q10\t\n"
                "vmovn.s16 d2, q7\t\n"
                "vmovn.s16 d1, q8\t\n"
                "vmovn.s16 d0, q9\t\n"
 
                //Second RGBA
                "vshll.u8 q7, d9, #6\t\n"
                "vshll.u8 q8, d9, #6\t\n"
                "vshll.u8 q9, d9, #6\t\n"
                "vmla.i16 q7, q6, q11\t\n"
                "vmls.i16 q8, q5, q13\t\n"
                "vmls.i16 q8, q6, q14\t\n"
                "vmla.i16 q9, q5, q15\t\n"
                "vshr.s16 q7, q7, #6\t\n"
                "vshr.s16 q8, q8, #6\t\n"
                "vshr.s16 q9, q9, #6\t\n"
                "vmov.s16 q10, #0\t\n"
                "vmax.s16 q7, q7, q10\t\n"
                "vmax.s16 q8, q8, q10\t\n"
                "vmax.s16 q9, q9, q10\t\n"
                "vmov.u16 q10, #255\t\n"
                "vmin.u16 q7, q7, q10\t\n"
                "vmin.u16 q8, q8, q10\t\n"
                "vmin.u16 q9, q9, q10\t\n"
                "vmovn.s16 d6, q7\t\n"
                "vmovn.s16 d5, q8\t\n"
                "vmovn.s16 d4, q9\t\n"

/*     black-white picture           
                "vmov.u8 d0,d8\t\n"
                "vmov.u8 d1,d8\t\n"
                "vmov.u8 d2,d8\t\n"

                "vmov.u8 d4,d9\t\n"
                "vmov.u8 d5,d9\t\n"
                "vmov.u8 d6,d9\t\n"
*/ 
                "vtrn.8 d2,d6\t\n"
                "vtrn.16 d2,d6\t\n"
                "vtrn.32 d2,d6\t\n"
 
                "vtrn.8 d1,d5\t\n"
                "vtrn.16 d1,d5\t\n"
                "vtrn.32 d1,d5\t\n"
 
                "vtrn.8 d0,d4\t\n"
                "vtrn.16 d0,d4\t\n"
                "vtrn.32 d0,d4\t\n"

                "vst4.8 {d0-d3}, [%[dst]]!\t\n"
                "vst4.8 {d4-d7}, [%[dst]]!\t\n"
 
                "subs r4, r4, #1\t\n"
                "bne 1b\t\n"
                "2:\t\n"
                : [dst] "+r" (dst), [y] "+r" (y), [u] "+r" (u), [v] "+r" (v), [c] "+r" (c)
                :
                : "r4", "cc","memory", "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7","d8","d9","d10","d11","d12","d13","d14","d15","d16","d17","d18","d19","d20","d21","d22","d23","d24","d25","d26","d27","d28","d29","d30","d31"
                );
        count%=16;
#endif

        /* rest */
        int r, g, b;
        while (count > 1) {
            unsigned char _y = y[0];
            unsigned char _u = u[0];
            unsigned char _v = v[0];
            r = _y + ((179*(_v-128))>>7);
            g = _y - ((43*(_u-128) - 91*(_v-128))>>7);
            b = _y + ((227*(_u-128))>>7);
            r = r<0?0:r;r=r>255?255:r;
            g = g<0?0:g;g=g>255?255:g;
            b = b<0?0:b;b=b>255?255:b;
            dst[0] = b;
            dst[1] = g;
            dst[2] = r;
            dst[3] = 0xFF;
 
            y++;
            dst+=4;
            _y = y[0];
 
            r = _y + ((179*(_v-128))>>7);
            g = _y - ((43*(_u-128) - 91*(_v-128))>>7);
            b = _y + ((227*(_u-128))>>7);
            r = r<0?0:r;r=r>255?255:r;
            g = g<0?0:g;g=g>255?255:g;
            b = b<0?0:b;b=b>255?255:b;
 
            dst[0] = b;
            dst[1] = g;
            dst[2] = r;
            dst[3] = 0xFF;
            y++;
            u++;
            v++;
            dst+=4;
 
            count-=2;
        }

        if (count > 0) {
            unsigned char _y = y[0];
            unsigned char _u = u[0];
            unsigned char _v = v[0];
            r = _y + ((179*(_v-128))>>7);
            g = _y - ((43*(_u-128) - 91*(_v-128))>>7);
            b = _y + ((227*(_u-128))>>7);
            r = r<0?0:r;r=r>255?255:r;
            g = g<0?0:g;g=g>255?255:g;
            b = b<0?0:b;b=b>255?255:b;
            dst[0] = b;
            dst[1] = g;
            dst[2] = r;
            dst[3] = 0xFF;
        }     
    }
}
