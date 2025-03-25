/* symtablelist.c */
/* Author: Vikram Kakaria */

#include "symtable.h"
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>

/* A binding has a key and a value. In this linked list version, 
it can be seen as one binding linked to another within a linked 
list. */
struct Binding {
    /* Binding key */
    const char *key; 

    /* Binding value */
    const void *value;

    /* Next binding memory address */
    struct Binding *next;
};

/* A SymTable (indicating a symbol table) consists of bindings 
that are linked together. Just as in a linked list, the pointer
to the first binding is noted, along with the table's length. */
struct SymTable {
    /* Gives the memory address of the first binding. */
    struct Binding *firstBinding;
    
    /* Tells number of bindings in SymTable. */
    size_t length;
};

SymTable_T SymTable_new(void){
    SymTable_T oSymTable;

    /* Use memory allocation to create a SymTable_T of size of 
    the SymTable data structure */
    oSymTable = (SymTable_T)malloc(sizeof(struct SymTable));

    if(oSymTable==NULL){
        return NULL;
    }

    /* Update for new object */
    oSymTable->firstBinding = NULL;
    oSymTable->length=0;

    return oSymTable;
}

void SymTable_free(SymTable_T oSymTable){
    struct Binding *thisBinding;
    struct Binding *nextBinding;
    
    assert(oSymTable!=NULL);

    /* Create a looping condition: first set current binding
    equal to table's first binding. Until the current binding
    is not NULL, loop through and then set the current binding
    to the next binding. */
    for(thisBinding = oSymTable->firstBinding; thisBinding != NULL; 
    thisBinding = nextBinding){
        nextBinding = thisBinding->next;
        free((char*)thisBinding->key);
        free(thisBinding);
    }
    free(oSymTable);
}

size_t SymTable_getLength(SymTable_T oSymTable){
    assert(oSymTable!=NULL);
    return oSymTable->length;
}

int SymTable_put(SymTable_T oSymTable,
     const char *pcKey, const void *pvValue){
        struct Binding *thisBinding;
        struct Binding *newBinding;

        assert(oSymTable!=NULL);
        assert(pcKey!=NULL);
        
        /* Go through all return 0 cases, since nothing is changed. */
        for(thisBinding = oSymTable->firstBinding;
        thisBinding != NULL; thisBinding = thisBinding->next){
            if(strcmp(pcKey, thisBinding->key)==0){
                return 0;
            }                    
        }
        
        /* Define newBinding */
        newBinding = (struct Binding*)malloc(sizeof(struct Binding));

        /* If returns NULL, then this means insufficient memory 
        is available. */
        if(newBinding==NULL){
            return 0;
        }

        /* If these tests are passed, then attempt to assign a key 
        to newBinding. Memory allocation size of key is string 
        length + 1 (for terminating null character). */
        newBinding->key = (const char*)malloc(strlen(pcKey)+1);

        /* Not only need to return 0, but get rid of memory allocation 
        for newBinding, since it does not satisfy the requirements. */
        if(newBinding->key==NULL){
            free(newBinding);
            return 0;
        }

        /* Give instructions for return 1 case, since a binding
        needs to be added. The binding is added to the head 
        of the linked list. */

        /* For key portion, need to use strcpy instead of 
        assignment operator, since pointer reference will not 
        actually result in new key. Need (char*) cast since 
        newBinding->key is const char by definition. */
        strcpy((char*)newBinding->key, pcKey);

        newBinding->value = pvValue;
        newBinding->next = oSymTable->firstBinding;
        oSymTable->firstBinding=newBinding;
        oSymTable->length+=1;
        return 1;            
    }
        
void *SymTable_replace(SymTable_T oSymTable,
     const char *pcKey, const void *pvValue){
        struct Binding *binding;
        const void *oldValue;

        assert(oSymTable!=NULL);
        assert(pcKey!=NULL);

        for(binding = oSymTable->firstBinding; binding != NULL; 
        binding = binding->next){
            if(strcmp(pcKey, binding->key)==0){
                break;
            }        
        }

        /* Check binding upon exiting for loop. */
        if(binding==NULL){
            return NULL;
        }

        /* Store oldValue and replace current value of 
        binding with pvValue. */
        oldValue = binding->value;
        binding->value = pvValue;

        return (void*)oldValue;
    }

int SymTable_contains(SymTable_T oSymTable, const char *pcKey){
    struct Binding *binding;

    assert(oSymTable!=NULL);
    assert(pcKey!=NULL);

    for(binding = oSymTable->firstBinding; binding != NULL; 
    binding = binding->next){
        /* If any binding matches, return 1. */
        if(strcmp(pcKey, binding->key)==0){
            return 1;
        }
    }
    /* Return 0 otherwise. */
    return 0;
}

void *SymTable_get(SymTable_T oSymTable, const char *pcKey){
    struct Binding *binding;

    assert(oSymTable!=NULL);
    assert(pcKey!=NULL);
    
    for(binding = oSymTable->firstBinding; binding != NULL; 
    binding = binding->next){
        /* If any binding matches, return its value, using
        (void*) to cast. */
        if(strcmp(pcKey, binding->key)==0){
            return (void*)binding->value;
        }
    }
    /* Return NULL otherwise. */
    return NULL;  
}

void *SymTable_remove(SymTable_T oSymTable, const char *pcKey){
    /* Need to know binding before the current one, if it exists. */
    struct Binding *previousBinding;
    struct Binding *thisBinding;
    const void *removedValue;

    assert(oSymTable!=NULL);
    assert(pcKey!=NULL);
    
    /* Want to start at beginning, so previousBinding is NULL. */
    previousBinding=NULL;

    for(thisBinding = oSymTable->firstBinding; thisBinding != NULL; 
    thisBinding = thisBinding->next){
        if(strcmp(pcKey, thisBinding->key)==0){
            /* Case 1: previousBinding is NULL, so thisBinding 
            is first. */
            if(previousBinding==NULL){
                oSymTable->firstBinding=thisBinding->next;
            }
            /* Case 2: previousBinding is not NULL */
            else{
                previousBinding->next = thisBinding->next;
            }

            oSymTable->length-=1;

            /* Free key */
            free((char*)thisBinding->key);
            removedValue = thisBinding->value;

            /* Then, free binding and return removedValue */
            free(thisBinding);
            return (void*)removedValue;
        }
        previousBinding=thisBinding;
    }
    
    return NULL;
}

void SymTable_map(SymTable_T oSymTable,
     void (*pfApply)(const char *pcKey, void *pvValue, void *pvExtra),
     const void *pvExtra){
        struct Binding *binding;

        assert(oSymTable!=NULL);
        assert(pfApply!=NULL);
        
        for(binding = oSymTable->firstBinding; binding != NULL; 
        binding = binding->next){
            (*pfApply)((char*)binding->key,(void*)binding->value, 
            (void*)pvExtra);
        }
     }
