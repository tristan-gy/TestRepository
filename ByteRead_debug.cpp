/* 
 * File:   ByteRead.cpp
 * Author: Tristan Gay
 * Description: 
 *  Reads files by individual bytes, and
 *  prints out the bytes in a readable format.
 * 
 *  Can read 1, 2, or 4 bytes in either
 *  little-endian or big-endian,
 *  and return the unsigned/signed values.
 */

/* INCLUDES */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <iostream>

/* CONSTANTS */
#define LITTLE 0
#define BIG 1
#define SIGNED 0
#define UNSIGNED 1
#define LINESIZE 16 //Number of bytes to read per line (binaryDumpAsASCII)

using namespace std;

/**
 * printLine - prints up to LINESIZE bytes and their
 * respective values in ASCII.
 * 
 * @param buffer    the array of bytes to read from
 * @param size      denotes the size of the buffer
 */
void printLine(uint8_t* buffer, int size){
    /* Prints our hex values */
    for(int i = 0; i < size; i++){
        printf("%02X ", buffer[i]);
    }
    
    /* If we have a line that contained no more bytes, print whitespace */
    if(size != LINESIZE){
        for(int i = size; i < LINESIZE; i++){
            printf("   ");
        }
    }
    printf("\t");
    
     /* Prints our ASCII values */
    for(int i = 0; i < size; i++){
        if(buffer[i] < 32 || buffer[i] > 126){
            printf(" ");
        } else {
            printf("%c", buffer[i]);
        }
    }
    printf("\n");
}

/**
 * getNumeric - can read up to 4 bytes of a specified endianness and return
 * it either signed or unsigned. 
 * 
 * @param f         the file pointer to read from
 * @param offset    how far from file pointer we should read
 * @param size      number of bytes to read (1, 2, or 4)
 * @param endian    read the file in LITTLE or BIG endian
 */
unsigned long getNumeric (FILE *f, long offset, int size, char type, char endian){
    unsigned long ret = 0;
    uint8_t buffer[size] = {}; //fills our array with zeroes
    int sizeInBits = size * 8;
    
    if(endian == LITTLE){
        fseek(f, (offset*-1), SEEK_END);
        long int temp = -1; //-1 to account for fread moving our position indicator forward
        for(int i = 0; i < size; i++){
            fseek(f, temp, SEEK_CUR); //back up one to allow fread to read forwards
            fread(&buffer[i], 1, 1, f);
            fseek(f, temp, SEEK_CUR); //back up again to move it back to where it was
        }  
    } else if(endian == BIG){
        fseek(f, offset, SEEK_SET);
        for(int i = 0; i < size; i++){
            fread(&buffer[i], 1, 1, f);
        }
    }
    
    int shiftBy = sizeInBits-8; //size is in bytes, but we shift in bits
    for(int i = 0; i < size; i++){
        ret |= ((unsigned long)buffer[i] << shiftBy);
        shiftBy = shiftBy - 8;
    }
    
    unsigned long mask = 0xFFFFFFFF;
    int sizeOfULong = 32;
    if(type == SIGNED){
        if(ret >> (sizeInBits-1) == 1){ //if our sign bit is on
            mask = mask >> (sizeOfULong-sizeInBits); //shift our mask to fit the size of our number
            ret = mask - ret; //now perform basic two's complement operations
            ret = ~ret;
        }
    }
    return ret;
}

/**
 * binaryDumpAsASCII -- print our file LINESIZE bytes at a time,
 * and show us their ASCII character values.
 * 
 * @param fin       the file pointer to read from
 */
void binaryDumpAsASCII(FILE *fin){
    int linePos = 0;
    uint8_t buffer[LINESIZE];
    
    /* fread returns the number of elements successfully read */
    fseek(fin, 0, SEEK_SET);
    while((fread(&buffer[linePos], 1, 1, fin) == 1)){ //while we are still reading values
        if(linePos == LINESIZE-1){ //if we've reached the end of our buffer
            printLine(buffer, LINESIZE);
            linePos = 0;
        } else {
            linePos++;
        }
    }
    /* If the line is not LINESIZE long */
    if(fread(&buffer[linePos], 1, 1, fin) != 1){
        printLine(buffer, linePos); //Print the remaining characters.
    }
}

int main() {
    FILE *fin;
    FILE *fin2;

    fin = fopen("file.bin", "r");
    printf("%ld\n", getNumeric(fin, 0, 4, SIGNED, LITTLE));
    printf("%ld\n", getNumeric(fin, 0, 4, SIGNED, BIG));
    printf("%ld\n", getNumeric(fin, 1, 2, SIGNED, LITTLE));
    printf("%ld\n", getNumeric(fin, 1, 2, SIGNED, BIG));
    fclose(fin);
    
    fin2 = fopen("icon.png", "r");
    binaryDumpAsASCII(fin2);
    fclose(fin2);
    
    return 0;
}