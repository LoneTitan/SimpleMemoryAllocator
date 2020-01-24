#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>
#include "memory.h"

#define PAGE_SIZE 4096
int ro(int);
int rofree(int);
struct metadata{
	int left;
	int allocated;
};

struct node{
    struct node* prev;
    struct node* next;
};

static void *alloc_from_ram(size_t size)
{
	assert((size % PAGE_SIZE) == 0 && "size must be multiples of 4096");
	void* base = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_ANON|MAP_PRIVATE, -1, 0);
	if (base == MAP_FAILED)
	{
		// printf("Unable to allocate RAM space\n");
		exit(0);
	}
	return base;
}

static void free_ram(void *addr, size_t size)
{
	munmap(addr, size);
}
static struct node* map[20] = {};
void myfree(void *ptr)
{
	// printf("%p  Freed\n", ptr);
	unsigned long whatever = 0xfff;
	struct metadata* mdadd = (struct metadata*)((unsigned long)(ptr)& ~whatever);
	if(mdadd -> left == -90){
		// printf("%p\n", mdadd);
		fflush(NULL);
		free_ram(mdadd, mdadd -> allocated);
		return;
	}
	int size = mdadd->allocated;
	mdadd -> left += size;
	struct node* help = (struct node*)ptr;
	int round = ro(size);
	struct node* traverse = map[round];
	if(map[round] != NULL){
		help->next = map[round];
		help->prev = NULL;
		map[round] -> prev = help;
	}
	else{
		help->prev = NULL;
		help->next = NULL;
	}
	map[round] = help;
	if(mdadd -> left == 4080){
		struct node* ll = map[round];
		while(ll != NULL){
			// printf("%p   %p   %p ll free\n", ll, ll->next, ll->prev);
			if(mdadd == (struct metadata*)((unsigned long)(ll)& ~0xfff)){
				// next :- next element
				// prev :- prev previous
				if(ll -> prev != NULL){
					(ll->prev) -> next = ll->next;
					if(ll -> next != NULL){
						(ll->next)-> prev = ll->prev;
					}
					ll = ll->next;
				}
				else{
					if(ll->next != NULL){
						(ll->next) -> prev = NULL;
						ll = ll->next;
						map[round] = map[round]->next;
					}
					else{
						map[round] = NULL;
						ll = NULL;
					}
				}
				// printf("asjfojho9fghsg\n");
			}
			else{
				ll = ll->next;
			}
		}
		free_ram(mdadd, 4096);
	}

}

int rofree(int n){
	int c = 2;
	for(int i = 1; i <= 15; i++){
		if(1<<i >= n){
			return i;
		}
		c*=2;
	}
	return 0;
}

int ro(int n){
	if(n <= 16){
		return 4;
	}
	if(n <= 4080){
		for(int i = 1; i <= 15; i++){
			if(1<<i >= n){
				return i;
			}
		}
	}
	else{
		n += 16;
		for(int i = 1; i <= 150; i++){
			if(1<<i >= n){
				return i;
			}
		}
	}
	return 0;
}

void *mymalloc(size_t size){
	int round = ro(size);
	if(size <= 4080){
		if(map[round] == NULL){
			void* mdp = alloc_from_ram(4096);
			struct metadata* md = mdp;
			md -> allocated  = 1<<round;
			md -> left = 4080;
			struct node* new_node = (void*) mdp+16;
			map[round] = new_node;
			struct node* ref = new_node;
			new_node -> prev = NULL;
			new_node -> next = NULL;
			for(int i = 0; i <= 4080 - (1<<round); i+=  (1<<round)){
				struct node* ext_node = ( struct node*)((void*)ref + (1<<round));
				ext_node -> prev = ref;
				ext_node -> next = NULL;
				ref -> next = ext_node;
				// printf("%p   %p   %p ll malloc\n", ext_node, ext_node->next, ext_node->prev);
				ref = ext_node;
			}
		}
		void* ans = map[round];
		struct metadata* mdadd = (struct metadata*)((unsigned long)(map[round])& ~0xfff);

		map[round] = map[round]->next;

		mdadd -> left -= (1<<round);
		// printf("%p   MALLOC\n", ans);
		return ans;
	}
	else{
		struct metadata* temp = alloc_from_ram(1<<round);
		fflush(NULL);
		temp -> left = -90;
		temp -> allocated = 1<<round;
		// printf("%p   MALLOC   <4080\n", (void*)temp + 16);
		return (void*)temp + 16;
	}
}
