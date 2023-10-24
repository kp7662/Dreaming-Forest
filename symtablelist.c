/*--------------------------------------------------------------------*/
/* symtablelist.c                                                     */
/* Author: Kok Wei Pua                                                */
/*--------------------------------------------------------------------*/

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "symtable.h"

/*--------------------------------------------------------------------*/

/* Each key and value is stored in a SymTableNode.  SymTableNodes are 
   linked to form a list.  */

struct SymTableNode
{
   /* The key. */
   const char *pcKey;

   /* The value. */
   const void *pvValue;

   /* The address of the next SymTableNode. */
   struct SymTableNode *psNextNode;
};

/*--------------------------------------------------------------------*/

/* A SymTable is a "dummy" node that points to the first SymtableNode 
and store the number of bindings in the SymTable. */

struct SymTable
{
   /* The address of the first SymtableNode. */
   struct SymTableNode *psFirstNode;

   /* The number of bindings. */
   size_t num;
};

/*--------------------------------------------------------------------*/

SymTable_T SymTable_new(void)
{
   SymTable_T oSymTable;

   oSymTable = (SymTable_T)malloc(sizeof(struct SymTable));
   if (oSymTable == NULL)
      return NULL;

   oSymTable->psFirstNode = NULL;
   oSymTable->num = 0;
   return oSymTable;
}

/*--------------------------------------------------------------------*/

void SymTable_free(SymTable_T oSymTable)
{
   struct SymTableNode *psCurrentNode;
   struct SymTableNode *psNextNode;

   assert(oSymTable != NULL);

   for (psCurrentNode = oSymTable->psFirstNode;
        psCurrentNode != NULL;
        psCurrentNode = psNextNode)
   {
      psNextNode = psCurrentNode->psNextNode;
      free((char*)psCurrentNode->pcKey);
      free(psCurrentNode);
   }

   free(oSymTable);
}

/*--------------------------------------------------------------------*/

size_t SymTable_getLength(SymTable_T oSymTable)
{
   assert(oSymTable != NULL);
   return oSymTable->num;
}

/*--------------------------------------------------------------------*/

int SymTable_put(SymTable_T oSymTable, const char *pcKey, 
                 const void *pvValue)
{
   struct SymTableNode *psNewNode;
   struct SymTableNode *psNextNode;

   assert(oSymTable != NULL && pcKey != NULL);

   /* Iterates through all nodes in the linked list. Return 0 if 
   matching key is found.*/
   for (psNewNode = oSymTable->psFirstNode;
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
   psNewNode->psNextNode = oSymTable->psFirstNode;

   /* Update the SymTable. */
   oSymTable->psFirstNode = psNewNode;
   oSymTable->num++;

   return 1;
}

/*--------------------------------------------------------------------*/

void *SymTable_replace(SymTable_T oSymTable, const char *pcKey, const void *pvValue)
{
   struct SymTableNode *psCurrentNode;
   struct SymTableNode *psNextNode;
   void *pvOldValue;

   assert(oSymTable != NULL && pcKey != NULL);
   
   /* Iterates through all nodes in the linked list until finding matching key. 
   Replace its value and return its old value. */
   for (psCurrentNode = oSymTable->psFirstNode;
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

   assert(oSymTable != NULL && pcKey != NULL);

   for (psCurrentNode = oSymTable->psFirstNode;
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

   assert(oSymTable != NULL && pcKey != NULL);

   for (psCurrentNode = oSymTable->psFirstNode;
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

   assert(oSymTable != NULL && pcKey != NULL);

   if (oSymTable->num == 0)
   {
      return NULL;
   }

   /* Remove the first node of matched key. */
   if (strcmp(oSymTable->psFirstNode->pcKey, pcKey) == 0) {
      psCurrentNode = oSymTable->psFirstNode;
      pvOldValue = (void*)psCurrentNode->pvValue;
      oSymTable->psFirstNode = psCurrentNode->psNextNode;
      oSymTable->num--;
      free((char*)psCurrentNode->pcKey);
      free(psCurrentNode);
      return pvOldValue;
   }

   /* Iterates through all nodes in the linked list until finding 
   matching key. Remove the node and return its old value. */
   for (psPrevNode = oSymTable->psFirstNode, psCurrentNode = psPrevNode->psNextNode;
        psCurrentNode != NULL;
        psPrevNode = psCurrentNode, psCurrentNode = psCurrentNode->psNextNode)
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

   assert(oSymTable != NULL && pfApply != NULL);

   for (psCurrentNode = oSymTable->psFirstNode;
        psCurrentNode != NULL;
        psCurrentNode = psCurrentNode->psNextNode) 
        {
         (*pfApply)(psCurrentNode->pcKey, (void*)psCurrentNode->pvValue, (void*)pvExtra);
        }
}