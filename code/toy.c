#include "arena.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef enum {
    NODE_NUMBER,
    NODE_OPERATOR
} NodeKind;

typedef enum {
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV
} Operator;

typedef struct Node {
    struct Node* parent;
    struct Node* left;
    struct Node* right;
    NodeKind kind;
    Operator op;
    i32 val;
} Node;

int parse(arena* ar, char* str[200], u8* idxptr);
int eval(Node* node);
Node* parseEx(arena* ar, char* str[200], u8* idxptr);
Node* parseExpr(arena* ar, Node* asker, char* str[200], u8* idxptr);
Node* parseTerm(arena* ar, char* str[200], u8* idxptr);
Node* parseTermpr(arena* ar, Node* asker, char* str[200], u8* idxptr);
Node* parseFactor(arena* ar, char* str[200], u8* idxptr);

Operator getOperator(char c){
    switch (c){
        case '+':
            return OP_ADD;
        case '-':
            return OP_SUB;
        case '*':
            return OP_MUL;
        case '/':
            return OP_DIV;
        default:
            fprintf(stderr, "Unable to determine operator\n");
            exit(EXIT_FAILURE);
    }
}

int main(){
    printf("Toy Parsing Calculator: \n");
    printf("Enter in a mathematical expression using integers,  "
         "\'*\', \'/\', \'+\', \'-\', \'(\', or \')\' up to 200 chars in length \n"); 
    printf("Or exit with \"exit\"\n");
    //creating some space 
    arena* ar = arena_create(KiB(4), KiB(4));
    while(1) { 
        char str[200];
        printf("New expression: ");
        scanf("%200[^\n]", &str);
        if (strcmp(str, "exit") == 0) {
          printf("Exiting...\n");
          exit(EXIT_SUCCESS);
        }
        u8 idx = 0;
        u8* p = &idx;
        i32 res = parse(ar, &str, p);
        fprintf(stdout, "Evaluates to %d \n", res);
        arena_clear(ar);
    }   
}

//Buffer, pointer to index of buffer
int parse(arena* ar, char* str[200], u8* idxptr){
    Node* root = parseEx(ar, str, idxptr);
    return eval(root);
}

int eval(Node* node){
    if(node->kind == NODE_NUMBER){
        return node->val;
    } else {
        switch(node->op){
            case OP_ADD:
                return eval(node->left) + eval(node->right);
            case OP_SUB:
                return eval(node->left) - eval(node->right);
            case OP_MUL:
                return eval(node->left) * eval(node->right);
            case OP_DIV:
                return eval(node->left) / eval(node->right);
            default:
                fprintf(stderr, "Improper Node operation\n");
                exit(EXIT_FAILURE);
        }
    }
}

Node* parseEx(arena* ar, char* str[200], u8* idxptr){
    Node* left = parseTerm(ar, str, idxptr);
    Node* parent = parseExpr(ar, left, str, idxptr);
    return parent;
}

Node* parseExpr(arena* ar, Node* asker, char* str[200], u8* idxptr){
    char curr = *str[*idxptr];
    if(curr == '+' || curr == '-'){
        Node* parent = arena_push(ar, sizeof(Node));
        asker->parent = parent;

        parent->kind = NODE_OPERATOR;
        parent->op = getOperator(curr);
        idxptr++;
        parent->left = asker;
        parent->right = parseTerm(ar, str, idxptr);
        idxptr++;
        Node* gpar = parseExpr(ar, parent, str, idxptr);
        if(gpar == NULL) return parent;
        return gpar;
    } else {
        return NULL;
    }
}

Node* parseTerm(arena* ar, char* str[200], u8* idxptr){
    Node* left = parseFactor(ar, str, idxptr);
    Node* parent = parseTermpr(ar, left, str, idxptr);
    return parent;
}

Node* parseTermpr(arena* ar, Node* asker, char* str[200], u8* idxptr){
    char curr = *str[*idxptr];
    if(curr == '*' || curr == '/'){
        Node* parent = arena_push(ar, sizeof(Node));
        asker->parent = parent;

        parent->kind = NODE_OPERATOR;
        parent->op = getOperator(curr);
        *idxptr++;
        parent->left = asker;
        parent->right = parseFactor(ar, str, idxptr);
        *idxptr++;
        Node* gpar = parseTermpr(ar, parent, str, idxptr);
        if(gpar == NULL) return parent;
        return gpar;
    } else {
        return NULL;
    }
}

Node* parseFactor(arena* ar, char* str[200], u8* idxptr){
    char curr = *str[*idxptr];
    if(curr == '('){
        Node* expr = parseEx(ar, str, idxptr);
        if(curr == ')'){
            *idxptr++;
        }
        return expr;
    } else {
        i32 inp;
        i32 count;
        sscanf(&str + *idxptr, "%d%n", &inp, &count); 
        *idxptr += count; //skip over the consumed int characters
        Node* intnode = (Node*) arena_push(ar, sizeof(Node));
        intnode->kind = NODE_NUMBER;
        intnode->val = inp;
        intnode->left = NULL;
        intnode->right = NULL;
        return intnode;
    }
}


