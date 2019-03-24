#include <r.h>
#include <stdio.h>
#include <math.h>

int main(void)
{
    xorshift_state_initalize();

    const size_t N = 1<<4;
    printf("vec_t unit_vectors(size_t i) { switch(i %% %zu) { ", 2*N*N);
    for(size_t i = 0; i < N; i++) {
        for(size_t j = 0; j < (N*2); j++) {
            float x = sin(M_PI*(i+1)/N)*cos(M_PI*j/N),
                  y = sin(M_PI*(i+1)/N)*sin(M_PI*j/N),
                  z = cos(M_PI*(i+1)/N);
            printf("case %zu: return vec(%f, %f, %f); \n", 2*N*i + j, x, y, z);
        }
    }

    printf("} return vec(0,0,0);}\n");
    return 0;
}
