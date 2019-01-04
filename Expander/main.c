#include <stdio.h>
#include <stdlib.h>
#include <openssl/sha.h>

typedef unsigned int uint;

//#define DEBUG
//#define DEBUG_TREE
#define CONTINUOUS

#define HASH_INPUT
#define TREE_DEPTH 16
#define SEED_SIZE 64

#define DIGEST_LEN SHA512_DIGEST_LENGTH
#define DIGEST SHA512


uint square(uint base, uint exp) {
    uint out = 1;
    if(exp == 0) return out;
    for(uint i = 0; i < exp; i++)
        out *= base;
    return out;
}

uint offset_hash_at(uint pos) {
    return DIGEST_LEN * pos;
}

uint offset_source_at(uint pos) {
    return offset_hash_at(pos) / 2;
}

uint hash_count_at(uint level) {
    return square(2, level - 1);
}


int main(int ac, char** argv){

    const uint BUFF_SIZE = hash_count_at(TREE_DEPTH) * DIGEST_LEN;
    char *buffer = malloc(BUFF_SIZE);

    FILE *fp;
    fp = freopen(NULL, "rb", stdin);

    #ifdef CONTINUOUS
    while(1) {
    #endif

    uint read_len = fread(buffer, 1, SEED_SIZE, fp);
    if(read_len != SEED_SIZE) {
        fprintf(stderr, "STDIN error.\n");
        free(buffer);
	return 1;
    }
    

    #ifdef DEBUG
    printf("Input seed read %i bytes: ", read_len);
    fwrite(buffer, 1, SEED_SIZE, stdout);
    printf("\n");
    #endif
    
    
    #ifdef HASH_INPUT
        DIGEST(buffer, SEED_SIZE, buffer);
    #endif
    
    
    for(uint level = 2; level <= TREE_DEPTH; level++) {
        for(uint hash_index = hash_count_at(level) - 1; //Count to index
         hash_index != -1; hash_index--) {
            
            #ifdef DEBUG_TREE
            printf("\n");
            printf("Level: %i", level);
            printf("\tHash ind: %i", hash_index);
            printf("\tHash pos: %i", offset_hash_at(hash_index));
            printf("\tHash src pos: %i", offset_source_at(hash_index));
            printf("\n");
            for(uint i = 0; i < BUFF_SIZE; i++) {
                if(i == 0);
                else if(i % DIGEST_LEN == 0) printf("\n\n");
                else if(i % 16 == 0) printf("\n");
                else if(i % 2 == 0) printf(" ");
                printf("%02X", buffer[i] & 0xFF);
            }
            printf("\n");
            #endif

            DIGEST(
                buffer + offset_source_at(hash_index),
                DIGEST_LEN / 2,
                buffer + offset_hash_at(hash_index));
        }
    }


    #ifdef DEBUG
    printf("\nFinal Out\n");
    for(uint i = 0; i < BUFF_SIZE; i++) {
        if(i == 0);
        else if(i % DIGEST_LEN == 0) printf("\n\n");
        else if(i % 16 == 0) printf("\n");
        else if(i % 2 == 0) printf(" ");
        printf("%02X", buffer[i] & 0xFF);
    }
    
    #else

    fwrite(buffer, 1, BUFF_SIZE, stdout);

    #endif


    #ifdef CONTINUOUS
    } //End while(true)
    #endif


    free(buffer);
    return 0;

}

