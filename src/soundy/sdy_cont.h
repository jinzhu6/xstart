/* Bastian von Halem  27.03.2004 */

#ifndef _9631B431_FB55_4146_BDD1_180FB193C59F_
#define _9631B431_FB55_4146_BDD1_180FB193C59F_

#include <malloc.h>
#include <memory.h>
#include <assert.h>

#define INLINE static __inline

/* C-Container item structure */
typedef struct CONT_ITEM
{
	void* pData;
	void* pPrev;
	void* pNext;
	void* pCont;

} CONT_ITEM;

/* C-Container structure */
typedef struct CONT
{
	CONT_ITEM*		pFirst;
	CONT_ITEM*		pLast;
	unsigned long	nItems;

} CONT;

INLINE CONT* cnew()
{
	CONT* pCont;
	pCont = (CONT*)malloc(sizeof(CONT));
	assert(pCont);
	memset(pCont, 0, sizeof(CONT));
	return pCont;
}

INLINE void cdel(CONT *c)
{
	assert(c);
	assert(c->nItems == 0);		/* be sure container is empty */
	free(c);
}

INLINE int ccount(CONT* c)
{
	assert(c);
	return c->nItems;
}

/* push back into container */
INLINE CONT_ITEM* cpush(CONT* c, const void* d)
{
	CONT_ITEM* i;

	assert(c);
	assert(d);

	i = (CONT_ITEM*)malloc(sizeof(CONT_ITEM));

	i->pNext = 0;
	i->pPrev = 0;
	i->pData = (void*)d;
	i->pCont = c;

	c->nItems++;

	if( !c->pFirst ) {
		c->pFirst = i;
		c->pLast = i;
		return i; }

	c->pLast->pNext = i;
	i->pPrev = c->pLast;

	c->pLast = i;

	return i;
}

/* push front into container */
INLINE CONT_ITEM* cpush_front(CONT* c, void* d)
{
	CONT_ITEM* i;

	assert(c);
	assert(d);

	i = (CONT_ITEM*)malloc(sizeof(CONT_ITEM));

	i->pNext = 0;
	i->pPrev = 0;
	i->pData = d;
	i->pCont = c;

	c->nItems++;

	if( !c->pFirst ) {
		c->pFirst = i;
		c->pLast = i;
		return i; }

	c->pFirst->pPrev = i;
	i->pNext = c->pFirst;

	c->pFirst = i;

	return i;
}

/* push after item */
INLINE CONT_ITEM* cpush_after(CONT* c, CONT_ITEM* i, void* d)
{
	CONT_ITEM* t;

	assert(c);
	assert(i);
	assert(d);

	t = (CONT_ITEM*)malloc(sizeof(CONT_ITEM));

	t->pCont = c;
	t->pData = d;
	t->pNext = i->pNext;
	i->pNext = t;
	t->pPrev = i;
	if(t->pNext) ((CONT_ITEM*)t->pNext)->pPrev = t;

	if(c->pLast == i) c->pLast = t;

	c->nItems++;

	return t;
}

/* push infront of item */
INLINE CONT_ITEM* cpush_before(CONT* c, CONT_ITEM* i, void* d)
{
	CONT_ITEM* t;

	assert(c);
	assert(i);
	assert(d);

	t = (CONT_ITEM*)malloc(sizeof(CONT_ITEM));

	t->pCont = c;
	t->pData = d;
	t->pPrev = i->pPrev;
	i->pPrev = t;
	t->pNext = i;
	if(t->pPrev) ((CONT_ITEM*)t->pPrev)->pNext = t;

	if(c->pFirst == i) c->pFirst = t;

	c->nItems++;

	return t;
}

INLINE void* cget(CONT* pCont, int nItem)
{
	CONT_ITEM* pItem;
	int i = 0;

	assert(pCont);

	pItem = pCont->pFirst;

	while(pItem)
	{
		if(i == nItem) return pItem->pData;
		pItem = (CONT_ITEM*)pItem->pNext;
		i++;
	}

	return 0;
}

/* remove specified item from the container */
INLINE void crem(CONT* pCont, CONT_ITEM* pItem)
{
	if(pItem->pNext)
		((CONT_ITEM*)pItem->pNext)->pPrev = pItem->pPrev;

	if(pItem->pPrev)
		((CONT_ITEM*)pItem->pPrev)->pNext = pItem->pNext;

	if(pCont->pLast == pItem)
		pCont->pLast = (CONT_ITEM*)pItem->pPrev;

	if(pCont->pFirst == pItem)
		pCont->pFirst = (CONT_ITEM*)pItem->pNext;

	free(pItem);

	pCont->nItems--;
}

/* pop last item from the container */
/* TODO: Optimize */
INLINE void* cpop(CONT* pCont)
{
	void* pData;

	assert(pCont);

	if(!pCont->nItems)
		return 0;

	pData = pCont->pLast->pData;

	crem(pCont, pCont->pLast);

	return pData;
}

INLINE void* cpop_front(CONT* c)
{
	void* d;

	assert(c);

	if(!c->nItems)
		return 0;

	d = c->pFirst->pData;

	crem(c, c->pFirst);

	return d;
}

/* search item in the container */
INLINE CONT_ITEM* csearch(CONT *pCont, void *pData)
{
	CONT_ITEM *pItem;

	pItem = pCont->pFirst;

	if(!pItem) return 0;

	do
	{
		if(pItem->pData == pData) return pItem;
		pItem = (CONT_ITEM*)pItem->pNext;

	} while(pItem);

	return 0;
}


INLINE void crndpop(CONT *pCont, void *pData)
{
	CONT_ITEM *pItem = csearch(pCont, pData);

	assert(pItem);

	crem(pCont, pItem);
}

INLINE CONT_ITEM* cfirst(CONT *pCont)
{
	return pCont->pFirst;
}

INLINE CONT_ITEM* clast(CONT *pCont)
{
	return pCont->pLast;
}

INLINE CONT_ITEM* cinext(CONT_ITEM* pItem)
{
	return (CONT_ITEM*)pItem->pNext;
}

INLINE CONT_ITEM* ciprev(CONT_ITEM* pItem)
{
	return (CONT_ITEM*)pItem->pPrev;
}

INLINE void* cidata(CONT_ITEM* pItem)
{
	return pItem->pData;
}


#endif
