#include <stdio.h>

int main(int argc, char** argv) {
    printf("testSystemCall program: fopen at ./t/fs1/file.txt\n");
    FILE* fp = fopen("./file.txt", "w+");
    fclose(fp);
    return 0;
}