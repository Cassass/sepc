/*****************************************************
 * Copyright Grégory Mounié 2008-2015                *
 *           Simon Nieuviarts 2002-2009              *
 * This code is distributed under the GLPv3 licence. *
 * Ce code est distribué sous la licence GPLv3+.     *
 *****************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "variante.h"
#include "readcmd.h"

#ifndef VARIANTE
#error "Variante non défini !!"
#endif

/* Guile (1.8 and 2.0) is auto-detected by cmake */
/* To disable Scheme interpreter (Guile support), comment the
 * following lines.  You may also have to comment related pkg-config
 * lines in CMakeLists.txt.
 */

#if USE_GUILE == 1
#include <libguile.h>

struct cell 
{
	int pid;
	char *command;
	struct cell* next;
};

int question6_executer(char *line)
{
	/* Question 6: Insert your code to execute the command line
	 * identically to the standard execution scheme:
	 * parsecmd, then fork+execvp, for a single command.
	 * pipe and i/o redirection are not required.
	 */
	printf("Not implemented yet: can not execute %s\n", line);

	/* Remove this line when using parsecmd as it will free it */
	free(line);
	
	return 0;
}

SCM executer_wrapper(SCM x)
{
        return scm_from_int(question6_executer(scm_to_locale_stringn(x, 0)));
}
#endif


void terminate(char *line) {
#if USE_GNU_READLINE == 1
	/* rl_clear_history() does not exist yet in centOS 6 */
	clear_history();
#endif
	if (line)
		free(line);
	printf("exit\n");
	exit(0);
}

//manipulation de la liste chainee utilisee pour jobs

//insertion en tete
void add_pid_list(struct cell **ptr_list, int p, char *cmd){
	//int max = 0;
	struct cell *new = malloc(sizeof(struct cell));
	new->pid = p;
	new->next = *ptr_list;
	new->command = malloc(sizeof(char)*strlen(cmd));
	strcpy(new->command, cmd);
	*ptr_list = new;
}

//suppresion
//ya du factorisable la dedans
void delete_pid(struct cell **ptr_list, struct cell *tbd){
	struct cell* current = *ptr_list;

	if (tbd->next == NULL){
		free(tbd); // queue de liste
	}else{
		if (current->pid == tbd->pid){
			// current "=" tbd puisque le pid est unique
			// tbd en tete de liste
			*ptr_list = current->next;
			free(current);
		}else{
			while(current->next->pid != tbd->pid){
				current = current->next;
			}
			//current est juste avant 
			current->next = tbd->next;
			free(tbd);
		}
	}
}

//parcours la liste chainee à la recherche des processus achevés
void check_running(struct cell **ptr_list){
	struct cell* current;
	current = *ptr_list;
	while (current != NULL){
		if (waitpid(current->pid, NULL, WNOHANG) != 0){ //pas sur pour NULL
			// autruche pour == -1, a traiter comme il se doit
			// on supprime le pid de la liste si jamais le processus est terminé
			delete_pid(ptr_list, current);
		}else{
			//le processus concerne est toujours actif, on affiche
			printf("[%d]+  Running \t\t%s", current->pid, current->command);
		}
		printf("\n");

		current = current->next;
	}
	
	// remarquons que la suppression n'est pas nécessaire,
	// afficher les processus actifs est suffisant
}

// commande shell `jobs` maison
void jobs(struct cell **pid_list){
	// parcours de la liste, affiche si processus actif, supprime sinon
	check_running(pid_list);
}

int main() {
        printf("Variante %d: %s\n", VARIANTE, VARIANTE_STRING);

#if USE_GUILE == 1
        scm_init_guile();
        /* register "executer" function in scheme */
        scm_c_define_gsubr("executer", 1, 0, 0, executer_wrapper);
#endif
	// allocation de la liste des pid pour jobs
	struct cell** pid_list = malloc(sizeof(struct cell*));
	*pid_list = malloc(sizeof(struct cell));
	*pid_list = NULL;
	
	while (1) {
		struct cmdline *l;
		char *line=0;
		int i;
		char *prompt = "ensishell>";
		int input = STDIN_FILENO; //e.g input = 0
		int output = STDOUT_FILENO; //e.g output = 1
		/* Readline use some internal memory structure that
		   can not be cleaned at the end of the program. Thus
		   one memory leak per command seems unavoidable yet */
		line = readline(prompt);
		if (line == 0 || ! strncmp(line,"exit", 4)) {
			terminate(line);
		}

#if USE_GNU_READLINE == 1
		add_history(line);
#endif


#if USE_GUILE == 1
		/* The line is a scheme command */
		if (line[0] == '(') {
			char catchligne[strlen(line) + 256];
			sprintf(catchligne, "(catch #t (lambda () %s) (lambda (key . parameters) (display \"mauvaise expression/bug en scheme\n\")))", line);
			scm_eval_string(scm_from_locale_string(catchligne));
			free(line);
                        continue;
                }
#endif
		
		/* parsecmd free line and set it up to 0 */
		l = parsecmd( & line);

		/* If input stream closed, normal termination */
		if (!l) {
		  
			terminate(0);
		}
		

		
		if (l->err) {
			/* Syntax error, read another command */
			printf("error: %s\n", l->err);
			continue;
		}
		
		if (l->in){
			input = open(l->in, O_RDONLY);
			printf("in: %s\n", l->in);
		}
		if (l->out){ 
			output = open(l->out, O_WRONLY | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
			// == 777
			printf("out: %s\n", l->out);
		}
		if (l->bg) printf("background (&)\n");

		/* Display each command of the pipe */
		for (i=0; l->seq[i]!=0; i++) {
			static int tuyau[2];
			int tuyau_tmp[2];
			memcpy(tuyau_tmp, tuyau, 2*sizeof(int));
			if(l->seq[i+1] != 0) // pas de pipe possible sur la derniere commande
				pipe(tuyau);
			int res;
			char **cmd = l->seq[i];
			if (strcmp(cmd[0], "jobs") == 0){
				// la commande courante est jobs
				jobs(pid_list);
			}else{
				switch (res = fork()){
				case -1:
					perror("fork : ");
					break;
				case 0:					
					// on est dans le fils
					if(i != 0){ // pas la premiere commande
						dup2(tuyau_tmp[0], 0);
						close(tuyau_tmp[0]);
						
					}else{ // 1e commande
						if(input != 0){ //entree autre que stdin
							// redirection sur l'entree
							dup2(input, 0);
						}
					}
					if(l->seq[i+1] != 0){ // pas la derniere commande
						close(tuyau[0]);
						dup2(tuyau[1], 1);
						close(tuyau[1]);
					}else{
						if(output != 1){ //sortie autre que stdout
							// redirection sur la sortie
							dup2(output, 1);
						}
					}
					// chargement de la nouvelle commande
					assert(execvp(cmd[0], cmd) != -1);
					// on ne doit jamais retourner d'un exec
					perror("retour exec, erreur chargement processus fils");
					exit(0);
				default:
				{
					if(l->seq[i+1] != 0){
						close(tuyau[1]);
					}
					if(i != 0){
						close(tuyau_tmp[0]);
					}

					//gestion de l'arrière plan
					// on ne bloque que s'il n'y a pas d'arriere plan
					// et qu'il s'agit de la derniere commande
					if (!l->bg && (l->seq[i+1] == 0)){
						int lock;
						wait(&lock);
						break;
					}					
					// actualisation de la liste des pids
					add_pid_list(pid_list, res, cmd[0]);

				}
				}
			}
		}
	}

}
