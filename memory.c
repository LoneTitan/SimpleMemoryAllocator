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
		printf("Unable to allocate RAM space\n");
		exit(0);
	}
	return base;
}

static void free_ram(void *addr, size_t size)
{
	munmap(addr, size);
}
static struct node* map[14] = {};
void myfree(void *ptr)
{
	struct metadata* mdadd = (struct metadata*)((unsigned long)(ptr)& ~0xfff);
	if(mdadd->allocated > 4080){
		free_ram(ptr, mdadd -> allocated);
		return;
	}
	int size = mdadd->allocated;
	// printf("Current : %d", size);
	mdadd -> left += mdadd -> allocated;
	struct node* help = ptr;
	int round = ro(size);
	struct node* traverse = map[round];
	help->next = traverse;
	help->prev = NULL;
	help -> prev = help;
	map[round] = help;
	if(mdadd -> allocated == 4080){
		free_ram(mdadd, 4096);
	}
}
int ro(int n){
	int c = 2;
	if(n <= 4080){
		for(int i = 1; i <= 15; i++){
			// printf("%d %d`", n, i);
			if(1<<i >= n){
				// printf("%d %d`", n, 1<<c);
				return i;
			}
			c*=2;
		}
	}
	else{
		n += 16;
		for(int i = 1; i <= 150; i++){
			// printf("%d     ", i);
			if(1<<i >= n){
				// printf("%d %d", n, 1<<c);
				return i;
			}
			c *= 2;

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
			md -> left = 4096;
			struct node* new_node = (void*) mdp+16;
			map[round] = new_node;
			struct node* ref = new_node;
			new_node -> prev = NULL;
			new_node -> next = NULL;
			printf("%p 		 %p %lu\n", mdp, ref,size);
			for(int i = 0; i < 4080  - (1<<round); i = i + (1<<round) ){
				struct node* ext_node = ( struct node*)((void*)ref + (1<<round));
				printf("%p 		 %p     %p      %d\n", mdp, ext_node,map[round], round);
				fflush(NULL);
				ext_node -> prev = ref;
				ext_node -> next = NULL;
				ref -> next = ext_node;
				ref = ext_node;
			}
		}
		// printf(" 	gg	 \n");
		fflush(NULL);
		void* ans = map[round];
		struct metadata* mdadd = (struct metadata*)((unsigned long)(map[round])& ~0xfff);

		// printf(" 	gg	 \n");
		map[round] = map[round]->next;
		// printf(" 	gg	 \n");
		// map[round]->prev = NULL;

		// printf(" 	gg	%p \n",mdadd);
		mdadd -> left -= (1<<round);

		// printf(" 	gg	 \n");
		return ans;
	}
	else{
		struct metadata* temp = alloc_from_ram(1<<round);
		// printf("%d    hsh ", round);
		fflush(NULL);
		temp -> left = 0;
		temp -> allocated = 1<<round;
		// while(1);
		return (void*)temp + 16;
	}
}
