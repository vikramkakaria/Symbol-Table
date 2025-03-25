/* symtablehash.c */
/* Author: Vikram Kakaria */

#include "symtable.h"
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>

/* Gives each potential value for bucket counts, progressing
from  primeBuckCounts[0] to primeBuckCounts[1] to the last
index of the array. These can be seen as a form of boundaries, 
which are handled by the SymTable_expand(SymTable_T oSymTable) 
function. */
static const size_t primeBuckCounts[] = {
    509, 1021, 2039, 4093, 8191, 16381, 32749, 65521
    };

/* A binding has a key and a value. It can be seen as linking
to another binding. */
struct Binding {
    /* Binding key */
    const char *key; 

    /* Binding value */
    const void *value;

    /* Next binding memory address */
    struct Binding *next;
};

/* A SymTable (indicating a symbol table) consists of bindings 
that are linked together. In a hash table representation, there 
are buckets present. The SymTable, in particular, is pointing
to the array of buckets. The number of bindings, as well as 
the number of buckets, are noted. */
struct SymTable {
    /* An array of buckets, where each bucket is functionally
    similar to a linked list. */
    struct Binding **buckets;
    
    /* Tells number of bindings present. */
    size_t length;

    /* Tells number of buckets present. */
    size_t numBuckets;
};

/* Return a hash code for pcKey that is between 0 and uBucketCount-1,
   inclusive. */
static size_t SymTable_hash(const char *pcKey, size_t uBucketCount)
{
   const size_t HASH_MULTIPLIER = 65599;
   size_t u;
   size_t uHash = 0;

   assert(pcKey != NULL);

   for (u = 0; pcKey[u] != '\0'; u++)
      uHash = uHash * HASH_MULTIPLIER + (size_t)pcKey[u];

   return uHash % uBucketCount;
}

SymTable_T SymTable_new(void){
    SymTable_T oSymTable;

    /* Use memory allocation to create a SymTable_T of size of 
    the SymTable data structure */
    oSymTable = (SymTable_T)malloc(sizeof(struct SymTable));

    /* Not enough memory */
    if(oSymTable==NULL){
        return NULL;
    }

    oSymTable->length=0;
    oSymTable->numBuckets=primeBuckCounts[0];

    /* Calloc does NULL initialization for pointers. */
    oSymTable->buckets=(struct Binding**)calloc(
        oSymTable->numBuckets, sizeof(struct Binding*));

    /* Check if there is insufficient memory for bucket array */
    if(oSymTable->buckets==NULL){
        free(oSymTable);
        return NULL;
    }

    return oSymTable;
}

void SymTable_free(SymTable_T oSymTable){
    size_t bucketNumber;
    struct Binding *thisBinding;
    struct Binding *nextBinding;
    
    assert(oSymTable!=NULL);

    /* Create a looping condition: first set current binding
    equal to table's first binding. Until the current binding
    is not NULL, loop through and then set the current binding
    to the next binding. */
    for(bucketNumber=0; bucketNumber<(oSymTable->numBuckets); 
    bucketNumber++){
        for(thisBinding = (oSymTable->buckets)[bucketNumber];
        thisBinding != NULL; thisBinding = nextBinding){
            nextBinding = thisBinding->next;
            free((char*)thisBinding->key);
            free(thisBinding);
        }
    }
    free(oSymTable->buckets);
    free(oSymTable);
}

size_t SymTable_getLength(SymTable_T oSymTable){
    assert(oSymTable!=NULL);
    return oSymTable->length;
}

/* This function seeks to expand oSymTable by increasing the number 
of buckets present. If not enough memory is available, then the table 
is unchanged. If, however, enough memory is available for expansion, 
then it is expanded. */
static void SymTable_expand(SymTable_T oSymTable){
    size_t newNumBuckets;
    size_t bucket;
    size_t newBucket;
    struct Binding **newBuckets;
    struct Binding *thisBinding;
    struct Binding *nextBinding;
    int index=0;
    
    /* Find newNumBuckets by going through primeBuckCounts array. Find 
    index of current bucket count and add 1 to it. */
    while(primeBuckCounts[index]!=(oSymTable->numBuckets)){
        index+=1;
    }
    newNumBuckets=primeBuckCounts[index+1];

    /* Make newBuckets, allocating memory for each one */
    newBuckets = (struct Binding**)calloc(newNumBuckets, 
    sizeof(struct Binding*));

    /* No expansion, so exit function. */
    if(newBuckets==NULL){
        return;
    }

    /* Hashing, iterating through bucket array for all buckets */
    for(bucket=0; bucket<(oSymTable->numBuckets); bucket++){
        /* Iterates through all bindings in each bucket and assigns 
        to new bucket. */
        for(thisBinding = (oSymTable->buckets)[bucket]; 
        thisBinding != NULL; thisBinding = nextBinding){
            /* Cannot do thisBinding=thisBinding->next due to 
            overwriting. */
            nextBinding=thisBinding->next;
            newBucket = SymTable_hash(thisBinding->key, newNumBuckets);
            /* First binding in the bucket */
            thisBinding->next=newBuckets[newBucket];
            /* Making thisBinding first */
            newBuckets[newBucket]=thisBinding;
        }
    }
        
    /* Change existing buckets array to new bucket array and get rid of
    original buckets array by freeing it from memory. Then return 1 to 
    indicate that expansion is possible. */
    free(oSymTable->buckets);
    oSymTable->buckets = newBuckets;
    oSymTable->numBuckets = newNumBuckets;
}

int SymTable_put(SymTable_T oSymTable,
     const char *pcKey, const void *pvValue){
        struct Binding *thisBinding;
        size_t bucket;
        size_t maximum;

        assert(oSymTable!=NULL);
        assert(pcKey!=NULL);

        /* Expand hash table if number of bindings is greater
        than number of buckets */
        if(((oSymTable->length)>(oSymTable->numBuckets))){
            /* Check to make sure that maximum has not been reached */
            maximum = sizeof(primeBuckCounts)/sizeof(size_t);
            if((oSymTable->numBuckets)<primeBuckCounts[maximum-1]){
                (void)SymTable_expand(oSymTable);
            }
        }

        bucket = SymTable_hash(pcKey, oSymTable->numBuckets);

        /* Non-expansion */
        for(thisBinding = oSymTable->buckets[bucket];
        thisBinding != NULL; thisBinding = thisBinding->next){
            if(strcmp(pcKey, thisBinding->key)==0){
                return 0;
            }                    
        }

        /* Define newBinding */
        thisBinding = (struct Binding*)malloc(sizeof(struct Binding));

        /* If returns NULL, then this means insufficient memory 
        is available. Have to check here, since this is an issue
        of insufficient memory if NULL. */
        if(thisBinding==NULL){
            return 0;
        }

        /* If these tests are passed, then attempt to assign a key 
        to newBinding. Memory allocation size of key is string 
        length + 1 (for terminating null character). */
        thisBinding->key = (const char*)malloc(strlen(pcKey)+1);

        /* Not only need to return 0, but get rid of memory allocation 
        for newBinding, since it does not satisfy the requirements. */
        if(thisBinding->key==NULL){
            free(thisBinding);
            return 0;
        }

        /* Give instructions for return 1 case, since a binding
        needs to be added. */

        /* For key portion, need to use strcpy instead of 
        assignment operator, since pointer reference will not 
        actually result in new key. Need (char*) cast since 
        newBinding->key is const char by definition. */
        strcpy((char*)thisBinding->key, pcKey);


        thisBinding->value = pvValue;
        thisBinding->next = (oSymTable->buckets)[bucket];
        (oSymTable->buckets)[bucket]=thisBinding;
        oSymTable->length+=1;
        return 1;            
    }

        
void *SymTable_replace(SymTable_T oSymTable,
     const char *pcKey, const void *pvValue){
        struct Binding *binding;
        const void *oldValue;
        size_t bucket;

        assert(oSymTable!=NULL);
        assert(pcKey!=NULL);

        bucket = SymTable_hash(pcKey, oSymTable->numBuckets);

        for(binding = oSymTable->buckets[bucket]; 
        binding != NULL; binding = binding->next){
            if(strcmp(pcKey, binding->key)==0){
                break;
            }        
        }

        if(binding==NULL){
            return NULL;
        }

        oldValue = binding->value;
        binding->value = pvValue;

        return (void*)oldValue;
    }

int SymTable_contains(SymTable_T oSymTable, const char *pcKey){
    struct Binding *binding;
    size_t bucket;

    assert(oSymTable!=NULL);
    assert(pcKey!=NULL);

    bucket = SymTable_hash(pcKey, oSymTable->numBuckets);

    for(binding = (oSymTable->buckets)[bucket]; binding != NULL; 
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
    size_t bucket;

    assert(oSymTable!=NULL);
    assert(pcKey!=NULL);

    bucket = SymTable_hash(pcKey, oSymTable->numBuckets);

    for(binding = (oSymTable->buckets)[bucket]; 
    binding != NULL; binding = binding->next){
        if(strcmp(pcKey, binding->key)==0){
            return (void*)binding->value;
        }
    }
    return NULL;
}

void *SymTable_remove(SymTable_T oSymTable, const char *pcKey){
    /* Need to know binding before the current one, if it exists. */
    struct Binding *previousBinding;
    struct Binding *thisBinding;
    const void *removedValue;
    size_t bucket;

    assert(oSymTable!=NULL);
    assert(pcKey!=NULL);

    bucket = SymTable_hash(pcKey, oSymTable->numBuckets);

    /* Want to start at beginning, so previousBinding is NULL. */
    previousBinding=NULL;

    for(thisBinding = (oSymTable->buckets)[bucket]; 
    thisBinding != NULL; thisBinding = thisBinding->next){
        if(strcmp(pcKey, thisBinding->key)==0){
            /* Case 1: previousBinding is NULL, so thisBinding 
            is first. */
            if(previousBinding==NULL){
                oSymTable->buckets[bucket]=thisBinding->next;
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
        size_t bucketNumber;

        assert(oSymTable!=NULL);
        assert(pfApply!=NULL);
        
        for(bucketNumber=0; bucketNumber<(oSymTable->numBuckets); 
        bucketNumber++){
            for(binding = (oSymTable->buckets)[bucketNumber]; 
            binding != NULL; binding = binding->next){
                (*pfApply)((char*)binding->key,(void*)binding->value, 
                (void*)pvExtra);
            }
        }
     }
