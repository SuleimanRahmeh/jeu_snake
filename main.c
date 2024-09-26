#include <SDL.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define TILE_SIZE 20

typedef struct {
    int x, y;
} Segment;

typedef struct {
    Segment segments[100];
    int taille;
    int direction;  // 0: haut, 1: droite, 2: bas, 3: gauche
} Serpent;

typedef struct {
    int x, y;
} Pomme;

bool init(SDL_Window **window, SDL_Renderer **renderer) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Erreur SDL: %s\n", SDL_GetError());
        return false;
    }

    *window = SDL_CreateWindow("Jeu du Serpent", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (*window == NULL) {
        printf("Erreur de cr�ation de fen�tre: %s\n", SDL_GetError());
        SDL_Quit();
        return false;
    }

    *renderer = SDL_CreateRenderer(*window, -1, SDL_RENDERER_ACCELERATED);
    if (*renderer == NULL) {
        printf("Erreur de cr�ation de renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(*window);
        SDL_Quit();
        return false;
    }

    return true;
}

void close(SDL_Window *window, SDL_Renderer *renderer) {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void deplacer_serpent(Serpent *serpent) {
    int i;
    for (i = serpent->taille - 1; i > 0; i--) {
        serpent->segments[i] = serpent->segments[i - 1];
    }

    switch (serpent->direction) {
        case 0: serpent->segments[0].y--; break;  // Haut
        case 1: serpent->segments[0].x++; break;  // Droite
        case 2: serpent->segments[0].y++; break;  // Bas
        case 3: serpent->segments[0].x--; break;  // Gauche
    }
}

bool collision_mur(Serpent *serpent) {
    Segment tete = serpent->segments[0];
    return (tete.x < 0 || tete.x >= SCREEN_WIDTH / TILE_SIZE || tete.y < 0 || tete.y >= SCREEN_HEIGHT / TILE_SIZE);
}

bool collision_queue(Serpent *serpent) {
    int i;
    Segment tete = serpent->segments[0];
    for (i = 1; i < serpent->taille; i++) {
        if (tete.x == serpent->segments[i].x && tete.y == serpent->segments[i].y) {
            return true;
        }
    }
    return false;
}

void generer_pommes(Pomme pommes[3]) {
    int i;
    for (i = 0; i < 3; i++) {
        pommes[i].x = rand() % (SCREEN_WIDTH / TILE_SIZE);
        pommes[i].y = rand() % (SCREEN_HEIGHT / TILE_SIZE);
    }
}

bool manger_pomme(Serpent *serpent, Pomme *pomme) {
    return serpent->segments[0].x == pomme->x && serpent->segments[0].y == pomme->y;
}

void sauvegarder_jeu(Serpent *serpent, Pomme pommes[3], const char *fichier) {
    FILE *fp = fopen(fichier, "w");
    if (fp == NULL) {
        printf("Erreur d'ouverture du fichier de sauvegarde.\n");
        return;
    }

    fprintf(fp, "%d\n", serpent->taille);
    int i;
    for (i = 0; i < serpent->taille; i++) {
        fprintf(fp, "%d %d\n", serpent->segments[i].x, serpent->segments[i].y);
    }

    for (i = 0; i < 3; i++) {
        fprintf(fp, "%d %d\n", pommes[i].x, pommes[i].y);
    }

    fclose(fp);
}

void charger_jeu(Serpent *serpent, Pomme pommes[3], const char *fichier) {
    FILE *fp = fopen(fichier, "r");
    if (fp == NULL) {
        printf("Erreur d'ouverture du fichier de sauvegarde.\n");
        return;
    }

    fscanf(fp, "%d", &serpent->taille);
    int i;
    for (i = 0; i < serpent->taille; i++) {
        fscanf(fp, "%d %d", &serpent->segments[i].x, &serpent->segments[i].y);
    }

    for (i = 0; i < 3; i++) {
        fscanf(fp, "%d %d", &pommes[i].x, &pommes[i].y);
    }

    fclose(fp);
}

int main(int argc, char* argv[]) {
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;
    if (!init(&window, &renderer)) {
        return 1;
    }

    srand(time(NULL));

    Serpent serpent = {{{5, 5}}, 3, 1};
    Pomme pommes[3];
    generer_pommes(pommes);

    bool game_over = false;

    int vitesse = 200;  // Vitesse par d�faut (lent)
    int vitesse_rapide = 100;  // Vitesse ???????

    SDL_Event e;
    bool quit = false;

    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            } else if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                    case SDLK_UP: serpent.direction = 0; break;
                    case SDLK_RIGHT: serpent.direction = 1; break;
                    case SDLK_DOWN: serpent.direction = 2; break;
                    case SDLK_LEFT: serpent.direction = 3; break;
                    case SDLK_s: sauvegarder_jeu(&serpent, pommes, "sauvegarde.txt"); break;
                    case SDLK_l: charger_jeu(&serpent, pommes, "sauvegarde.txt"); break;
                    case SDLK_1: vitesse = 200; break;  // Vitesse lente
                    case SDLK_2: vitesse = 100; break;  // Vitesse rapide
                }
            }
        }

        deplacer_serpent(&serpent);

        if (collision_mur(&serpent) || collision_queue(&serpent)) {
            game_over = true;
        }

        int i;
        for (i = 0; i < 3; i++) {
            if (manger_pomme(&serpent, &pommes[i])) {
                serpent.taille++;
                pommes[i].x = rand() % (SCREEN_WIDTH / TILE_SIZE);
                pommes[i].y = rand() % (SCREEN_HEIGHT / TILE_SIZE);
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        SDL_Rect rect_head = {serpent.segments[0].x * TILE_SIZE, serpent.segments[0].y * TILE_SIZE, TILE_SIZE, TILE_SIZE};
        SDL_RenderFillRect(renderer, &rect_head);

        SDL_SetRenderDrawColor(renderer, 0, 200, 0, 255);
        for (i = 1; i < serpent.taille; i++) {
            SDL_Rect rect = {serpent.segments[i].x * TILE_SIZE, serpent.segments[i].y * TILE_SIZE, TILE_SIZE, TILE_SIZE};
            SDL_RenderFillRect(renderer, &rect);
        }

        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        for (i = 0; i < 3; i++) {
            if (pommes[i].x != -1) {
                SDL_Rect rect = {pommes[i].x * TILE_SIZE, pommes[i].y * TILE_SIZE, TILE_SIZE, TILE_SIZE};
                SDL_RenderFillRect(renderer, &rect);
            }
        }

        SDL_RenderPresent(renderer);

        if (game_over) {
            printf("Game Over!\n");
            break;
        }

        SDL_Delay(vitesse);  // ?????? ?? ???? ?????? ????? ??? ?????? ??????? ?? "vitesse"
    }

    close(window, renderer);
    return 0;
}