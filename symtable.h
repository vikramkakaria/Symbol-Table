/* symtable.h */
/* Author: Vikram Kakaria */

#ifndef SYMTABLE_H
#define SYMTABLE_H

#include <stddef.h>

/* SymTable_T is essentially a pointer to a struct 
SymTable, allowing specific details to be hidden. 
In particular, SymTable_T is an object consisting 
of bindings, each of which are comprised of a key 
and a value. */
typedef struct SymTable *SymTable_T;

/* Returns a new SymTable object without bindings, or, if not enough 
memory is available, return NULL. */
SymTable_T SymTable_new(void);

/* Frees the memory that oSymTable occupies (if NULL, does nothing). */
void SymTable_free(SymTable_T oSymTable);

/* Returns number of bindings in oSymTable. */
size_t SymTable_getLength(SymTable_T oSymTable);

/* If there does not exist a binding in oSymTable whose key is pcKey, 
return 1 (for true) and add new binding with key pcKey and value 
pvValue. If not, or if there is not enough memory available, return 0 
(for false) and do not change oSymTable. */
int SymTable_put(SymTable_T oSymTable,
     const char *pcKey, const void *pvValue);

/* If there exists a binding in oSymTable whose key is pcKey, pvValue 
replaces binding's current value, and old value is returned, but, if 
such a binding does not exist, oSymTable does not change and NULL is 
returned. */
void *SymTable_replace(SymTable_T oSymTable,
     const char *pcKey, const void *pvValue);

/* Returns a truth value (0 or 1). Specifically, if oSymTable
does not contain a binding with key of pcKey, return 0 (for false), 
but if it does contain such a binding, return 1 (for true). */
int SymTable_contains(SymTable_T oSymTable, const char *pcKey);

/* If a binding whose key is pcKey exists in oSymTable, return 
the value of said binding, but, if not, return NULL. */
void *SymTable_get(SymTable_T oSymTable, const char *pcKey);

/* If there exists a binding in oSymTable whose key is pcKey, then
this binding is removed and its value is returned, but, if not, 
return NULL. */
void *SymTable_remove(SymTable_T oSymTable, const char *pcKey);

/* On each binding that is present in oSymTable, apply the *pfApply
function, having parameters pcKey, pvValue, and pvExtra. Here, 
pvExtra is an additional parameter. */
void SymTable_map(SymTable_T oSymTable,
     void (*pfApply)(const char *pcKey, void *pvValue, void *pvExtra),
     const void *pvExtra);

#endif
