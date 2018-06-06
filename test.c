#include <stdio.h>
#include <openssl/sha.h>

int main(int ac, char** argv){

    char data[] = "data to hash";
    char hash[SHA512_DIGEST_LENGTH];
    SHA512(data, sizeof(data) - 1, hash);

    for(int i = 0; i < SHA512_DIGEST_LENGTH; i++) {
        if(i == 0);
        else if(i % SHA512_DIGEST_LENGTH == 0) printf("\n\n"); //TODO check
        else if(i % 16 == 0) printf("\n");
        else if(i % 2 == 0) printf(" ");
        printf("%02X", hash[i] & 0xFF);
    }
    //printf("%s", hash);
    printf("\n");
    return 0;

}
