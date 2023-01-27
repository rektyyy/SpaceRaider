#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 800;
const int PLAYER_WIDTH = SCREEN_WIDTH / 12;
const int PLAYER_HEIGHT = SCREEN_HEIGHT / 12;
const int PLAYER_SPEED = 2;
const int BULLET_WIDTH = 10;
const int BULLET_HEIGHT = 10;
const int BULLET_SPEED = 4;
const int BULLET_COOLDOWN = 2;
const int PACKAGE_SPEED = 2;
/*------------------------------------------FUNCTIONS------------------------------------------*/

//Starts up SDL and creates window
bool init();

//Starts up ttf library
bool ttfInit();

//Starts up mixer library and play background music
bool mixer_init();

//Loads media
bool loadMedia();

//Frees media and shuts down SDL
void close();

//Loads individual image
SDL_Texture* loadTexture(char* path);

//Returns rectangle with player movement and creates bullets
SDL_FRect keyboardCheck(SDL_FRect player);

//Creates an asteroid
void createAsteoid();

//Creates a bullet
void createBullet(SDL_FRect player);

//Creates a package with bullets
void createPackage();

//converts SDL_Rect to SDL_FRect
SDL_Rect convert(SDL_FRect frect);

//Checks all asteroids for collision
bool collisionCheckAsteroid(SDL_FRect player);

//Checks all bullets for collision
bool collisionCheckBullet(int i);

//Checks all packages for collision
bool collisionCheckPackage(SDL_FRect player);

//Renders everything
void render(SDL_FRect player);

//rendering asteroids
void asteroids_render();

//Renders scoreboard and text
void render_scoreboard();

//Game over screen
void gameOver();

//Lowers asteroids, packages and makes bullets go up
void asteroidBulletAndPackageMovement();

//Counts score (+1 per dodged asteroid)
void getScore(SDL_FRect player);

//needed
bool gameLoop(SDL_Event e, SDL_FRect *player_pointer);

//comment
void menu_render(SDL_Event e);

//comment
void option_render(SDL_Event e);

/*------------------------------------------GLOBAL VARIABLES------------------------------------------*/

// Pointers to our window, renderer, texture, music, and sound
SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;
SDL_Texture* gTexturePlayer = NULL;
SDL_Texture* gTextureAsteroid1 = NULL;
SDL_Texture* gTextureAsteroid2 = NULL;
SDL_Texture* gTextureAsteroid3 = NULL;
SDL_Texture* gTextureAsteroid4 = NULL;
SDL_Texture* gTextureAsteroid5 = NULL;
SDL_Texture* gTextureAsteroid6 = NULL;
SDL_Texture* gTextureAsteroid7 = NULL;
SDL_Texture* gTextureAsteroid8 = NULL;
SDL_Texture* gTextureAsteroid9 = NULL;
SDL_Texture* textureList[9];
SDL_Texture* gTexturePackage = NULL;
SDL_Texture* gTextureBullet = NULL;
Mix_Music* menu;
Mix_Music* game;
Mix_Music* game_over;
Mix_Chunk* sound;
TTF_Font* font;

//Counter of asteroids
int asteroids_count = 0;

//Counter of bullets
int bullets_count = 0;
int bullets_available = 10;

//Counter of packages
int packages_count = 0;

//Quantity of asteroids
#define asteroids_quantity 200

//Quantity of bullets
#define bullets_quantity 20

//Quantity of packages
#define package_quantity 5

//All asteroids to render
struct asteroids
{
    SDL_FRect dim;
    SDL_Texture* texture;
    float speed;
    bool visible;
    int HP;
    bool is_hit;
    double rotation;
    double angle;
}all_asteroids[asteroids_quantity];

//All bullets to render
SDL_Rect all_bullets[bullets_quantity];

//All bullets to render
SDL_Rect all_packages[package_quantity];

//Locks shot speed
int between_shots = 0;

//Time
unsigned int lastTime = 0, currentTime, prevtime = 0, menuTime;;

//Score
int currentScore = 0;

//level of difficulty
int difficulty = 0;

//angle of rotation
double default_angle = 0;

/*------------------------------------------FUNCTIONS CODE------------------------------------------*/
bool init()
{
    bool success = true;
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
        success = false;
    }
    else
    {
        //Set texture filtering to linear
        if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
        {
            printf("Warning: Linear texture filtering not enabled!");
        }

        gWindow = SDL_CreateWindow("Space Raider", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
        if (gWindow == NULL)
        {
            printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
            success = false;
        }
        else
        {
            gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
            if (gRenderer == NULL)
            {
                printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
                success = false;
            }
            else
            {
                SDL_SetRenderDrawColor(gRenderer, 96, 128, 255, 255);
            }
        }
    }

    success = mixer_init();
    success = ttfInit();

    return success;
}

bool ttfInit()
{
    // Initialize SDL_ttf
	if ( TTF_Init() < 0 )
	{
        printf("Error intializing SDL_ttf: %s\n", TTF_GetError());
        return false;
	}

	// Load font
	font = TTF_OpenFont("font.ttf", 48);
	if ( !font )
	{
        printf("Error loading font: %s", TTF_GetError());
        return false;
	}

	// Start sending SDL_TextInput events
	SDL_StartTextInput();

	return true;
}

bool mixer_init()
{
    // Initialize SDL_mixer
    int result = Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 1024 );
    if ( result != 0 )
    {
        printf("Music could not be created! SDL Error: %s\n", SDL_GetError());
        return false;
    }

	// Load music and play music forever if loaded
	game = Mix_LoadMUS("sounds/game.mp3");
	game_over = Mix_LoadMUS("sounds/game_over.mp3");
	menu = Mix_LoadMUS("sounds/menu.mp3");
	if ( !game || !game_over || !menu)
    {
        printf("Failed to load music: %s\n", Mix_GetError());
        return false;
	}

	// Load sound
	sound = Mix_LoadWAV("sounds/lasergun.mp3");
	if ( !sound )
    {
        printf("Failed to load sound: %s\n", Mix_GetError());
        return false;
	}

    return true;
}

SDL_Texture* loadTexture(char* path)
{
    SDL_Texture* newTexture = NULL;
    SDL_Surface* loadedSurface = SDL_LoadBMP(path);

    //Make transparent background for textures
    SDL_SetColorKey(loadedSurface, SDL_TRUE, SDL_MapRGB(loadedSurface->format, 255, 255, 255));
    if (loadedSurface == NULL)
    {
        printf("Unable to load image %s! SDL Error: %s\n", path, SDL_GetError());
    }
    else
    {
        newTexture = SDL_CreateTextureFromSurface(gRenderer, loadedSurface);
        if (newTexture == NULL)
        {
            printf("Unable to create a texture from %s! SDL Error: %s\n", path, SDL_GetError());
        }
        SDL_FreeSurface(loadedSurface);
    }
    return newTexture;
}

bool loadMedia()
{
    //Loads all textures
    bool success = true;
    gTexturePlayer = loadTexture("images/statek.bmp");
    gTextureAsteroid1 = loadTexture("images/asteroida1.bmp");
    gTextureAsteroid2 = loadTexture("images/asteroida2.bmp");
    gTextureAsteroid3 = loadTexture("images/asteroida3.bmp");
    gTextureAsteroid4 = loadTexture("images/asteroida4.bmp");
    gTextureAsteroid5 = loadTexture("images/asteroida5.bmp");
    gTextureAsteroid6 = loadTexture("images/asteroida6.bmp");
    gTextureAsteroid7 = loadTexture("images/asteroida7.bmp");
    gTextureAsteroid8 = loadTexture("images/asteroida8.bmp");
    gTextureAsteroid9 = loadTexture("images/asteroida9.bmp");
    gTexturePackage = loadTexture("images/package.bmp");
    gTextureBullet = loadTexture("images/bullet.bmp");

    if (gTexturePlayer == NULL || gTextureAsteroid1 == NULL || gTextureAsteroid2 == NULL ||
        gTextureAsteroid3 == NULL || gTextureAsteroid4 == NULL || gTextureAsteroid5 == NULL ||
        gTextureAsteroid6 == NULL || gTextureAsteroid7 == NULL || gTextureAsteroid8 == NULL || gTextureAsteroid9 == NULL)
    {
        printf("Failed to load texture image!\n");
        success = false;
    }
    //Creates list of textures
    textureList[0] = gTextureAsteroid1;
    textureList[1] = gTextureAsteroid2;
    textureList[2] = gTextureAsteroid3;
    textureList[3] = gTextureAsteroid4;
    textureList[4] = gTextureAsteroid5;
    textureList[5] = gTextureAsteroid6;
    textureList[6] = gTextureAsteroid7;
    textureList[7] = gTextureAsteroid8;
    textureList[8] = gTextureAsteroid9;
    return success;
}

void close()
{
    //Destroy music and sound
    Mix_FreeMusic( game );
    Mix_FreeMusic( game_over );
	Mix_FreeChunk( sound );

    //Destroy texture
    SDL_DestroyTexture(gTexturePlayer);
    SDL_DestroyTexture(gTextureAsteroid1);
    SDL_DestroyTexture(gTextureAsteroid2);
    SDL_DestroyTexture(gTextureAsteroid3);
    gTexturePlayer = NULL;
    gTextureAsteroid1 = NULL;
    gTextureAsteroid2 = NULL;
    gTextureAsteroid3 = NULL;

    //Destroy window
    SDL_DestroyRenderer(gRenderer);
    SDL_DestroyWindow(gWindow);
    gRenderer = NULL;
    gWindow = NULL;

    //Quit SDL subsystems
    TTF_Quit();
    Mix_Quit();
    SDL_Quit();
}

SDL_FRect keyboardCheck(SDL_FRect player)
{
    //Check for user input and move player/close program

    const Uint8* keyboardstate = SDL_GetKeyboardState(NULL);
    float speed = PLAYER_SPEED;
    if((keyboardstate[SDL_SCANCODE_UP]|| keyboardstate[SDL_SCANCODE_DOWN]) && (keyboardstate[SDL_SCANCODE_RIGHT] || keyboardstate[SDL_SCANCODE_LEFT])) speed = sqrt(speed);
    //First condition checks input && second condition keeps player in the playable area
    if (keyboardstate[SDL_SCANCODE_UP] && player.y > SCREEN_HEIGHT / 2) player.y -= speed;
    if (keyboardstate[SDL_SCANCODE_DOWN] && player.y + PLAYER_HEIGHT <= SCREEN_HEIGHT) player.y += speed;
    if (keyboardstate[SDL_SCANCODE_LEFT] && player.x > 0) player.x -= speed;
    if (keyboardstate[SDL_SCANCODE_RIGHT] && player.x + PLAYER_WIDTH <= SCREEN_WIDTH) player.x += speed;
    if (keyboardstate[SDL_SCANCODE_SPACE] && between_shots == 0 && bullets_available != 0)
    {
        createBullet(player);
        between_shots = BULLET_COOLDOWN;
        Mix_PlayChannel( -1, sound, 0 );
    }

    return player;
}

void createAsteoid()
{
    // Create asteroid(rectangle) with random parameters and add them to global array
    int xy = rand() % 70 + 30;
    SDL_FRect asteroid = { rand() % 1000, -rand() % 100 - 100, xy, xy};
    all_asteroids[asteroids_count].dim = asteroid;
    all_asteroids[asteroids_count].speed = (rand()%501 + 900)/1000.0;
    all_asteroids[asteroids_count].visible = true;
    all_asteroids[asteroids_count].texture = textureList[rand() % 9];
    if(xy > 70) all_asteroids[asteroids_count].HP = 2;
    else all_asteroids[asteroids_count].HP = 1;
    all_asteroids[asteroids_count].is_hit=false;
    all_asteroids[asteroids_count].angle = rand()%360 + 1;
    all_asteroids[asteroids_count].rotation = (rand()%40 + 1)/100.0;
    if(asteroids_count == asteroids_quantity / 20)   createPackage();
    asteroids_count++;
    asteroids_count %= asteroids_quantity;
}

void createBullet(SDL_FRect player)
{
    // Create bullet(square) and add them to global array

    SDL_Rect bullet = { (player.x + PLAYER_WIDTH / 2 - 5), player.y, BULLET_WIDTH, BULLET_HEIGHT };
    all_bullets[bullets_count] = bullet;
    bullets_count++;
    bullets_count %= bullets_quantity;
    bullets_available--;
}

void createPackage()
{
    SDL_Rect package = {rand() % SCREEN_WIDTH, -rand() % 100 - 100, 75, 75};
    all_packages[packages_count] = package;
    packages_count++;
    packages_count %= package_quantity;
}

void render(SDL_FRect player)
{
    //Clears screen
    SDL_SetRenderDrawColor(gRenderer, 96, 128, 255, 255);
    SDL_RenderClear(gRenderer);

    //render asteroid
    asteroids_render();

    //render bullets
    for(int i = 0; i < bullets_quantity; i++)
    {
        SDL_RenderCopy(gRenderer, gTextureBullet, NULL, &all_bullets[i]);
    }

    //render packages
    for(int i = 0; i < package_quantity; i++)
    {
        SDL_RenderCopy(gRenderer, gTexturePackage, NULL, &all_packages[i]);
    }

    render_scoreboard();

    //Render texture to screen
    SDL_RenderCopyF(gRenderer, gTexturePlayer, NULL, &player);

    SDL_SetRenderDrawColor(gRenderer, 255, 0, 0, 128);
    SDL_RenderDrawLine(gRenderer, 0, SCREEN_HEIGHT / 2, SCREEN_WIDTH, SCREEN_HEIGHT / 2);

    //Update screen
    SDL_RenderPresent(gRenderer);
}

void asteroids_render()
{
    SDL_RendererFlip flip = SDL_FLIP_HORIZONTAL | SDL_FLIP_VERTICAL;

    if ((currentTime - prevtime) > 300 - 50*difficulty - (currentTime-menuTime)/1000 )
    {
        createAsteoid();
        prevtime = currentTime;
    }

    for (int i = 0; i < asteroids_quantity; i++)
    {
        if(all_asteroids[i].is_hit == true) SDL_SetTextureAlphaMod(all_asteroids[i].texture,170);
        SDL_RenderCopyExF(gRenderer,all_asteroids[i].texture, NULL, &all_asteroids[i].dim , default_angle + all_asteroids[i].angle ,NULL, flip);
        SDL_SetTextureAlphaMod(all_asteroids[i].texture,255);
        all_asteroids[i].angle+=all_asteroids[i].rotation;
    }
    default_angle+=0.1;
}

void render_scoreboard()
{
    SDL_Surface* text;
    // Set color to white
    SDL_Color color = {255, 255, 255, 255};

    char* str = (char*)malloc(50*sizeof(char));

    if(sprintf(str,"Time: %d   Score: %d   Bullets: %d", (currentTime-menuTime)/1000, currentScore, bullets_available)<0)
        str="Failed to load text";

    text = TTF_RenderText_Solid( font, str, color );
    if ( !text )
    {
        printf("Failed to render text: %s", TTF_GetError());
    }

    SDL_Texture* text_texture;
    text_texture = SDL_CreateTextureFromSurface( gRenderer, text );
    SDL_Rect dest = { 0, 0, text->w, text->h };

    //render black rectangle
    SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 255);
    SDL_Rect scoreboard={0,0,SCREEN_WIDTH, text->h};
    SDL_RenderFillRect(gRenderer, &scoreboard);

    //render text
    SDL_RenderCopy( gRenderer, text_texture, NULL, &dest );
    free(str);
    SDL_FreeSurface(text);
    SDL_DestroyTexture(text_texture);
}

SDL_Rect convert(SDL_FRect frect)
{
    SDL_Rect rect;

    rect.h = (int)frect.h;
    rect.w = (int)frect.w;
    rect.x = (int)frect.x;
    rect.y = (int)frect.y;
    return rect;
}

bool collisionCheckAsteroid(SDL_FRect player)
{
    SDL_Rect player_rect = convert(player);
    //Use SDL_HasIntersection to see if asteroid collides with player
    for (int i = 0; i < asteroids_quantity; i++)
    {
        SDL_Rect rect = convert(all_asteroids[i].dim);

        if (SDL_HasIntersection(&player_rect, &rect))
        {
            return false;
        }
        if (collisionCheckBullet(i) == false  && all_asteroids[i].HP == 0)
        {
            //If asteroid collides with bullet, change asteroid size to 0 and change position to player's y to get a point
            all_asteroids[i].dim.h = 0;
            all_asteroids[i].dim.w = 0;
            all_asteroids[i].dim.x = 0;
            all_asteroids[i].dim.y = player.y-1;
        }
    }

    return true;
}

bool collisionCheckBullet(int j)
{
    SDL_Rect rect = convert(all_asteroids[j].dim);
    //Use SDL_HasIntersection to see if bullet collides with asteroid and delete them if so
    for (int i = 0; i < bullets_quantity; i++)
    {
        if (SDL_HasIntersection(&rect, &all_bullets[i]))
        {
            all_asteroids[j].HP--;
            all_asteroids[j].is_hit = true;
            all_bullets[i].h = 0;
            all_bullets[i].w = 0;
            all_bullets[i].x = 0;
            all_bullets[i].y = 0;
            return false;
        }
    }
    return true;
}

bool collisionCheckPackage(SDL_FRect player)
{
    SDL_Rect player_rect = convert(player);
    for(int i = 0; i < packages_count; i++)
    {
        if(SDL_HasIntersection(&player_rect, &all_packages[i]))
        {
            all_packages[i].w = 0;
            all_packages[i].h = 0;
            all_packages[i].x = 0;
            all_packages[i].y = 0;
            return false;
        }
    }
    return true;
}

void gameOver(SDL_Event e)
{
    //Create a game over screen(I have to add some text to it)
    SDL_Rect game_over_screen = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
    SDL_SetRenderDrawColor(gRenderer, 255, 0, 0, 255);
    SDL_RenderFillRect(gRenderer, &game_over_screen);
    Mix_PlayMusic( game_over, 0 );

    SDL_Surface* text1;
    SDL_Surface* text2;
    // Set color to white
    SDL_Color color = {0, 0, 0, 255};
    prevtime=currentTime;
    while(currentTime - prevtime < 4000)
    {
        SDL_RenderClear(gRenderer);
        currentTime = SDL_GetTicks();
        SDL_PollEvent(&e);
        char* str = (char*)malloc(50*sizeof(char));

        if(sprintf(str,"Time: %d   Score: %d", (prevtime-menuTime)/1000, currentScore)<0)
            str="Failed to load text";

        text1 = TTF_RenderText_Solid( font, "GAME OVER", color );
        text2 = TTF_RenderText_Solid( font, str, color );

        if ( !text1 || !text2)
        {
            printf("Failed to render text: %s", TTF_GetError());
        }

        SDL_Texture* text_texture1;
        SDL_Texture* text_texture2;
        text_texture1 = SDL_CreateTextureFromSurface( gRenderer, text1 );
        text_texture2 = SDL_CreateTextureFromSurface( gRenderer, text2 );
        SDL_Rect dest1 = {(SCREEN_WIDTH - text1->w)/2, SCREEN_HEIGHT/2, text1->w, text1->h };
        SDL_Rect dest2 = {(SCREEN_WIDTH - text2->w)/2 , SCREEN_HEIGHT/2 + text1->h, text2->w, text2->h };

        //render text
        SDL_RenderCopy( gRenderer, text_texture1, NULL, &dest1 );
        SDL_RenderCopy( gRenderer, text_texture2, NULL, &dest2 );

        SDL_RenderPresent(gRenderer);


        free(str);
        SDL_FreeSurface(text1);
        SDL_FreeSurface(text2);
        SDL_DestroyTexture(text_texture1);
        SDL_DestroyTexture(text_texture2);

        if(e.type == SDL_QUIT) close();
    }
}

void asteroidBulletAndPackageMovement()
{
    for(int i = 0; i < package_quantity; i++)
    {
        all_packages[i].y += PACKAGE_SPEED;
    }
    for(int i = 0; i < asteroids_quantity; i++)
    {
        all_asteroids[i].dim.y += (all_asteroids[i].speed + ((float)(currentTime-menuTime))/20000.0);
    }
    for(int i = 0; i < bullets_quantity; i++)
    {
        all_bullets[i].y -= BULLET_SPEED;
    }
}

void getScore(SDL_FRect player)
{
    //Looks at all asteroids and if player is higher than asteroid then it adds one to the score
    for (int i = 0; i < asteroids_quantity; i++)
    {
        if (player.y < all_asteroids[i].dim.y && all_asteroids[i].visible == true)
        {
            currentScore++;
            all_asteroids[i].visible = false;
        }
    }
}

bool gameLoop(SDL_Event e, SDL_FRect *player_pointer)
{
    SDL_FRect player = *player_pointer;
    //Handle events on queue
    SDL_PollEvent(&e);

    //User requests quit
    if (e.type == SDL_QUIT) return false;
    if(SDL_GetKeyboardState(NULL)[SDL_SCANCODE_ESCAPE]) return false;

    //player movement
    if (e.type != SDL_MOUSEMOTION)
    {
        player = keyboardCheck(player);

        asteroidBulletAndPackageMovement();

        if (collisionCheckAsteroid(player))
        {
            render(player);
            if(!collisionCheckPackage(player))
            {
                bullets_available += 10;
            }
        }
        else
        {
            gameOver(e);
            return 0;
        }
    }

    //show score
    getScore(player);

    //show time
    currentTime = SDL_GetTicks();
    if (currentTime > lastTime + 1000)
    {
        lastTime = currentTime;

        if (between_shots != 0) between_shots--;
    }

    *player_pointer=player;
    return true;
}

void menu_render(SDL_Event e)
{
    bool quit = true;

    Mix_PlayMusic( menu, -1 );
    //Create a menu screen
    SDL_Rect background = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };

    // Set color to white
    SDL_Color color = {0, 0, 0, 255};

    int choice_number=0;
    char* choice[3];
    char* str[3];
    char arrow[3] = "->";

    SDL_Surface* text[3];
    SDL_Texture* text_texture[3];

    while(quit)
    {
        //render background
        SDL_RenderClear(gRenderer);
        SDL_SetRenderDrawColor(gRenderer, 108, 255, 235, 0);
        SDL_RenderFillRect(gRenderer, &background);

        asteroids_render();
        asteroidBulletAndPackageMovement();

        currentTime = SDL_GetTicks();

        //Handle events on queue
        SDL_PollEvent(&e);

        choice[0] = (char*)malloc(3*sizeof(char));
        choice[1] = (char*)malloc(3*sizeof(char));
        choice[2] = (char*)malloc(3*sizeof(char));

        str[0] = (char*)malloc(10*sizeof(char));
        str[1] = (char*)malloc(10*sizeof(char));
        str[2] = (char*)malloc(10*sizeof(char));

        //keyboard check
        if(e.type == SDL_QUIT) quit=false;
        if(e.type == SDL_KEYDOWN)
        switch (e.key.keysym.sym)
        {
            case SDLK_UP: choice_number+=2;
                    break;
            case SDLK_DOWN: choice_number++;
                    break;
            case SDLK_ESCAPE: quit = false;
                    break;
            case SDLK_RETURN:
                if(choice_number == 0) return;
                else if(choice_number == 1) option_render(e);
                else if(choice_number == 2) quit = false;
                    break;
            default:;
        }

        choice_number%=3;

        sprintf(choice[0],"  ");
        sprintf(choice[1],"  ");
        sprintf(choice[2],"  ");

        if(choice_number == 0) sprintf(choice[0],"%s",arrow);
        else if(choice_number == 1) sprintf(choice[1],"%s",arrow);
        else if(choice_number == 2) sprintf(choice[2],"%s",arrow);

        sprintf(str[0],"%s START", choice[0]);
        sprintf(str[1],"%s OPTIONS", choice[1]);
        sprintf(str[2],"%s QUIT", choice[2]);

        for(int i=0; i <3; i++)
        {
            if(choice_number==i) color.r=255;
            text[i] = TTF_RenderText_Solid( font, str[i], color );
            color.r=0;
        }

        if ( !text[0] || !text[2] || !text[1])printf("Failed to render text: %s", TTF_GetError());

        for(int i=0; i <3; i++) text_texture[i] = SDL_CreateTextureFromSurface( gRenderer, text[i] );

        SDL_Rect dest1 = {(SCREEN_WIDTH-200)/2, SCREEN_HEIGHT/2 - text[1]->h, text[0]->w, text[0]->h };
        SDL_Rect dest2 = {(SCREEN_WIDTH-200)/2, SCREEN_HEIGHT/2 , text[1]->w, text[1]->h };
        SDL_Rect dest3 = {(SCREEN_WIDTH-200)/2, SCREEN_HEIGHT/2 + text[1]->h, text[2]->w, text[2]->h };

        //render text
        SDL_RenderCopy( gRenderer, text_texture[0], NULL, &dest1 );
        SDL_RenderCopy( gRenderer, text_texture[1], NULL, &dest2 );
        SDL_RenderCopy( gRenderer, text_texture[2], NULL, &dest3 );

        SDL_RenderPresent(gRenderer);

        //free the memory
        for(int i=0; i<3; i++)
        {
            SDL_FreeSurface(text[i]);
            SDL_DestroyTexture(text_texture[i]);
            free(str[i]);
            free(choice[i]);
        }

        SDL_Delay(10);
    }

    close();
}

void option_render(SDL_Event e)
{
    //Create a menu screen
    SDL_Rect background = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };

    // Set color to white
    SDL_Color color = {0, 0, 0, 255};

    int choice_number=0;
    char* choice[4];
    char* str[4];
    char arrow[3] = "->";

    SDL_Surface* text[4];
    SDL_Texture* text_texture[4];

    while(true)
    {
        //render background
        SDL_RenderClear(gRenderer);
        SDL_SetRenderDrawColor(gRenderer, 108, 255, 235, 0);
        SDL_RenderFillRect(gRenderer, &background);

        asteroids_render();
        asteroidBulletAndPackageMovement();

        currentTime = SDL_GetTicks();

        //Handle events on queue
        SDL_PollEvent(&e);

        choice[0] = (char*)malloc(3*sizeof(char));
        choice[1] = (char*)malloc(3*sizeof(char));
        choice[2] = (char*)malloc(3*sizeof(char));
        choice[3] = (char*)malloc(3*sizeof(char));

        str[0] = (char*)malloc(10*sizeof(char));
        str[1] = (char*)malloc(10*sizeof(char));
        str[2] = (char*)malloc(10*sizeof(char));
        str[3] = (char*)malloc(10*sizeof(char));

        //keyboard check
        if (e.type == SDL_QUIT) goto quit;
        if(e.type == SDL_KEYDOWN)
        switch (e.key.keysym.sym)
        {
            case SDLK_UP: choice_number+=3;
                    break;
            case SDLK_DOWN: choice_number++;
                    break;
            case SDLK_ESCAPE: return;
            case SDLK_RETURN:
                if(choice_number == 0)
                {
                    difficulty = 0;
                }
                else if(choice_number == 1)
                {
                    difficulty = 1;
                }
                else if(choice_number == 2)
                {
                    difficulty = 2;
                }
                else if(choice_number == 3) return;
                    break;
            default:;
        }

        choice_number%=4;

        sprintf(choice[0],"  ");
        sprintf(choice[1],"  ");
        sprintf(choice[2],"  ");
        sprintf(choice[3],"  ");

        if(choice_number == 0) sprintf(choice[0],"%s",arrow);
        else if(choice_number == 1) sprintf(choice[1],"%s",arrow);
        else if(choice_number == 2) sprintf(choice[2],"%s",arrow);
        else if(choice_number == 3) sprintf(choice[3],"%s",arrow);

        sprintf(str[0],"%s DIFFICULTY 1", choice[0]);
        sprintf(str[1],"%s DIFFICULTY 2", choice[1]);
        sprintf(str[2],"%s DIFFICULTY 3", choice[2]);
        sprintf(str[3],"%s BACK", choice[3]);

        for(int i=0; i <4; i++)
        {
            if(choice_number == i) color.r=255;
            if(difficulty == i) color.b=255;
            text[i] = TTF_RenderText_Solid( font, str[i], color );
            color.r=0;
            color.b=0;
        }

        if ( !text[0] || !text[2] || !text[1] || !text[3])printf("Failed to render text: %s", TTF_GetError());

        for(int i=0; i <4; i++) text_texture[i] = SDL_CreateTextureFromSurface( gRenderer, text[i] );

        SDL_Rect dest1 = {(SCREEN_WIDTH-200)/2, SCREEN_HEIGHT/2 - text[1]->h, text[0]->w, text[0]->h };
        SDL_Rect dest2 = {(SCREEN_WIDTH-200)/2, SCREEN_HEIGHT/2 , text[1]->w, text[1]->h };
        SDL_Rect dest3 = {(SCREEN_WIDTH-200)/2, SCREEN_HEIGHT/2 + text[1]->h, text[2]->w, text[2]->h };
        SDL_Rect dest4 = {(SCREEN_WIDTH-200)/2, SCREEN_HEIGHT/2 + text[1]->h + text[2]->h, text[3]->w, text[3]->h };

        //render text
        SDL_RenderCopy( gRenderer, text_texture[0], NULL, &dest1 );
        SDL_RenderCopy( gRenderer, text_texture[1], NULL, &dest2 );
        SDL_RenderCopy( gRenderer, text_texture[2], NULL, &dest3 );
        SDL_RenderCopy( gRenderer, text_texture[3], NULL, &dest4 );

        SDL_RenderPresent(gRenderer);

        //free the memory
        for(int i=0; i<4; i++)
        {
            SDL_FreeSurface(text[i]);
            SDL_DestroyTexture(text_texture[i]);
            free(str[i]);
            free(choice[i]);
        }

        SDL_Delay(10);
    }

    quit:
    for(int i=0; i<4; i++)
    {
        SDL_FreeSurface(text[i]);
        SDL_DestroyTexture(text_texture[i]);
        free(str[i]);
        free(choice[i]);
    }
    close();
}
/*------------------------------------------MAIN------------------------------------------*/

int main(int argc, char* argv[])
{
    //Initialize srand
    srand((unsigned int)time(NULL));

    //Start up SDL and create window
    if (!init())
    {
        printf("Failed to initialize!\n");
    }
    else
    {
        //Load media
        if (!loadMedia())
        {
            printf("Failed to load media!\n");
        }
        else
        {
            while(true)
            {
                 //sets every asteroid as invisible
                for(int i = 0; i < asteroids_quantity; i++)
                {
                    all_asteroids[i].visible = false;
                    all_asteroids[i].dim.y = SCREEN_HEIGHT;
                }

                bullets_available = 10;
                currentScore = 0;
                //Event handler
                SDL_Event e;

                //rendering menu
                menu_render(e);
                menuTime = SDL_GetTicks();


                //sets every asteroid as invisible
                for(int i = 0; i < asteroids_quantity; i++)
                {
                    all_asteroids[i].visible = false;
                    all_asteroids[i].dim.y = SCREEN_HEIGHT;
                }

                for(int i = 0; i < package_quantity; i++)
                {
                    all_packages[i].y = SCREEN_HEIGHT;
                }

                //Player model
                SDL_FRect player = { SCREEN_WIDTH / 2, SCREEN_HEIGHT - 100, PLAYER_WIDTH, PLAYER_HEIGHT };

                //While application is running
                Mix_PlayMusic( game, -1 );
                while (gameLoop(e, &player))
                {
                    // Wait before next frame
                    SDL_Delay(10 - 3*difficulty);
                }
            }
        }
    }

    return 0;
}
