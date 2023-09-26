
#include <stdarg.h>
#include <stdio.h>

#include <terminal.h>
#include <core.h>
#include <platform.h>
#include <im.h>

typedef struct {
	core_window_t window;
	vec3_t pos;
	int2 speed;
	dynarr_t blocks;
} state_t;

typedef struct {
	float color;
} block_t;

void* start() {
	state_t* state = malloc(sizeof(state_t));

	core_window(&state->window, "Reloader Test", 800, 600, 0);
	core_opengl(&state->window);

	state->pos = (vec3_t){0, 5};
	state->speed = (int2){1, 1};

	state->blocks = dynarr(sizeof(block_t));

	return state;
}

void frame(void* param) {
	state_t* state = param;
	core_window_update(&state->window);
	gfx_coord_system(800.0f/64.0f, 600.0f/64.0f);
	gfx_clear(vec4(sinf(core_time_seconds(&state->window))*0.5f+0.5f,0.8f,0.5f,0));
	// gfx_clear(vec4(0.5f,0.8f,0.5f,0));
	gfx_color(vec4(1, 1, 1, 1));
	if(state->pos.x < -8.0f) state->speed.x = 1;
	if(state->pos.x > 8.0f) state->speed.x = -1;
	if(state->pos.y < -8.0f) state->speed.y = 1;
	if(state->pos.y > 8.0f) state->speed.y = -1;
	state->pos.x += (f32)state->speed.x * state->window.dt * 5.0f;
	state->pos.y += (f32)state->speed.y * state->window.dt * 5.0f;
	gfx_quad(state->pos, vec2(0.5f, 0.5f));

	float time = sinf(core_time_seconds(&state->window) * 1.0f) * 0.4f + 0.6f;
	// printf("%f \n", time);
	block_t block = {time};
	dynarr_push(&state->blocks, &block);

	if (state->blocks.count > 420) {
		FOR (i, 420) {
			dynarr_pop(&state->blocks, 0);
		}
	}

	block_t* b1 = dynarr_get(&state->blocks, state->blocks.count-1);
	// printf("%f \n", b1->color);

	FOR (i, state->blocks.count) {
		block_t* b = dynarr_get(&state->blocks, i);
		// printf("%f \n", *color);
		gfx_color(vec4(0, 0, b->color, 1));
		gfx_quad(vec3(-10.0f + (i % 21)*1.0f, -10.0f + (i / 21), 0), vec2(0.5f, 0.5f));
	}

	core_opengl_swap(&state->window);
}

int main() {
	start(NULL);
	for(;;) {
		frame(NULL);
	}
}

