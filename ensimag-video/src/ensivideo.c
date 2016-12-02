#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <SDL2/SDL.h>
#include <pthread.h>
#include <errno.h>

#include "synchro.h"
#include "stream_common.h"
#include "oggstream.h"

#define handle_error_en(en, msg) \
    do{ errno = en; perror(msg); exit(EXIT_FAILURE);} while (0)

int main(int argc, char *argv[]) {
    int res;

    if (argc != 2) {
	fprintf(stderr, "Usage: %s FILE", argv[0]);
	exit(EXIT_FAILURE);
    }
    assert(argc == 2);


    // Initialisation de la SDL
    res = SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_EVENTS);
    atexit(SDL_Quit);
    assert(res == 0);
    
    // start the two stream readers
    int check_theora;
    int check_vorbis;
    check_theora = pthread_create(&theora_reader_thread, NULL, theoraStreamReader, &argv[1]);
    if (check_theora != 0)
        handle_error_en(check_theora, "pthread_create");

    check_vorbis = pthread_create(&vorbis_reader_thread, NULL, vorbisStreamReader, &argv[1]);
    if (check_vorbis != 0)
        handle_error_en(check_vorbis, "pthread_create");
        
    // wait audio thread
    void* aud_status;
    check_vorbis = pthread_join(vorbis_reader_thread, &aud_status);
    if (check_vorbis != 0)
        handle_error_en(check_vorbis, "pthread_join");        
    
    printf("thread [%lu, vorbis] achevé\n", vorbis_reader_thread);
    // 1 seconde de garde pour le son,
    sleep(1);

    // tuer les deux threads videos si ils sont bloqués
    void* vid_theora_status;
    void* sdl_status;
    int check_sdl;
    check_theora = pthread_cancel(theora_reader_thread);
    if (check_theora != 0) {
        handle_error_en(check_theora, "pthread_cancel");
    }
    
    check_sdl = pthread_cancel(sdl_thread);
    if (check_sdl != 0)
        handle_error_en(check_sdl, "pthread_cancel");        
    
    // attendre les 2 threads videos
    check_theora = pthread_join(theora_reader_thread, &vid_theora_status);
    if (check_theora != 0)
        handle_error_en(check_theora, "pthread_join");        
    
    pthread_join(sdl_thread, &sdl_status);
    if (check_sdl != 0) 
        handle_error_en(check_sdl, "pthread_join");        

    if (vid_theora_status == PTHREAD_CANCELED)
        printf("thread [%lu , theora] was cancelled\n", theora_reader_thread);
    else 
        printf("thread [%lu , theora] was not cancelled !!\n", theora_reader_thread);

    if (sdl_status == PTHREAD_CANCELED)
        printf("thread [%lu , sdl] was cancelled\n", sdl_thread);
    else
        printf("thread [%lu , sdl] was not cancelled !!\n", sdl_thread);
    exit(EXIT_SUCCESS);    
}
