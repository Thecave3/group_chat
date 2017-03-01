#include "server_utils.h"

void goodbye (void) {
	while (client_list != NULL) {
		client_l aux = client_list;
		client_list = client_list->next;
		free(aux);
	}
	sem_destroy(&client_list_semaphore);
	fprintf(stderr, "\nThe server say goodbye!\n");
}
