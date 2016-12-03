
#include "synchro.h"
#include "ensitheora.h"


bool fini;

/* les variables pour la synchro, ici */
// threads video et audio
pthread_t theora_reader_thread, vorbis_reader_thread, sdl_thread;
// mutex de protection pour la hasmap
pthread_mutex_t hashmap_mutex;
// mutex pour les synchronisations
pthread_mutex_t synchro_fenetre;
pthread_mutex_t synchro_texture;
pthread_mutex_t synchro_conso;
// conditions pour les synchros
pthread_cond_t cond_fenetre;
pthread_cond_t cond_texture;
pthread_cond_t cond_conso;
pthread_cond_t cond_depo;
// booleen pour attente fenetre/texture
bool tex_done = false;
// nombre courant de textures deposees pas affichees
int64_t nb_tex = 0; // initialement aucune texture deposee

/* l'implantation des fonctions de synchro ici */

void envoiTailleFenetre(th_ycbcr_buffer buffer) {
    pthread_mutex_lock(&synchro_fenetre);
    windowsx = buffer[0].width; // [0] ? [1] ? [2] ?
    windowsy = buffer[0].height;
    pthread_cond_signal(&cond_fenetre);
    pthread_mutex_unlock(&synchro_fenetre);
}

void attendreTailleFenetre() {
    //on attend l'envoi de la taille de la fenetre
    // protection de la section critique
    pthread_mutex_lock(&synchro_fenetre);
    while (windowsx == 0 || windowsy == 0)
        pthread_cond_wait(&cond_fenetre, &synchro_fenetre);
    pthread_mutex_unlock(&synchro_fenetre);
}

void signalerFenetreEtTexturePrete() {
    pthread_mutex_lock(&synchro_texture);
    tex_done = true;
    pthread_cond_signal(&cond_texture);
    pthread_mutex_lock(&synchro_texture);
    
}

void attendreFenetreTexture() {
    pthread_mutex_lock(&synchro_texture);
    while (!tex_done) 
        pthread_cond_signal(&cond_texture);
    pthread_mutex_lock(&synchro_texture);    
}

// ici verif que l'on peut consommer les textures
void debutConsommerTexture() {
    pthread_mutex_lock(&synchro_conso);
    while (nb_tex == 0) {
        pthread_cond_wait(&cond_conso, &synchro_conso);
    }
    pthread_mutex_unlock(&synchro_conso);
}

// consommation
void finConsommerTexture() {
    pthread_mutex_lock(&synchro_conso);
    --nb_tex;
    pthread_cond_broadcast(&cond_depo);
    pthread_mutex_unlock(&synchro_conso);
}

// verification possibilite depose
void debutDeposerTexture() {
    pthread_mutex_lock(&synchro_conso);
    while (nb_tex == NBTEX) {
        pthread_cond_wait(&cond_depo, &synchro_conso);
    }
    pthread_mutex_unlock(&synchro_conso);

}

//  depot
void finDeposerTexture() {
    pthread_mutex_lock(&synchro_conso);
    ++nb_tex;
    pthread_cond_broadcast(&cond_conso);
    pthread_mutex_unlock(&synchro_conso);
}
