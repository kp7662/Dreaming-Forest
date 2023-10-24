/*--------------------------------------------------------------------*/
/* symtable.h.c                                                       */
/* Author: Kok Wei Pua                                                */
/*--------------------------------------------------------------------*/

#ifndef SYMTABLE_INCLUDED
#define SYMTABLE_INCLUDED
#include <stddef.h>

/*--------------------------------------------------------------------*/
/* A Symtable_T is an unordered collection of bindings that consists of
a key and a value. */
 
typedef struct SymTable *SymTable_T;

/*--------------------------------------------------------------------*/

/* Return a new SymTable object that contains no bindings, or NULL if
insufficient memory is available. */

SymTable_T SymTable_new(void);

/*--------------------------------------------------------------------*/

/* Free all memory occupied by oSymTable. */

void SymTable_free(SymTable_T oSymTable);

/*--------------------------------------------------------------------*/

/* Return the number of bindings in oSymTable. */

size_t SymTable_getLength(SymTable_T oSymTable);

/*--------------------------------------------------------------------*/

/* Add a new binding to oSymTable consisting of key pcKey and value 
pvValue and return 1 (TRUE). Otherwise, return 0 (FALSE). */

int SymTable_put(SymTable_T oSymTable, const char *pcKey, 
                 const void *pvValue);

/*--------------------------------------------------------------------*/

/* Replace existing binding with key pcKey in oSymTable with pvValue
and return the old value. Otherwise, return NULL. */

void *SymTable_replace(SymTable_T oSymTable, const char *pcKey, 
                       const void *pvValue);

/*--------------------------------------------------------------------*/

/* Return 1 (TRUE) if oSymTable contains a binding whose key is pcKey.
Otherwise, return 0 (FALSE). */

int SymTable_contains(SymTable_T oSymTable, const char *pcKey);

/*--------------------------------------------------------------------*/

/* Return the value of the binding within oSymTable whose key is pcKey,
or NULL if no such bindign exists. */

void *SymTable_get(SymTable_T oSymTable, const char *pcKey);

/*--------------------------------------------------------------------*/

/* Remove existing binding with key pcKey from oSymTable and return the 
binding's value. Otherwise, return NULL. */

void *SymTable_remove(SymTable_T oSymTable, const char *pcKey);

/*--------------------------------------------------------------------*/

/* Apply function *pfApply to each binding in oSymTable, passing 
pvExtra as an extra parameter. */

void SymTable_map(SymTable_T oSymTable,
    void (*pfApply)(const char *pcKey, void *pvValue, void *pvExtra),
    const void *pvExtra);

/*--------------------------------------------------------------------*/

#endif