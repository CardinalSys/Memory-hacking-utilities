#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void GetArray(char **buffer, int bufferSize){
    *buffer = (char *)malloc(bufferSize * sizeof(char));
    if (*buffer == NULL) {
        printf("Memory allocation failed.\n");
        exit(1);
    }

    if (fgets(*buffer, bufferSize, stdin) != NULL) {
        size_t len = strlen(*buffer);
        if (len > 0 && (*buffer)[len - 1] == '\n') {
            (*buffer)[len - 1] = '\0';
        }
    } else {
        printf("Error reading input\n");
        exit(1);
    }
}

void AoBComparation(char* firstArray, int bufferSize){
    char* secondArray = NULL;

    printf("Enter second array: ");
    GetArray(&secondArray, bufferSize);

    printf("First: %s\n", firstArray);
    printf("Second: %s\n", secondArray);

    char* resultArray = (char *)malloc(bufferSize * sizeof(char));
    if (resultArray == NULL) {
        printf("Memory allocation failed.\n");
        free(firstArray);
        free(secondArray);
        exit(1);
    }


    for(int i = 0; i < bufferSize; i++){
        if(firstArray[i] == secondArray[i]){
            resultArray[i] = firstArray[i];
        }else{
            resultArray[i] = '?';
        }
    }

    resultArray[bufferSize - 1] = '\0';

    printf("Result: %s\n", resultArray);

    free(firstArray);
    free(secondArray);

    AoBComparation(resultArray, bufferSize);

}

int main(void){
    int bufferSize;
    
    printf("Enter the array size: ");
    if (scanf("%d", &bufferSize) != 1 || bufferSize <= 0) {
        printf("Invalid array size.\n");
        return 1;
    }

    bufferSize = (bufferSize * 3) + 1;


    while (getchar() != '\n');

    char* firstArray = NULL;

    printf("Enter first array: ");
    GetArray(&firstArray, bufferSize);

    AoBComparation(firstArray, bufferSize);

    return 0;
}
