#include<stdio.h>
#include <inttypes.h>
#include <string.h>
#include "aes.h"

// 4x4 matrix in GF(256)
unsigned char M[] = {
	0x02, 0x01, 0x01, 0x03,
	0x03, 0x02, 0x01, 0x01, 
	0x01, 0x03, 0x02, 0x01,
	0x01, 0x01, 0x03, 0x02
};

/* =========================================
	Function definitions
   ========================================= */

void PrintState(unsigned char * state){
	int i;
	
	for(i=0; i<16; i++){
		printf("%x \t",state[i]);
	}
	printf("\n\n");
}

/* =========================================
	AES Encryption / Decryption subfunctions
   ========================================= */
	
void AddRoundKey(unsigned char * roundkey, unsigned char * state){
	uint8_t i;
		
	for(i=0; i<16; i++){
		state[i] ^= roundkey[i];
	}
}

void SubBytes(unsigned char * state, unsigned char * S){
	
	uint8_t i;
	uint8_t a, b, idx;					//entries
	
	for(i=0; i<16; i++){
		
		// mask first and last four bits for a,b entries
		a = (uint8_t)(state[i] & 0xf0);
		a = a >> 4;
		
		b = (uint8_t)(state[i] & 0x0f);
		//printf("state=%x \t a=%u b=%u\n",state[i], a, b);
		//printf("a=%u b=%u\n\n", a, b);
		
		// S-box lookup
		idx = ((16*a)+b);
		state[i] = S[idx];
		//printf("idx=%u\t S[idx]=%x \t state[i]=%x\n", idx, S[idx], state[i]);
	}
}

void ShiftRows(unsigned char * state){
	
	uint8_t i, j;
	unsigned char tmp[16];
	
	for(j=0;j<4;j++){
		for(i=0;i<4;i++){
			// shift left by i bytes 
			if(j-i<0){
				tmp[i+4*(4+(j-i))] = state[i+4*j];
			}
			else{
				tmp[i+4*(j-i)] = state[i+4*j];
			}
		}
	}
	// copy shifted array to AES state vector	
	for(j=0;j<4;j++){
		for(i=0;i<4;i++){
			state[i+4*j] = tmp[i+4*j];
		}
	}
	
}

void InvShiftRows(unsigned char * state){
	
	int i, j;
	unsigned char tmp[16];
	
	// circular shift:   y = (x << bitshift) | (x >> (8 - bitshift));
	for(j=0;j<4;j++){
		for(i=0;i<4;i++){
			// shift right by i bytes 
			if(3-(j+i)<0){
				tmp[i+4*(j+i-4)] = state[i+4*j];
			}
			else{
				tmp[i+4*(j+i)] = state[i+4*j];
			}
		}
	}
	// copy shifted array to AES state vector
	for(j=0;j<4;j++){
		for(i=0;i<4;i++){
			state[i+4*j] = tmp[i+4*j];
		}
	}
}

void MixColumns(unsigned char * M, unsigned char * state){
	
	unsigned char state_out[16] = { 0x0 };
	
	int i,j,k;
	unsigned char mult; 
	
	for(i=0; i<4; i++){
		for(j=0; j<4; j++){
			
			// Calculate i,j state_out vector
			for(k=0; k<4; k++){
				mult = M[i+k*4];
				
				if(mult == 0x01){
					state_out[i+j*4] ^= state[k+j*4];
						
				} else if(mult == 0x02){
					state_out[i+j*4] ^= MulBy02(&state[k+j*4]);
					
				} else {
					state_out[i+j*4] ^= ( MulBy02(&state[k+j*4]) ) ^ state[k+j*4];
				}
			}
		}
	}
	// copy temporary array to AES state vector
	for(j=0;j<4;j++){
		for(i=0;i<4;i++){
			state[i+4*j] = state_out[i+4*j];
		}
	}
}


unsigned char MulBy02(unsigned char * ptr){
	unsigned char val;
	
	//0x02*x <=> ( (x<<1) XOR 0x1B ) iff MSB of x == 1	
	if( (0x80 & *ptr) == 0x80 ){
			val = ((*ptr << 1) ^ 0x1B);
		} else {
			val = *ptr << 1;
		}
	
	return val;
}

/* ==================================
	AES encryption
   ================================== */
void AES_ENC(unsigned char * plaintext, unsigned char * roundkey, unsigned char * state, unsigned char * S, int rounds){
		
	//initialize state vector
	int i;
	for(i=0; i<16; i++)
		state[i] |= plaintext[i];
	
	// printf("============\npre-whitening\n"); PrintState(state);
	
	//round 0:
	// printf("round-key\n"); PrintState(roundkey);
	AddRoundKey(roundkey, state);
	// printf("round 0\n"); PrintState(state);
	
	//round 1 to n-1
	for(i=1; i<=rounds-1; i++){
		// printf("round %d\n", i);
		SubBytes(state, S);
		// PrintState(state);
	
	
		ShiftRows(state);
		// PrintState(state);
	
		MixColumns(M, state);
		// PrintState(state);
			
		AddRoundKey(&roundkey[i*16], state);
		// PrintState(state);
	}
	// printf("round %d\n", i);
	
	//last round
	SubBytes(state, S);
	// PrintState(state);
	
	ShiftRows(state);
	// PrintState(state);
	
	// printf("round key \n"); PrintState(&roundkey[i*16]);
	AddRoundKey(&roundkey[i*16], state);
	// PrintState(state);
}




