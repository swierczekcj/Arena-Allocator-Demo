#include "arena.h"
#include <stdlib.h>
#include <stdio.h>




int main(){
    printf("Toy Recursive Descent Parser: \n");
    printf("Enter in a mathematical expression using integers,  "
         "\'*\', \'+\', \'-\', \'(\', or \')\' up to 200 chars in length \n"); 
    printf("Or exit with \"exit\"");
    while(1){
        char str[200];
        printf("New expression: ");
        scanf("%200[^\n]", &str);
        parse(&str);
    }   
}