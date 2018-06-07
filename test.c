#include <stdio.h>
#include <openssl/sha.h>

typedef unsigned int uint;

#define TREE_DEPTH 3 
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

    //printf("Digest size: %i\n", SHA512_DIGEST_LENGTH);
    //printf("Buffer size: %i\n", BUFF_SIZE);

    char data[] = "data to hash";
    char hash[BUFF_SIZE];
    DIGEST(data, sizeof(data) - 1, hash);

    for(uint i = 0; i < BUFF_SIZE; i++) {
        if(i == 0);
        else if(i % DIGEST_LEN == 0) printf("\n\n"); //TODO check
        else if(i % 16 == 0) printf("\n");
        else if(i % 2 == 0) printf(" ");
        printf("%02X", hash[i] & 0xFF);
    }
    //printf("%s", hash);
    printf("\n");
    return 0;

}

