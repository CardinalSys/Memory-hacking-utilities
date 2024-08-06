#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int getArraySize(char array[4096]){
    int size = -1;
    for (int i = 0; i < 4096; i++) {
        if (i > 0 && array[i - 1] == ' ' && array[i] == ' ' || array[i] == '\0' || array[i] == '\n') {
            size = i;
            array[size] = '\0';
            break;
        }
    }
    return size;
}

void Comparation(char firstArray[], char secondArray[], int size, char *outArray){
    for (int i = 0; i < size; i++){
        if(firstArray[i] != secondArray[i]){
            outArray[i] = '?';
        }
        else{
            outArray[i] = firstArray[i];
        }
    }
    outArray[size] = '\0';
}

void InputSecondArray(char firstArray[] ,int firstArraySize){

    printf("Input second array\n");

    char secondArray[4096];

    if(fgets(secondArray, sizeof(secondArray), stdin) == NULL){
        printf("Invalid input\n");
        exit(1);
    }

    int secondArraySize = getArraySize(secondArray);

    if(firstArraySize != secondArraySize){
        printf("The two arrays must have the same length\n");
        exit(1);
    }

    char outArray[4096];

    Comparation(firstArray, secondArray, firstArraySize, outArray);

    printf("Output array: \n%s\n", outArray);

    InputSecondArray(outArray, firstArraySize);
}

int main(void){
    char firstArray[4096];
    printf("Input the first array\n");

    if (fgets(firstArray, sizeof(firstArray), stdin) == NULL) {
        printf("Invalid input\n");
        exit(1);
    }
    

    int firstArraySize = getArraySize(firstArray);


    InputSecondArray(firstArray, firstArraySize);

    return 0;
}
