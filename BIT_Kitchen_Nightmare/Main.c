#define SDL_MAIN_HANDLED
#include <stdio.h>
#include "SDL.h"
#include "./constant.h"
#include "SDL_image.h"
#include "SDL_main.h"
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

//Define variable
int game_is_running = FALSE;
int in_menu = TRUE;
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
SDL_Texture* MC_texture = NULL;
SDL_Texture* map_texture = NULL;
SDL_Texture* start_button_texture = NULL;
SDL_Texture* menu_background_texture = NULL;

//Pause menu
int is_game_paused = FALSE;
int menu_item_selected = 0;//0 for resume, 1 for exit
SDL_Texture* resume_button_texture = NULL;
SDL_Texture* exit_button_texture = NULL;

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

//Player camera position
struct Camera {
	float x, y;
	int width, height;
} camera = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };

//Attack system
typedef struct {
	SDL_Rect area; // The attack area
	Uint32 cooldown; // Time in milliseconds between attacks
	Uint32 lastAttackTime; // Last time this attack was used
	SDL_Texture* vfxTexture; // Texture for the attack's visual effect
	int damage; // Damage dealt by the attack
	int level; // Level of the attack, influencing damage, area, etc.
	bool isActive;// Is the attack currently active/enabled
	bool isHave;
} AutoAttack;

//Character stat
struct character {
	float x;
	float y;
	float width;
	float height;
	float movement_speed; // Pixels per second, adjust as needed
	float health;
	AutoAttack attacks[MAX_ATTACKS];
	int haveAttacks; // Number of attacks currently active
	SDL_Texture* texture;
} MC;

//Each enemy type stat
typedef struct {
	float width;
	float height;
	float movement_speed;
	float health;
	float atk;
	SDL_Texture* texture;
} Enemy_type;

//Store each enemy in the game;
typedef struct {
	SDL_Rect rect; // Position and size
	int type; // Index to the Enemy_type
	float currentHealth; // Current health if it can decrease from the base
	int isActive; // 1 if active, 0 if not
} Enemy;

//Wave data
typedef struct {
	int Enemy_count[MAX_ENEMY_TYPE]; //index will shown amount of itype enemy
}Wave;

//stage data
typedef struct {
	Wave waves[MAX_WAVES];
} Stage;

//Define struct of enemy data
Enemy_type type[MAX_ENEMY_TYPE];
Stage stage1;
Enemy Enemies[MAX_ENEMIES_STAGE1];
int randPos;
int typeTexture;


//8 spawn position
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

//Enemies function related
void initialize_enemies(SDL_Renderer* renderer);
void initialize_stage1_enemies();
void spawn_wave(int waveIndex);
void render_enemies(SDL_Renderer* renderer);
void update_enemies(float delta_time);

// Texture loading
SDL_Texture* load_texture(const char* filename, SDL_Renderer* renderer);

//combat mechanics
void check_collision_and_apply_damage(float delta_time);
void initialize_attacks(void);
void updated_attacks(Uint32 currentTime);
void render_attacks();
void apply_attack_damage_to_enemies();



// Game loop functions
void process_input(void);
void update(float delta_time);
void render(void);
void update_camera(void); 
void cap_framerate(int* last_frame_time, float* delta_time); //FPS


// Rendering helpers
void render_health_bar(SDL_Renderer* renderer, float health, float max_health, int x, int y, int width, int height);

// Cleanup
void destroy_window();

int main(int argc, char *argv[]) {
	int waveIndex = 0;
	printf("Game is running...\n");
	game_is_running = initialize_window();

	setup();
	lastPeriodicCall = SDL_GetTicks() - periodicInterval;

	while (game_is_running) {

		//Wave will spawn every 1 minute.
		Uint32 currentTime = SDL_GetTicks();
		if (currentTime - lastPeriodicCall >= periodicInterval) {
			
			while (waveIndex <= 19) {
				spawn_wave(waveIndex);
				waveIndex++;
				break;
			}

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

	initialize_enemies(renderer);
	initialize_stage1_enemies();
	initialize_attacks();

	return TRUE;
}

void process_input() {
	const Uint8* state = SDL_GetKeyboardState(NULL);
	SDL_Event event;

	int mouseX, mouseY;
	SDL_GetMouseState(&mouseX, &mouseY);

	mouseX += camera.x;
	mouseY += camera.y;

	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT) {
			game_is_running = FALSE;
		}

		if (in_menu) {
			if (event.type == SDL_MOUSEBUTTONDOWN) {
				int x, y;
				SDL_GetMouseState(&x, &y);
				if (x >= 720 && x <= 1160 && y >= 450 && y <= 670) { // If the mouse click is within the start button area
					in_menu = FALSE; // Start the game
				}
			}
		}

		if(event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
			is_game_paused = !is_game_paused;
		}

		if (is_game_paused && event.type == SDL_MOUSEBUTTONDOWN) {
			// Assuming these are the button positions and sizes
			SDL_Rect resume_button_rect = { 785, 320, 350, 150 }; // Placeholder positions
			SDL_Rect exit_button_rect = { 785, 600, 350, 150 }; // Placeholder positions

			int mouseX, mouseY;
			SDL_GetMouseState(&mouseX, &mouseY);

			if (mouseX >= resume_button_rect.x && mouseX <= resume_button_rect.x + resume_button_rect.w &&
				mouseY >= resume_button_rect.y && mouseY <= resume_button_rect.y + resume_button_rect.h) {
				// Resume button was clicked
				is_game_paused = FALSE;
			}
			else if (mouseX >= exit_button_rect.x && mouseX <= exit_button_rect.x + exit_button_rect.w &&
				mouseY >= exit_button_rect.y && mouseY <= exit_button_rect.y + exit_button_rect.h) {
				// Exit button was clicked
				game_is_running = FALSE;
			}
		}

		// Update facing direction based mouse direction
		if (mouseX > (MC.x + MC.width / 2)) {
			facing_left = 0; // Face right
		}
		else {
			facing_left = 1; // Face left
		}
	}

	// Update movement directions based on key states
	move_up = state[SDL_SCANCODE_W];
	move_down = state[SDL_SCANCODE_S];
	move_left = state[SDL_SCANCODE_A];
	move_right = state[SDL_SCANCODE_D];

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

	//Set up MC stat
	MC.x = MAP_WIDTH / 2;
	MC.y = MAP_HEIGHT / 2;
	MC.width = 83;
	MC.height = 97;
	MC.movement_speed = 300.0f;
	MC.health = 100.0f; // maximum health is 100

	//Load texture.
	MC.texture = load_texture("Assets/Main_character/MC.png", renderer);
	map_texture = load_texture("Assets/Background/BG.png", renderer);
	start_button_texture = load_texture("Assets/Lobby/start.png", renderer);
	menu_background_texture = load_texture("Assets/Lobby/Lobby_BG.png", renderer);
	resume_button_texture = load_texture("Assets/Pause_menu/Resume_button.png", renderer);
	exit_button_texture = load_texture("Assets/Pause_menu/Exit_button.png", renderer);

	camera.x = MC.x - WINDOW_WIDTH / 2;
	camera.y = MC.y - WINDOW_HEIGHT / 2;

}

//Each type of Enemy stat initialize
void initialize_enemies(SDL_Renderer* renderer) {
	type[0].width = 100;
	type[0].height = 100;
	type[0].movement_speed = 150;
	type[0].health = 100;
	type[0].atk = 10;
	type[0].texture = load_texture("Assets/Enemy/Enemy_pig.png", renderer);
	
	type[1].width = 50;
	type[1].height = 50;
	type[1].movement_speed = 150;
	type[1].health = 200;
	type[1].atk = 30;
	type[1].texture = load_texture("Assets/Enemy/Enemy_chicken.png", renderer);

	//---------------------------------------
	type[2].width = 40;
	type[2].height = 40;
	type[2].movement_speed = 150;
	type[2].health = 200;
	type[2].atk = 30;
	type[2].texture = load_texture("Assets/Enemy/Enemy_bell_pepper.png", renderer);
	//---------------------------------------------
	
}

//initialize enemy per wave.
void initialize_stage1_enemies() {

	stage1.waves[0].Enemy_count[0] = 15; 
	stage1.waves[0].Enemy_count[1] = 5;
	stage1.waves[0].Enemy_count[2] = 5;

	stage1.waves[1].Enemy_count[0] = 20;
	stage1.waves[1].Enemy_count[1] = 10;
	stage1.waves[1].Enemy_count[2] = 5;

	stage1.waves[2].Enemy_count[0] = 25;
	stage1.waves[2].Enemy_count[1] = 10;
	stage1.waves[2].Enemy_count[2] = 5;

	stage1.waves[3].Enemy_count[0] = 25;
	stage1.waves[3].Enemy_count[1] = 20;
	stage1.waves[3].Enemy_count[2] = 5;

	stage1.waves[4].Enemy_count[0] = 30;
	stage1.waves[4].Enemy_count[1] = 30;
	stage1.waves[4].Enemy_count[2] = 5;

}

//Spawn a wave of enemy
void spawn_wave(int waveIndex) {
	Wave wave = stage1.waves[waveIndex];
	for (int typeIndex = 0; typeIndex < MAX_ENEMY_TYPE; ++typeIndex) { //loop for amount of type
		int count = wave.Enemy_count[typeIndex];
		for (int i = 0; i < count; ++i) { //loop for amount of each type
			// Find a free spot in the enemies array
			for (int j = 0; j < MAX_ENEMIES_STAGE1; ++j) {
				if (!Enemies[j].isActive) {
					Enemies[j].type = typeIndex;
					Enemies[j].isActive = 1;
					Enemies[j].currentHealth = type[typeIndex].health;
					
					randPos = rand() % 8; //Random postion for enemy's spawn
						
					Enemies[j].rect.x = spawn_position[randPos].x;
					Enemies[j].rect.y = spawn_position[randPos].y;
					Enemies[j].rect.w = type[typeIndex].width;
					Enemies[j].rect.h = type[typeIndex].height;
					break; // Break the loop after setting up an enemy
				}
			}
		}
	}
}


void update_enemies(float delta_time) {
	for (int i = 0; i < MAX_ENEMIES_STAGE1; ++i) {
		if (Enemies[i].isActive) {
			float dx = MC.x - (Enemies[i].rect.x - Enemies[i].rect.w / 2);
			float dy = MC.y - (Enemies[i].rect.y - Enemies[i].rect.h / 2);
			float distance = sqrt(dx * dx + dy * dy);

			// Avoid division by zero
			if (distance == 0) continue;

			float dx_normalized = dx / distance;
			float dy_normalized = dy / distance;

			// Get the enemy's movement speed
			float speed = type[Enemies[i].type].movement_speed;

			// Update enemy position
			Enemies[i].rect.x += dx_normalized * speed * delta_time;
			Enemies[i].rect.y += dy_normalized * speed * delta_time;

			for (int j = 0; j < MAX_ENEMIES_STAGE1; ++j) {
				if (i != j && Enemies[j].isActive) {
					SDL_Rect rect1 = Enemies[i].rect;
					SDL_Rect rect2 = Enemies[j].rect;

					// Calculate the intersection rectangle
					SDL_Rect intersection;
					if (SDL_IntersectRect(&rect1, &rect2, &intersection)) {
						int overlapWidth = intersection.w;
						int overlapHeight = intersection.h;

						// Calculate 50% of each enemy's size
						int allowedOverlapWidth = rect1.w / 2;
						int allowedOverlapHeight = rect1.h / 2;

						// Check if the overlap exceeds 50% of the size
						if (overlapWidth > allowedOverlapWidth || overlapHeight > allowedOverlapHeight) {
							// Resolve the overlap by adjusting positions
							// This is a simple resolution strategy and may need refinement
							if (rect1.x < rect2.x) {
								Enemies[i].rect.x -= overlapWidth / 20;
								Enemies[j].rect.x += overlapWidth / 20;
							} else {
								Enemies[i].rect.x += overlapWidth / 20;
								Enemies[j].rect.x -= overlapWidth / 20;
							}

							if (rect1.y < rect2.y) {
								Enemies[i].rect.y -= overlapHeight / 20;
								Enemies[j].rect.y += overlapHeight / 20;
							} else {
								Enemies[i].rect.y += overlapHeight / 20;
								Enemies[j].rect.y -= overlapHeight / 20;
							}
						}
					}
				}
			}
		}
	}
}

void render_enemies(SDL_Renderer* renderer) {
	for (int i = 0; i < MAX_ENEMIES_STAGE1; ++i) {
		if (Enemies[i].isActive) {

			SDL_Rect screen_rect = {
				Enemies[i].rect.x - (int)camera.x,
				Enemies[i].rect.y - (int)camera.y,
				Enemies[i].rect.w,
				Enemies[i].rect.h
			};
			SDL_RenderCopy(renderer, type[Enemies[i].type].texture, NULL, &screen_rect);
			//For debug
			//SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255); // Blue for enemy hitbox
			//SDL_RenderDrawRect(renderer, &screen_rect);

		}
	}
}

void check_collision_and_apply_damage(float delta_time) {
	// Assuming the MC's rect is set up like this
	SDL_Rect mcRect = { MC.x, MC.y, MC.width, MC.height };

	for (int i = 0; i < MAX_ENEMIES_STAGE1; ++i) {
		if (Enemies[i].isActive) {
			// Check for collision between MC and the active enemy
			if (SDL_HasIntersection(&mcRect, &Enemies[i].rect)) {
				// Collision detected, MC takes damage from enemy's attack power
				float damage = type[Enemies[i].type].atk * delta_time;
				MC.health -= damage;
				if (MC.health < 0) MC.health = 0; // Prevent health from dropping below zero

			}
		}
	}
}

//Attack
void initialize_attacks(void) {
	for (int i = 0; i < MAX_ATTACKS; ++i) {
		MC.attacks[i].isHave = false; // Initially, most attacks are inactive
		// Initialize other fields as needed...
	}

	// Setup the first attack
	MC.attacks[0].isActive = false;
	MC.attacks[0].isHave = true;
	MC.attacks[0].cooldown = 2000; // Example: 2 seconds
	MC.attacks[0].lastAttackTime = 0;
	MC.attacks[0].damage = 50; // Example damage
	MC.attacks[0].area = (SDL_Rect){0, 0, 200, 300}; // Set size, position is dynamic
	MC.attacks[0].vfxTexture = load_texture("Assets/Attack_VFX/Attack3.png", renderer);

	MC.haveAttacks = 1; // MC starts with one active attack
}

void updated_attacks(Uint32 currentTime) {
	
	for (int i = 0; i < MC.haveAttacks; ++i) {
		AutoAttack* attack = &MC.attacks[i];
		if (attack->isHave && !attack->isActive && (currentTime - attack->lastAttackTime >= attack->cooldown)) {
			// Set the attack as active
			attack->isActive = true;

			// Record the current time as the last attack time
			attack->lastAttackTime = currentTime;

			// Determine the attack area's X position based on facing direction
			if (facing_left) {
				// Attack spawns to the left of the MC, aligning its right edge with MC's center
				attack->area.x = MC.x - (attack->area.w - (MC.width / 2));
			}
			else {
				// Attack spawns to the right of the MC, starting from MC's center
				attack->area.x = MC.x + (MC.width / 2);
			}

			// Vertically center the attack area relative to the MC
			attack->area.y = MC.y - (attack->area.h - MC.height) / 2;

		}

		// Optionally, deactivate the attack after a short duration
		// This is useful if your attack should "expire" even if it hasn't hit anything
		if (attack->isActive && (currentTime - attack->lastAttackTime > ATTACK_DURATION)) {
			attack->isActive = false;
		}
	}
}

void render_attacks() {

	Uint32 currentTime = SDL_GetTicks();

	/*For debug------------------------------------------------------------------------ -
	if (MC.attacks[0].isActive && (currentTime - MC.attacks[0].lastAttackTime <= 100)) {
		// Adjust the position of the debug visual for the attack area relative to the camera
		SDL_Rect debugAttackArea = {
			MC.attacks[0].area.x - camera.x, // Adjust X position relative to camera
			MC.attacks[0].area.y - camera.y, // Adjust Y position relative to camera
			MC.attacks[0].area.w, // Width of the attack area
			MC.attacks[0].area.h  // Height of the attack area
		};

		// Set the draw color to red for high visibility
		SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);

		//Draw the debug visual as a solid rectangle
		SDL_RenderFillRect(renderer, &debugAttackArea);
	}
	*/
	

	if (MC.attacks[0].isActive && (currentTime - MC.attacks[0].lastAttackTime <= 100)) { // Display VFX for a short duration
		// Adjust the attack area position relative to the camera
		SDL_Rect vfxPosition = {
			MC.attacks[0].area.x - camera.x, // Adjust X position relative to camera
			MC.attacks[0].area.y - camera.y, // Adjust Y position relative to camera
			MC.attacks[0].area.w, // Width of the VFX
			MC.attacks[0].area.h  // Height of the VFX
		};

		// Determine the flip state based on the character's facing direction
		SDL_RendererFlip flipType = facing_left ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;

		// Render the VFX texture within the adjusted position, with possible horizontal flip
		SDL_RenderCopyEx(renderer, MC.attacks[0].vfxTexture, NULL, &vfxPosition, 0.0, NULL, flipType);


	}

}

void apply_attack_damage_to_enemies() {
	
	Uint32 currentTime = SDL_GetTicks();

	for (int attackIndex = 0; attackIndex < MC.haveAttacks; ++attackIndex) {
		AutoAttack* attack = &MC.attacks[attackIndex];

		// Skip if the attack is not active or the character doesn't have this attack
		if (!attack->isHave || !attack->isActive) {
			continue;
		}

		// Check if the attack's active duration has passed (assuming a fixed short duration for demonstration)
		if (currentTime - attack->lastAttackTime > 100) { // 100 ms for example
			attack->isActive = false;
			continue;
		}

		// Assuming you have a function or logic here to determine if enemies are within the attack area
		// This could involve iterating over all enemies and checking collision with the attack->area
		for (int enemyIndex = 0; enemyIndex < MAX_ENEMIES_STAGE1; ++enemyIndex) {
			Enemy* enemy = &Enemies[enemyIndex];

			if (!enemy->isActive) continue; // Skip inactive enemies

			SDL_Rect enemyHitbox = { enemy->rect.x, enemy->rect.y, enemy->rect.w, enemy->rect.h };
			SDL_Rect attackArea = { attack->area.x, attack->area.y, attack->area.w, attack->area.h };

			// Check if the attack area intersects with the enemy hitbox
			if (SDL_HasIntersection(&attackArea, &enemyHitbox)) {
				// Apply damage to the enemy
				enemy->currentHealth -= attack->damage;

				// Log or handle enemy defeat if health drops below zero
				if (enemy->currentHealth <= 0) {
					enemy->isActive = false; // Enemy defeated
					// Additional logic for handling defeated enemy (e.g., scoring)
				}

				
			}
		}
	}
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

	Uint32 currentTime = SDL_GetTicks();

	if (is_game_paused || in_menu) return;

	// Process auto-attack trigger
	updated_attacks(currentTime);
	update_enemies(delta_time);
	updated_attacks(currentTime);
	apply_attack_damage_to_enemies();
	check_collision_and_apply_damage(delta_time);

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

	if (in_menu) {

		SDL_Rect menu_background_rect = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };
		SDL_RenderCopy(renderer, menu_background_texture, NULL, &menu_background_rect);

		SDL_Rect start_button_rect = { 720, 450, 420, 120 };
		SDL_RenderCopy(renderer, start_button_texture, NULL, &start_button_rect);

	}
	else {

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

		//Render enemies
		render_enemies(renderer);

		// Render the attack VFX if active and within the display window
		render_attacks();
		


		int health_bar_width = 1920;
		int health_bar_height = 40;
		int health_bar_x = 0; // Top left corner of the health bar
		int health_bar_y = 0; // Adjusted to be at the bottom of the window
		render_health_bar(renderer, MC.health, 100.0f, health_bar_x, health_bar_y, health_bar_width, health_bar_height);
	}

	if (is_game_paused) {
		// Adjust position and sizes as needed
		SDL_Rect resume_button_rect = { 785, 320, 350, 150 }; // Placeholder position
		SDL_Rect exit_button_rect = { 785, 600, 350, 150 }; // Placeholder position

		SDL_RenderCopy(renderer, resume_button_texture, NULL, &resume_button_rect);
		SDL_RenderCopy(renderer, exit_button_texture, NULL, &exit_button_rect);

		// Optionally highlight the selected menu item
		// This part is not fully detailed in the provided snippet. You might draw a rectangle
		// or use different textures to indicate the selected item.

	}

	SDL_RenderPresent(renderer);
}

//Control framerate( 60 FPS )
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
	SDL_DestroyTexture(resume_button_texture);
	SDL_DestroyTexture(exit_button_texture);


	SDL_Quit();
}