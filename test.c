#include <stdio.h>
#include <openssl/sha.h>

int main(int ac, char** argv){

    char data[] = "data to hash";
    char hash[SHA512_DIGEST_LENGTH];
    SHA512(data, sizeof(data) - 1, hash);

    for(int i = 0; i < SHA512_DIGEST_LENGTH; i++)
        printf("%hhX", hash[i]);
    //printf("%s", hash);
    printf("\n");
    return 0;

}
