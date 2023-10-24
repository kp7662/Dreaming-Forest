/*--------------------------------------------------------------------*/
/* symtablehash.c                                                     */
/* Author: Kok Wei Pua                                                */
/*--------------------------------------------------------------------*/

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "symtable.h"

/*--------------------------------------------------------------------*/

/* The bucket counts array. */
static const size_t auBucketCounts[] = {509, 1021, 2039, 4093, 8191, 
                                        16381, 32746, 65521}; 

/* The number of bucket counts. */
static const size_t numBucketCounts =
sizeof(auBucketCounts)/sizeof(auBucketCounts[0]);

/*--------------------------------------------------------------------*/

/* Each key and value is stored in a SymTableNode.  SymTableNodes are 
   linked to form a list.  */

struct SymTableNode
{
   /* The key. */
   const char *pcKey;

   /* The value. */
   const void *pvValue;

   /* The address of the next SymtableNode. */
   struct SymTableNode *psNextNode;
};

/*--------------------------------------------------------------------*/

/* A SymTable is a "dummy" node that points to the first SymtableNode, 
store the current bucket counts, and number of bindings in the SymTable. */

struct SymTable
{
   /* The pointer to the bucket array of pointers to nodes. */
   struct SymTableNode **psBuckets;

   /* The index of the current bucket count. */
   size_t index;

   /* The number of bindings. */
   size_t num;
};

/*--------------------------------------------------------------------*/

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

/*--------------------------------------------------------------------*/

SymTable_T SymTable_new(void)
{
   SymTable_T oSymTable;

   oSymTable = (SymTable_T)malloc(sizeof(struct SymTable));
   if (oSymTable == NULL)
      return NULL;

   oSymTable->psBuckets = (struct SymTableNode**)
      calloc(auBucketCounts[0], sizeof(struct SymTableNode*));
   if (oSymTable->psBuckets == NULL) 
   {
    free(oSymTable);
    return NULL;
   }

   oSymTable->num = 0;
   oSymTable->index = 0;
   return oSymTable;
}

/*--------------------------------------------------------------------*/

void SymTable_free(SymTable_T oSymTable)
{
   struct SymTableNode *psCurrentNode;
   struct SymTableNode *psNextNode;
   size_t i;

   assert(oSymTable != NULL);

   for(i = 0; i < auBucketCounts[oSymTable->index]; i++)
   {
    /* Iterate through the linked list to free all nodes in bucket i. */
    for (psCurrentNode = oSymTable->psBuckets[i];
            psCurrentNode != NULL;
            psCurrentNode = psNextNode)
    {
        psNextNode = psCurrentNode->psNextNode;
        oSymTable->psBuckets[i] = psNextNode;
        free((char*)psCurrentNode->pcKey);
        free(psCurrentNode);
    }
   }

   free(oSymTable->psBuckets);
   free(oSymTable);
}

/*--------------------------------------------------------------------*/

size_t SymTable_getLength(SymTable_T oSymTable)
{
   assert(oSymTable != NULL);
   return oSymTable->num;
}

/*--------------------------------------------------------------------*/

/* Increase the number of buckets of the oSymTable object. */
static void SymTable_expand(SymTable_T oSymTable)
{
   struct SymTableNode *psCurrentNode;
   struct SymTableNode *psNextNode;
   struct SymTableNode **psNewBuckets;
   size_t i;
   size_t hashKey;
   size_t newIndex = oSymTable->index + 1;

   /* Keep newIndex value within the size of bucket counts array. */
   if (newIndex > numBucketCounts - 1)
   {
      return;
   }

   psNewBuckets = (struct SymTableNode**)
      calloc(auBucketCounts[newIndex], sizeof(struct SymTableNode*));
   if (psNewBuckets == NULL) 
   {
      return;
   }

   for(i = 0; i < auBucketCounts[newIndex - 1]; i++)
   {

    /* Iterate through the linked list to re-hash all nodes in bucket i. */
    for (psCurrentNode = oSymTable->psBuckets[i];
            psCurrentNode != NULL;
            psCurrentNode = psNextNode)
    {
      hashKey = SymTable_hash(psCurrentNode->pcKey, auBucketCounts[newIndex]);
      psNextNode = psCurrentNode->psNextNode; 
      psCurrentNode->psNextNode = psNewBuckets[hashKey]; 
      psNewBuckets[hashKey] = psCurrentNode;
    }
   }

   /* Free old buckets. */
   free(oSymTable->psBuckets);

   /* Update the SymTable. */
   oSymTable->psBuckets = psNewBuckets;
   oSymTable->index = newIndex;
}

/*--------------------------------------------------------------------*/

int SymTable_put(SymTable_T oSymTable, const char *pcKey, const void *pvValue)
{
   struct SymTableNode *psNewNode;
   struct SymTableNode *psNextNode;
   size_t hashKey;

   assert(oSymTable != NULL && pcKey != NULL);

   /* Expand the SymTable object's bucket count upon reaching capacity. */
   if (oSymTable->num == auBucketCounts[oSymTable->index])
   { 
      SymTable_expand(oSymTable);
   }
   
   hashKey = SymTable_hash(pcKey, auBucketCounts[oSymTable->index]);

   /* Iterates through all nodes in the linked list. Return 0 if 
   matching key is found. */
   for (psNewNode = oSymTable->psBuckets[hashKey];
        psNewNode != NULL;
        psNewNode = psNextNode)
   {
      psNextNode = psNewNode->psNextNode;
      if (strcmp(psNewNode->pcKey, pcKey) == 0) {
         return 0;
      }
   }

   /* Allocate memory for the new node and its key. Return 0 if 
   insufficient memory is available. */
   psNewNode = (struct SymTableNode*)malloc(sizeof(struct SymTableNode));
   if (psNewNode == NULL)
   {
      return 0;
   }

   psNewNode->pcKey = (const char*)malloc(strlen(pcKey) + 1);
   if (psNewNode->pcKey == NULL) {
      free(psNewNode); 
      return 0;
   }

   strcpy((char*)psNewNode->pcKey, pcKey); /* Make a defensive copy of pcKey. */
   
   /* Update pvValue and insert the node to the front. */
   psNewNode->pvValue = pvValue;
   psNewNode->psNextNode = oSymTable->psBuckets[hashKey];

   /* Update the SymTable. */
   oSymTable->psBuckets[hashKey] = psNewNode;
   oSymTable->num++;

   return 1;
}

/*--------------------------------------------------------------------*/

void *SymTable_replace(SymTable_T oSymTable, const char *pcKey, const void *pvValue)
{
   struct SymTableNode *psCurrentNode;
   struct SymTableNode *psNextNode;
   void *pvOldValue;
   size_t hashKey;

   assert(oSymTable != NULL && pcKey != NULL);

   hashKey = SymTable_hash(pcKey, auBucketCounts[oSymTable->index]);

   for (psCurrentNode = oSymTable->psBuckets[hashKey];
        psCurrentNode != NULL;
        psCurrentNode = psNextNode)
   {
      psNextNode = psCurrentNode->psNextNode;
      if (strcmp(psCurrentNode->pcKey, pcKey) == 0) {
         pvOldValue = (void*)psCurrentNode->pvValue;
         psCurrentNode->pvValue = pvValue;
         return pvOldValue;
      }
   }
   return NULL;
}

/*--------------------------------------------------------------------*/

int SymTable_contains(SymTable_T oSymTable, const char *pcKey) 
{
   struct SymTableNode *psCurrentNode;
   struct SymTableNode *psNextNode;
   size_t hashKey;

   assert(oSymTable != NULL && pcKey != NULL);

   hashKey = SymTable_hash(pcKey, auBucketCounts[oSymTable->index]);

   for (psCurrentNode = oSymTable->psBuckets[hashKey];
        psCurrentNode != NULL;
        psCurrentNode = psNextNode)
   {
      psNextNode = psCurrentNode->psNextNode;
      if (strcmp(psCurrentNode->pcKey, pcKey) == 0) {
         return 1;
      }
   }
   return 0;
}

/*--------------------------------------------------------------------*/

void *SymTable_get(SymTable_T oSymTable, const char *pcKey)
{
   struct SymTableNode *psCurrentNode;
   struct SymTableNode *psNextNode;
   size_t hashKey;

   assert(oSymTable != NULL && pcKey != NULL);

   hashKey = SymTable_hash(pcKey, auBucketCounts[oSymTable->index]);

   for (psCurrentNode = oSymTable->psBuckets[hashKey];
        psCurrentNode != NULL;
        psCurrentNode = psNextNode)
   {
      psNextNode = psCurrentNode->psNextNode;
      if (strcmp(psCurrentNode->pcKey, pcKey) == 0) {
         return (void*)psCurrentNode->pvValue;
      }
   }
   return NULL;
}

/*--------------------------------------------------------------------*/

void *SymTable_remove(SymTable_T oSymTable, const char *pcKey)
{
   struct SymTableNode *psCurrentNode;
   struct SymTableNode *psPrevNode;
   void *pvOldValue;
   size_t hashKey;

   assert(oSymTable != NULL && pcKey != NULL);

   hashKey = SymTable_hash(pcKey, auBucketCounts[oSymTable->index]);

   /* Handle empty SymTable object. */
   if (oSymTable->psBuckets[hashKey] == NULL)
   {
      return NULL;
   }

   /* Remove the first node of matched key. */
   if (strcmp(oSymTable->psBuckets[hashKey]->pcKey, pcKey) == 0) {
      psCurrentNode = oSymTable->psBuckets[hashKey];
      pvOldValue = (void*)psCurrentNode->pvValue;
      oSymTable->psBuckets[hashKey] = psCurrentNode->psNextNode;
      oSymTable->num--;
      free((char*)psCurrentNode->pcKey);
      free(psCurrentNode);
      return pvOldValue;
   }

   /* Iterates through all nodes in the linked list until finding 
   matching key. Remove the node and return its old value. */
   for (psPrevNode = oSymTable->psBuckets[hashKey], 
        psCurrentNode = psPrevNode->psNextNode;
        psCurrentNode != NULL;
        psPrevNode = psCurrentNode, 
        psCurrentNode = psCurrentNode->psNextNode)
   {
      if (strcmp(psCurrentNode->pcKey, pcKey) == 0) {
         pvOldValue = (void*)psCurrentNode->pvValue;
         psPrevNode->psNextNode = psCurrentNode->psNextNode;
         oSymTable->num--;
         free((char*)psCurrentNode->pcKey);
         free(psCurrentNode);
         return pvOldValue;
      }
   }
   return NULL;
}

/*--------------------------------------------------------------------*/

void SymTable_map(SymTable_T oSymTable,
    void (*pfApply)(const char *pcKey, void *pvValue, void *pvExtra),
    const void *pvExtra)
{
   struct SymTableNode *psCurrentNode;
   size_t i;

   assert(oSymTable != NULL && pfApply != NULL);

   for(i = 0; i < auBucketCounts[oSymTable->index]; i++)
   {
    for (psCurrentNode = oSymTable->psBuckets[i];
         psCurrentNode != NULL;
         psCurrentNode = psCurrentNode->psNextNode) 
         {
            (*pfApply)(psCurrentNode->pcKey, (void*)psCurrentNode->pvValue, (void*)pvExtra);
         }
   }
}