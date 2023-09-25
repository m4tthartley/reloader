
#include <stdarg.h>
#include <stdio.h>

#include <terminal.h>
#include <core.h>
#include <platform.h>
#include <im.h>

core_window_t window;
vec3_t pos = {0, 5};
int2 speed = {1, 1};

void start(void* param) {
	core_window(&window, "Reloader Test", 800, 600, 0);
	core_opengl(&window);

	gfx_coord_system(800.0f/64.0f, 600.0f/64.0f);

}

void frame(void* param) {
	core_window_update(&window);
	gfx_clear(vec4(0,0,0,0));
	// gfx_color(vec4(1, 1, 1, 1));
	if(pos.x < -8.0f) speed.x = 1;
	if(pos.x > 8.0f) speed.x = -1;
	if(pos.y < -8.0f) speed.y = 1;
	if(pos.y > 8.0f) speed.y = -1;
	pos.x += (f32)speed.x * window.dt * 5.0f;
	pos.y += (f32)speed.y * window.dt * 5.0f;
	gfx_quad(pos, vec2(0.5f, 0.5f));
	core_opengl_swap(&window);
}

int main() {
	start(NULL);
	for(;;) {
		frame(NULL);
	}
}

