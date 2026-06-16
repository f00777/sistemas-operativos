#include <stdio.h>
#include <stdlib.h>

//refs: https://www.geeksforgeeks.org/c/doubly-linked-list-in-c/

typedef struct Block{
	int is_free;
	int process_id;
	size_t size;
	struct Block* next;
	struct Block* prev;
}Block;

Block* createBlock(int process_id, size_t size){
	Block* newBlock = (Block*)malloc(sizeof(Block));
	newBlock->is_free = 0;
	newBlock->process_id = process_id;
	newBlock->size = size;
	newBlock->next = NULL;
	newBlock->prev = NULL;
	return newBlock;
}

void insertAtBeginning(Block** head, int process_id, size_t size){
	Block* newBlock = createBlock(process_id, size);
	if(*head == NULL){
		*head = newBlock;
		return;
	}
	newBlock->next = *head;
	(*head)->prev = newBlock;
	*head = newBlock;
}

void insertAtEnd(Block** head, int process_id, size_t size){
	Block* newBlock = createBlock(process_id, size);
	if(*head==NULL){
		*head = newBlock;
		return;
	}
	Block* temp = *head;
	while(temp->next != NULL){
		temp = temp->next;
	}
	temp->next = newBlock;
	newBlock->prev = temp;
}

void insertAtPosition(Block** head, int process_id, size_t size, int position)
{
    if (position < 1) {
        printf("Position should be >= 1.\n");
        return;
    }
    // if we are inserting at head
    if (position == 1) {
        insertAtBeginning(head, process_id, size);
        return;
    }
    Block* newBlock = createBlock(process_id, size);
    Block* temp = *head;
    for (int i = 1; temp != NULL && i < position - 1; i++) {
        temp = temp->next;
    }
    if (temp == NULL) {
        printf(
            "Position greater than the number of nodes.\n");
        return;
    }
    newBlock->next = temp->next;
    newBlock->prev = temp;
    if (temp->next != NULL) {
        temp->next->prev = newBlock;
    }
    temp->next = newBlock;
}

int main(){
	printf("Hola mundo \n");
	Block* head = NULL;
	//Block* bloque1 = createBlock(2333, 1000);
	//printf("el tamano del proceso es: %d \n", bloque1->size);
	insertAtEnd(&head, 4, 10);
	insertAtEnd(&head, 8, 10);
	insertAtEnd(&head, 6, 10);
	printf("el id del head es: %d \n", head->next->next->prev->prev->process_id);
	insertAtPosition(&head, 777, 1000, 2);
	printf("el id de la segunda posicion es: %d \n", head->next->process_id);
	//printf("el id del proceso prev es: %d \n", bloque1->prev->process_id);
	
	//free(bloque1);
	
	return 0;
}