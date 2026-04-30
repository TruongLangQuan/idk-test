#ifndef __VECX_H
#define __VECX_H

#include "gbConfig.h"
enum {
	VECTREX_MHZ		= 1500000, /* speed of the vectrex being emulated - full speed */
	VECTREX_COLORS  = 128,     /* number of possible colors ... grayscale */

	ALG_MAX_X		= 33000,	
	ALG_MAX_Y		= 41000
};

 typedef struct vector_type
 {
  unsigned short int x0, y0; //start coordinate
  unsigned short int x1, y1; //end coordinate
 } vector_t;

#ifdef use_lib_snd_regs_8bits
 extern unsigned char snd_regs[16];
#else
 extern unsigned int snd_regs[16];
#endif
#ifdef use_lib_alg_jch_8bits
 extern unsigned char alg_jch0;
 extern unsigned char alg_jch1;
 extern unsigned char alg_jch2;
 extern unsigned char alg_jch3;
#else
 extern unsigned int alg_jch0;
 extern unsigned int alg_jch1;
 extern unsigned int alg_jch2;
 extern unsigned int alg_jch3;
#endif 

extern int vector_draw_cnt;
extern int vector_erse_cnt;

extern vector_t *vectors_draw;
extern vector_t *vectors_erse;

void vecx_reset (void);
void vecx_emu (int cycles, int ahead);

int GetSizeBytes_vectors_set(void);
int GetSizeBytes_vector_hash(void);
int GetVECTOR_HASH(void);
int GetVECTOR_CNT(void);
int GetVECTREX_PDECAY(void);


#ifdef use_lib_optimice_readwrite8
 unsigned char read8 (unsigned short int address);
 void write8 (unsigned short int address, unsigned char data);
#else
 unsigned char read8 (unsigned address);
 void write8 (unsigned address, unsigned char data);
#endif 


#ifndef use_lib_optimice_call_via_sstep0
 void via_sstep0(void);
#endif
#ifndef use_lib_optimice_call_alg_sstep
 void alg_sstep(void);
#endif 
//void via_sstep1(void);

void snd_update(void);
void alg_update (void);

#endif
