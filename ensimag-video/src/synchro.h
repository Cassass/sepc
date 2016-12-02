#ifndef SYNCHRO_H
#define SYNCHRO_H

#include <stdbool.h>
#include <pthread.h>

#include "ensitheora.h"

extern bool fini;


/* Les extern des variables pour la synchro ici */
extern pthread_t theora_reader_thread, vorbis_reader_thread, sdl_thread;
extern pthread_mutex_t hashmap_mutex;
extern pthread_mutex_t synchro_fenetre, synchro_texture;
extern pthread_cond_t cond_fenetre;
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
