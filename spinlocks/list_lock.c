/*
 * Linked list protected by a single lock.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * Copyright (c) 2002 IBM Corporation.
 * Copyright (c) 2005 Tom Hart.
 */

#include <stdio.h>
#include <string.h>
#include "locks/picklock.h" /* To define spinlock types to use */
#include "list.h"
// #define DEBUG

struct list {
	spinlock_t list_lock;
	node_t *list_head;
};

void list_init(struct list **l)
{
	*l = (struct list *)malloc(sizeof(struct list));
	spin_init(&((*l)->list_lock));
	(*l)->list_head = NULL;
}

void list_destroy(struct list **l)
{
	free(*l);
}

/* Debugging function. */
void show(struct list *l) {
	node_t *cur;
	
	for (cur = l->list_head; cur != NULL; cur=cur->next)
		printf("{%lu} ", cur->key);
	printf("\n");
}

int search(struct list *l, unsigned long key)
{	
	node_t *cur;

	spin_lock(&l->list_lock);
	
	for (cur = l->list_head; cur != NULL; cur=cur->next) {
		if (cur->key >= key) {
			int ret = cur->key == key;
			spin_unlock(&l->list_lock);
			return ret;
		}
	}
	
	spin_unlock(&l->list_lock);
	return 0;
}

int delete(struct list *l, unsigned long key)
{
	node_t *cur;
	node_t **prev;

	spin_lock(&l->list_lock);
	
	prev = &l->list_head;

	for (cur = *prev; cur != NULL; cur=cur->next) {
		if (cur->key == key) {
			*prev = cur->next;
			free(cur);
#ifdef DEBUG
			printf("[delete %lu (1)] ", key);
			show(l);
#endif
			spin_unlock(&l->list_lock);
			return 1;
		}
		prev = &cur->next;
	}
#ifdef DEBUG
	printf("[delete %lu (0)] ", key);
	show(l);
#endif
	spin_unlock(&l->list_lock);
	return 0;
}

int insert(struct list *l, unsigned long key)
{
	node_t *cur;
	node_t **prev;
	node_t *newnode;

	spin_lock(&l->list_lock);
	
	/* Find cur, prev s. th. *prev=cur, and (cur=NULL or cur->key>=key). */
	prev = &l->list_head;
	for (cur = *prev; 
	     cur != NULL && cur->key < key; 
	     cur=cur->next, prev=&(*prev)->next) {}

	/* Abort if the key is already in the list. */
	if (cur != NULL) {
	  if (cur->key == key) {
#ifdef DEBUG
		printf("[insert %lu (0)] ", key);
		show(l);
#endif
	    spin_unlock(&l->list_lock);
	    return 0;
	  }
	}

	/* Insert the new key. */
	newnode = (node_t *)malloc(sizeof(node_t));
	newnode->key = key;
	newnode->next = cur;
	*prev = newnode;

#ifdef DEBUG
	printf("[insert %lu (1)] ", key);
	show(l);
#endif
	spin_unlock(&l->list_lock);
	return 1;
}
