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
	newBlock->is_free = 1;
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

void deleteAtBeginning(Block** head)
{
    // checking if the DLL is empty
    if (*head == NULL) {
        printf("The list is already empty.\n");
        return;
    }
    Block* temp = *head;
    *head = (*head)->next;
    if (*head != NULL) {
        (*head)->prev = NULL;
    }
    free(temp);
}

void deleteAtEnd(Block** head)
{
    // checking if DLL is empty
    if (*head == NULL) {
        printf("The list is already empty.\n");
        return;
    }

    Block* temp = *head;
    if (temp->next == NULL) {
        *head = NULL;
        free(temp);
        return;
    }
    while (temp->next != NULL) {
        temp = temp->next;
    }
    temp->prev->next = NULL;
    free(temp);
}


void deleteAtPosition(Block** head, int position)
{
	if (*head == NULL) {
		printf("The list is already empty.\n");
		return;
	}
	Block* temp = *head;
	if (position == 1) {
		deleteAtBeginning(head);
		return;
	}
	for (int i = 1; temp != NULL && i < position; i++) {
		temp = temp->next;
	}
	if (temp == NULL) {
		printf("Position is greater than the number of "
		 "nodes.\n");
		return;
	}
	if (temp->next != NULL) {
		temp->next->prev = temp->prev;
	}
	if (temp->prev != NULL) {
		temp->prev->next = temp->next;
	}
	free(temp);
}

void printBlock(Block* b){
	printf("process_id: %d\n size: %d\n is_free:%d\n nextPI: %d\n prevPI: %d\n", b->process_id, b->size, b->is_free, b->next->process_id, b->prev->process_id);
}

void printListForward(Block* head)
{
    Block* temp = head;
    printf("Forward List: \n");
	int position = 1;
    while (temp != NULL) {
        printf("position: %d ; isfree: %d ; processid: %d ;  size: %d ;\n", position, temp->is_free, temp->process_id, temp->size);
        temp = temp->next;
		position++;
    }
    printf("\n");
}

void printListReverse(Block* head)
{
	Block* temp = head;
    if (temp == NULL) {
        printf("The list is empty.\n");
        return;
    }
    // Move to the end of the list
    while (temp->next != NULL) {
        temp = temp->next;
    }
    // Traverse backwards
    printf("Reverse List: \n");
    while (temp != NULL) {
        printf("isfree: %d ; processid: %d ;  size: %d ;\n", temp->is_free, temp->process_id, temp->size);
        temp = temp->prev;
    }
    printf("\n");
}

Block* findByPI(Block* head, int process_id)
{
    Block* temp = head;
    while (temp != NULL) {
        if(temp->process_id == process_id)
			return temp;
		temp = temp->next;
    }
	return NULL;
}

Block* findFirstFit(Block* head, int size){
	//block.size >= size
	//NULL = it did not find a free block that fit the given size
	Block* temp = head;
	
    while (temp != NULL) {
		if((temp->is_free ==1) && (temp->size >= size)){
			return temp;
		}
		temp = temp->next;
    }
    return NULL;
}

Block* findBestFit(Block* head, int size){
	//NULL = it did not find a free block that fit the given size
	Block* temp = head;
	Block* tempBF = createBlock(99999, 99999);
	
    while (temp != NULL) {
		if((temp->is_free==1) && (temp->size < tempBF->size) && (temp->size >= size)) {
			tempBF = temp; 
		}
		temp = temp->next;
    }
	
	if(tempBF->size == 99999)
		return NULL;
	
    return tempBF;
}

Block* findWorstFit(Block* head, int size){
	//NULL = it did not find a free block that fit the given size
	Block* temp = head;
	Block* tempWF = createBlock(99999, 0);
	
    while (temp != NULL) {
		//printf("temp_id %d ; temp_size %d ; tempWF_id %d ; tempWF_size %d ; temp->size >= size %d ; temp->size > tempWF->size %d \n", temp->process_id, temp->size, tempWF->process_id, tempWF->size,(temp->size >= size), (temp->size > tempWF->size) );
		if((temp->is_free == 1)&& (temp->size > tempWF->size) && (temp->size >= size)) {
			tempWF = temp; 
		}
		temp = temp->next;
    }
	
	if(tempWF->size == -1)
		return NULL;
	
    return tempWF;
}

int alloc(Block* head, int process_id, int size, int algorithm){
	//return values: -2=no recognized algorithm code ; -1=no block found with the size required ; 0 = unknown error ; 1= well allocated
	//int algorithm: FF=1;BF=2;WF=3;
	Block* temp;
	switch(algorithm){
		case(1):
			temp = findFirstFit(head, size);
			break;
		case(2):
			temp = findBestFit(head, size);
			break;
		case(3):
			temp = findWorstFit(head, size);
			break;
		default:
			return -2;
	}
	if(temp == NULL){
		return -1;
	}
	
	if((temp->is_free==1)&&(temp->size >= size)){
		int prevSize = temp->size;
		temp->size=size;
		temp->is_free = 0;
		temp->process_id = process_id;
		if(prevSize-size>0){
			printf("entre aquiiii 2 prevSize-size=%d \n \n", prevSize-size);
			Block* newBlock = createBlock(-1, prevSize-size);
			newBlock->next = temp->next;
			newBlock->prev = temp;
			if(temp->next != NULL){
				temp->next->prev = newBlock;
			}
			temp->next = newBlock;
		}
		return 1;
	}
	
	return 0;
}

int coalescence(Block* head){
	for(int i=0; i<999; i++){
		Block* temp = head;
		int pos = 1;
		while (temp != NULL) {
			if((temp->is_free ==1) && (temp->next != NULL) && (temp->next->is_free == 1)){
				temp->size +=temp->next->size;
				deleteAtPosition(&head, pos+1);
			}
			temp = temp->next;
			pos++;
		}
	}
    return 0;
}

int freeMem(Block* head, int process_id){
	Block* temp = findByPI(head, process_id);
	if(temp==NULL)
		return -1;
	temp->is_free = 1;
	coalescence(head);
	return 1;
	
}



int main(){
	
	Block* head = NULL;
	//MEMORIA INICIAL
	insertAtEnd(&head, 1, 10);
	//insertAtEnd(&head, 2, 5);
	//insertAtEnd(&head, 3, 3);
	//Block* blockFF = findFirstFit(head, test);
	//Block* blockBF = findBestFit(head, test);
	//Block* blockWF = findWorstFit(head, test);
	//printf("el process_id para FF es: %d \n", blockFF != NULL ? blockFF->process_id : -1);
	//printf("el process_id para BF es: %d \n", blockBF != NULL ? blockBF->process_id : -1);
	//printf("el process_id para WF es: %d \n", blockWF != NULL ? blockWF->process_id : -1);
	//alloc(head, 77, 3, 1);
	//alloc(head, 69, 3, 1);
	insertAtEnd(&head, 2, 200);
	insertAtEnd(&head, 3, 40);
	insertAtEnd(&head, 4, 300);
	insertAtEnd(&head, 5, 50);
	insertAtEnd(&head, 6, 60);
	
	head->is_free = 1;
	head->next->is_free = 1;
	head->next->next->is_free = 1;
	head->next->next->next->is_free = 1;
	head->next->next->next->next->is_free = 1;
	head->next->next->next->next->next->is_free = 1;
	
	//alloc(head, 77, 35, 1);
	//alloc(head, 77, 35, 2);
	//alloc(head, 77, 35, 3);
	//printf("%d \n\n", alloc(head, 77, 35, 1));
	
	printListForward(head);
	Block* PI = findByPI(head, 2);
	//printf("pid %d\n size %d \n", PI->process_id, PI->size);
	//printBlock(PI);

	//printListForward(head);
	//coalescence(head);
	//printListForward(head);
	
	return 0;
}
