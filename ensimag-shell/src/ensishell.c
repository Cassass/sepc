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
#include <stdbool.h>
#include <glob.h>

#include "jobs_handler.h"
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
void treat_scheme_command(struct cmdline *l, int i, int input, int output){
	int res;
	char **cmd = l->seq[i];
	if (strcmp(cmd[0], "jobs") == 0){
		// la commande courante est jobs
		jobs();
	}else{
		switch (res = fork()){
		case -1:
			perror("fork : ");
			break;
		case 0:					
			// on est dans le fils
			// chargement de la nouvelle commande
			assert(execvp(cmd[0], cmd) != -1);
			// on ne doit jamais retourner d'un exec
			perror("retour exec, erreur chargement processus fils");
			exit(0);
		default:
		{
			//gestion de l'arrière plan
			// on ne bloque que s'il n'y a pas d'arriere plan
			// et qu'il s'agit de la derniere commande
			if (!l->bg && (l->seq[i+1] == 0)){
				int lock;
				wait(&lock);
				break;
			}					
			// actualisation de la liste des pids
			add_pid_list(res, cmd[0]);

		}
		}
	}	
}

void replace_glob(char **cmd){
	glob_t* pglob = malloc(sizeof(glob_t));
	for (int i = 1; cmd[i] != 0; i++){
		glob(cmd[i], GLOB_DOOFFS | (GLOB_APPEND & (i==1)), NULL, pglob);
	}
	
}
void treat_command(struct cmdline *l, int i, int input, int output){
	static int tuyau[2];
	int tuyau_tmp[2];
	memcpy(tuyau_tmp, tuyau, 2*sizeof(int));
	if(l->seq[i+1] != 0) // pas de pipe possible sur la derniere commande
		pipe(tuyau);
	int res;
	char **cmd = l->seq[i];
	if (strcmp(cmd[0], "jobs") == 0){
		// la commande courante est jobs
		jobs();
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
			add_pid_list(res, cmd[0]);

		}
		}
	}
	
}

int executer(char* line, bool is_scheme){
	int input = STDIN_FILENO; //e.g input = 0
	int output = STDOUT_FILENO; //e.g output = 1
	struct cmdline *l;
	int i;
	/* parsecmd free line and set it up to 0 */
	l = parsecmd( & line);

	/* If input stream closed, normal termination */
	if (!l) {
		  
		terminate(0);
	}		
	if (l->err) {
		/* Syntax error, read another command */
		printf("error: %s\n", l->err);
		return 0;
	}
	if (l->in){
		input = open(l->in, O_RDONLY); // si redirection d'entrée, ouverture en lecture
		assert(input != -1);
		printf("in: %s\n", l->in);
	}
	if (l->out){ 
		output = open(l->out, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO);
		assert(output != -1);		
		// == 777
		printf("out: %s\n", l->out);
	}
	if (l->bg) printf("background (&)\n");

	for (i=0; l->seq[i]!=0; i++) {
		if(!is_scheme){
			treat_command(l, i, input, output);
		}else{
			treat_scheme_command(l, i, input, output);
		}
	}

	return 0;
}

int question6_executer(char *line)
{
	/* Question 6: Insert your code to execute the command line
	 * identically to the standard execution scheme:
	 * parsecmd, then fork+execvp, for a single command.
	 * pipe and i/o redirection are not required.
	 */
	executer(line, true);
	
	return 0;
}

SCM executer_wrapper(SCM x)
{
        return scm_from_int(question6_executer(scm_to_locale_stringn(x, 0)));
}
#endif



int main() {
        printf("Variante %d: %s\n", VARIANTE, VARIANTE_STRING);

#if USE_GUILE == 1
        scm_init_guile();
        /* register "executer" function in scheme */
        scm_c_define_gsubr("executer", 1, 0, 0, executer_wrapper);
#endif
	
	while (1) {
		char *line=0;
		char *prompt = "ensishell> ";
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
		executer(line, false); 
		
	}

}
