#include <stdio.h>
#include "SDL.h"
#include "./constant.h"
#include "SDL_image.h"
#include "SDL_main.h"
#include <math.h>
#include <stdlib.h>
#include <time.h>


int game_is_running = FALSE;
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
SDL_Texture* MC_texture = NULL;
SDL_Texture* map_texture = NULL;
SDL_Texture* Enemy_texture[2];

//Time
int last_frame_time = 0;
float  delta_time;
const Uint32 periodicInterval = 60000;
Uint32 lastPeriodicCall = 0;

//MC
int move_up = FALSE;
int move_down = FALSE;
int move_left = FALSE;
int move_right = FALSE;
int facing_left = 0; // Add this global variable
int map[MAP_HEIGHT][MAP_WIDTH];


struct Camera {
	float x, y;
	int width, height;
} camera = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };


struct character {
	float x;
	float y;
	float width;
	float height;
	float movement_speed; // Pixels per second, adjust as needed
	float atk;
	float health;
	SDL_Texture* texture;
	int is_active; // 1 for active, 0 for inactive
	int type;
} MC;



struct spawn_pos {
	float x;
	float y;
}spawn_position[8] = {
	{ -200, -200},
	{ 1920, -200},
	{ 4040, -200},
	{ 4040, 1080},
	{ 4040, 2360},
	{ 1920, 2360},
	{ -200, 2360},
	{ -200, 1080},
};

// Initialization and setup functions
int initialize_window(void);
void setup(void);

// Texture loading
SDL_Texture* load_texture(const char* filename, SDL_Renderer* renderer);


// Game loop functions
void process_input(void);
void update(float delta_time);
void render(void);
void update_camera(void); void cap_framerate(int* last_frame_time, float* delta_time); //FPS


// Rendering helpers
void render_health_bar(SDL_Renderer* renderer, float health, float max_health, int x, int y, int width, int height);

// Cleanup
void destroy_window();

int main(int argc, char *argv[]) {
	int num = 0;
	printf("Game is running...\n");
	game_is_running = initialize_window();

	setup();
	lastPeriodicCall = SDL_GetTicks() - periodicInterval;

	while (game_is_running) {

		Uint32 currentTime = SDL_GetTicks();
		if (currentTime - lastPeriodicCall >= periodicInterval) {
			printf("minute %d\n", num); // Future for enemy wave come every 1 minute.
			num++;
			lastPeriodicCall += periodicInterval; // Reset the timer
		}


		process_input();
		update(delta_time);
		render();
		cap_framerate(&last_frame_time, &delta_time);
	}

	destroy_window();

	return 0;
}

int initialize_window(void) {
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {

		fprintf(stderr, "Error initializing SDL.\n");
		return FALSE;
	}

	window = SDL_CreateWindow(
		NULL,
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		WINDOW_WIDTH,
		WINDOW_HEIGHT,
		SDL_WINDOW_FULLSCREEN
	);
	if (!window) {
		fprintf(stderr, "Error creating SDL Window.\n");
		return FALSE;
	}

	renderer = SDL_CreateRenderer(window, -1, 0);

	if (!renderer) {
		fprintf(stderr, "Error creating SDL Renderer.\n");
		return FALSE;
	}

	if (IMG_Init(IMG_INIT_PNG) && IMG_INIT_PNG != IMG_INIT_PNG) {
		fprintf(stderr, "Failed to initialize SDL_image: %s\n", IMG_GetError());
		return FALSE;
	}


	return TRUE;
}

void process_input() {
	const Uint8* state = SDL_GetKeyboardState(NULL);
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT) {
			game_is_running = FALSE;
		}
		if (state[SDL_SCANCODE_ESCAPE]) {
			game_is_running = FALSE;
		}
	}

	// Update movement directions based on key states
	move_up = state[SDL_SCANCODE_W];
	move_down = state[SDL_SCANCODE_S];
	move_left = state[SDL_SCANCODE_A];
	move_right = state[SDL_SCANCODE_D];

	// Update facing direction based on movement
	if (move_left) facing_left = 1;
	else if (move_right) facing_left = 0;
}

SDL_Texture* load_texture(const char* filename, SDL_Renderer* renderer) {
	SDL_Surface* surface = IMG_Load(filename);
	if (!surface) {
		fprintf(stderr, "Failed to load image: %s\n", IMG_GetError());
		return NULL;
	}

	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
	if (!texture) {
		fprintf(stderr, "Failed to create texture: %s\n", SDL_GetError());
	}
	else {
		// Set the blend mode for the texture
		SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
	}

	SDL_FreeSurface(surface); // No longer need the surface

	return texture;
}

void setup() {
	srand(time(NULL));

	MC.x = MAP_WIDTH / 2;
	MC.y = MAP_HEIGHT / 2;
	MC.width = 83;
	MC.height = 97;
	MC.movement_speed = 300.0f;
	MC.health = 100.0f; // maximum health is 100


	MC.texture = load_texture("C:/Users/Kanta/source/repos/BIT_Kitchen_Nightmare/BIT_Kitchen_Nightmare/Picture/MC2.png", renderer);
	map_texture = load_texture("C:/Users/Kanta/source/repos/BIT_Kitchen_Nightmare/BIT_Kitchen_Nightmare/Picture/BG4K.png", renderer);

	camera.x = MC.x - WINDOW_WIDTH / 2;
	camera.y = MC.y - WINDOW_HEIGHT / 2;

}

void update_camera() {
	camera.x = MC.x - WINDOW_WIDTH / 2;
	camera.y = MC.y - WINDOW_HEIGHT / 2;

	// Clamp camera to the edges of the map
	if (camera.x < 0) camera.x = 0;
	if (camera.y < 0) camera.y = 0;
	if (camera.x > MAP_WIDTH - camera.width) camera.x = MAP_WIDTH - camera.width;
	if (camera.y > MAP_HEIGHT - camera.height) camera.y = MAP_HEIGHT - camera.height;
}

void update(float delta_time) {

	//Mc movement
	if (move_up) MC.y -= MC.movement_speed * delta_time;
	if (move_down) MC.y += MC.movement_speed * delta_time;
	if (move_left) MC.x -= MC.movement_speed * delta_time;
	if (move_right) MC.x += MC.movement_speed * delta_time;

	// Updated boundary checks for a 1920x1080 map
	if (MC.x < 0) MC.x = 0;
	if (MC.y < 0) MC.y = 0;
	if (MC.x + MC.width > MAP_WIDTH) MC.x = MAP_WIDTH - MC.width; // Use 1920 for the new map width
	if (MC.y + MC.height > MAP_HEIGHT) MC.y = MAP_HEIGHT - MC.height; // Use 1080 for the new map height

	// Update camera position to follow the ball
	update_camera();
}

void render_health_bar(SDL_Renderer* renderer, float health, float max_health, int x, int y, int width, int height) {
	// Background of the health bar (usually red or black)
	SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Red
	SDL_Rect bg_rect = { x, y, width, height };
	SDL_RenderFillRect(renderer, &bg_rect);

	// Foreground of the health bar showing current health
	float health_percentage = health / max_health;
	SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); // Green
	SDL_Rect fg_rect = { x, y, (int)(width * health_percentage), height };
	SDL_RenderFillRect(renderer, &fg_rect);
}

void render() {
	SDL_SetRenderDrawColor(renderer, 34, 139, 34, 255); // Optional, depending on if your map covers the whole screen
	SDL_RenderClear(renderer);

	if (map_texture) {
		SDL_Rect srcRect = {
			(int)camera.x,
			(int)camera.y,
			camera.width,
			camera.height
		};
		SDL_Rect destRect = {
			0, 0, camera.width, camera.height
		};

		// Render the visible portion of the map
		SDL_RenderCopy(renderer, map_texture, &srcRect, &destRect);
	}

	// Render other entities like the player, enemies, etc.
	if (MC.texture) {
		SDL_Rect MC_rect = {
			(int)(MC.x - camera.x), // Adjusted for camera
			(int)(MC.y - camera.y), // Adjusted for camera
			(int)MC.width,
			(int)MC.height
		};

		SDL_RendererFlip flip = facing_left ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
		SDL_RenderCopyEx(renderer, MC.texture, NULL, &MC_rect, 0, NULL, flip);
	}


	int health_bar_width = 1920;
	int health_bar_height = 40;
	int health_bar_x = 0; // Top left corner of the health bar
	int health_bar_y = 0; // Adjusted to be at the bottom of the window
	render_health_bar(renderer, MC.health, 100.0f, health_bar_x, health_bar_y, health_bar_width, health_bar_height);


	SDL_RenderPresent(renderer);
}

void cap_framerate(int* last_frame_time, float* delta_time) {

	int time_to_wait = FRAME_TARGET_TIME - (SDL_GetTicks() - *last_frame_time);
	if (time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME) {
		SDL_Delay(time_to_wait);
	}

	*delta_time = (SDL_GetTicks() - *last_frame_time) / 1000.0f;
	*last_frame_time = SDL_GetTicks();
}


void destroy_window() {
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	if (map_texture) {
		SDL_DestroyTexture(map_texture);
	}

	SDL_Quit();
}