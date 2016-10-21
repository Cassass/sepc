#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/wait.h>

#include "jobs_handler.h"
struct cell 
{
	int pid;
	char *command;
	struct cell* next;
};

// permettra def et alloc en global de la liste des pid's
/* struct cell** pid_list = malloc(sizeof(struct cell*)); */
/* *pid_list = malloc(sizeof(struct cell)); */
struct cell *pid_list = NULL;

//manipulation de la liste chainee utilisee pour jobs

//insertion en tete
void add_pid_list(int p, char *cmd){
	struct cell *new = malloc(sizeof(struct cell));
	new->pid = p;
	new->next = pid_list;
	new->command = malloc(sizeof(char)*strlen(cmd));
	strcpy(new->command, cmd);
	pid_list = new;
}

//suppresion
void delete_pid(int to_destroy){
	struct cell* current = pid_list;
	struct cell* tbd = NULL;
	// current == NULL gere par while dans check_running
	if (current->pid == to_destroy){
		// to_destroy en tete de liste
		pid_list = current->next;
		free(current);
	}else{
		while(current->next->pid != to_destroy){
			current = current->next;
		}
		tbd = current->next;
		//current est juste avant 
		current->next = tbd->next;
		free(tbd);
	}
}

//parcours la liste chainee à la recherche des processus achevés
void check_running(void){
	struct cell* current;
	current = pid_list;
	while (current != NULL){
		if (waitpid(current->pid, NULL, WNOHANG) != 0){ //pas sur pour NULL
			// autruche pour == -1, a traiter comme il se doit
			// on supprime le pid de la liste si jamais le processus est terminé
			delete_pid(current->pid);
		}else{
			//le processus concerne est toujours actif, on affiche
			printf("[%d]+  Running \t\t%s \n", current->pid, current->command);
		}

		current = current->next;
	}
	
	// remarquons que la suppression n'est pas nécessaire,
	// afficher les processus actifs est suffisant
}

// commande shell `jobs` maison
void jobs(void){
	// parcours de la liste, affiche si processus actif, supprime sinon
	check_running();
}
