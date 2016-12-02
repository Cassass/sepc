#ifndef SYNCHRO_H
#define SYNCHRO_H

#include <stdbool.h>
#include <pthread.h>

#include "ensitheora.h"

extern bool fini;


/* Les extern des variables pour la synchro ici */
// threads video et audio
extern pthread_t theora_reader_thread, vorbis_reader_thread, sdl_thread;
// mutex de protection pour la hasmap
extern pthread_mutex_t hashmap_mutex;
// mutex pour les synchronisations
extern pthread_mutex_t synchro_fenetre, synchro_texture, synchro_conso, synchro_depo;
// conditions pour les synchros
extern pthread_cond_t cond_fenetre, cond_texture, cond_conso, cond_depo;
// booleen pour attente fenetre/texture
extern bool tex_done;
// nombre courant de textures deposees pas affichees
extern int64_t nb_tex; // initialement aucune texture deposee

/* Fonctions de synchro à implanter */

void envoiTailleFenetre(th_ycbcr_buffer buffer);
void attendreTailleFenetre();

void attendreFenetreTexture();
void signalerFenetreEtTexturePrete();

void debutConsommerTexture();
void finConsommerTexture();

void debutDeposerTexture();
void finDeposerTexture();

#endif
