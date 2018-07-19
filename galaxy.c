#define SDL_MAIN_HANDLED
#include "SDL2/SDL.h"
#include "SDL2/SDL_video.h"
#include "SDL2/SDL_render.h"
#include "SDL2/SDL_mixer.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <tgmath.h>
#include <unistd.h>

SDL_Window* window;
SDL_Renderer* renderer;

const double radius 	= 64;	//ly
const double twistval 	= 1.0/256*M_PI;	//rad/ly
const double ab 	= 1.2;	//Major axis / minor axis of stellar orbits
const double star_num	= 100;	//Probability correlation
const int screen_size	= 1024;

struct Point {
	double x;
	double y;
};

struct PointPolar {
	double r;
	double phi;
};

void
init() {
	//Check SDL Version
	SDL_version compiled;
	SDL_version linked;
	SDL_VERSION(&compiled);
	SDL_GetVersion(&linked);
	if (compiled.major != linked.major) {
		fprintf(stderr, "SDL version mismatch! Found version %d, require version %d", linked.major, compiled.major);
		exit(EXIT_FAILURE);
	}


	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER | SDL_INIT_AUDIO)) {
		fprintf(stderr, "Initialisation Error: %s", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	//Hints
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
}

void
quit(int exit_code) {
	SDL_Quit();
	exit(exit_code);
}

void
wait_for_key(SDL_Keycode sym) {
	bool wait = true;
	SDL_Event event;
	while(wait) {
		while(SDL_PollEvent(&event)) {
			if(event.type == SDL_KEYDOWN) {
				if(event.key.keysym.sym == sym) {
					wait = false;
				}
			} else if(event.type == SDL_QUIT) {
				quit(EXIT_SUCCESS);
			}
		}
		SDL_RenderPresent(renderer);
		SDL_Delay(16);
	}
}

struct Point
polar_to_cart(struct PointPolar pp) {
	struct Point p;
	p.x = pp.r*cos(pp.phi);
	p.y = pp.r*sin(pp.phi);
	return p;
}

struct PointPolar
cart_to_polar(struct Point p) {
	struct PointPolar pp;
	pp.r = sqrt(p.x*p.x + p.y*p.y);
	pp.phi = atan2(p.y, p.x);
	return pp;
}

double
pdf(struct PointPolar pp) {
	double val = 1/radius * exp(-pp.r/radius);
	//printf("Pdf = %.100f\n", val);
	return val;
}

struct PointPolar
twist(struct PointPolar pin) {
	struct PointPolar pout;
	pout.r = pin.r;
	pout.phi = pin.phi;
	double a = ab*pin.r;
	double b = pin.r;
	//"Ellipse Number"
	double en = a*b/(sqrt(pow(b * cos(pin.phi), 2) + pow(a * sin(pin.phi), 2)));
	pout.phi = pin.phi + twistval * en;
	return pout;
}

int
main(int argc, char **argv) {
	init();
	srand(getpid());

	if(SDL_CreateWindowAndRenderer(screen_size, screen_size, SDL_WINDOW_BORDERLESS, &window, &renderer)) {
		fprintf(stderr, "Window creation failed: %s", SDL_GetError());
		exit(EXIT_FAILURE);
	}
	SDL_SetWindowTitle(window, "Galaxy");

	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);

	int star_count = 0;

	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	for(double y = -screen_size/2; y < screen_size/2; y++) {
		for(double x = -screen_size/2; x < screen_size/2; x++) {
			double randval = (rand() / (double)RAND_MAX) / star_num;
			struct PointPolar pp = cart_to_polar((struct Point){x, y});
			if(pdf(pp) > randval) {
				struct Point p = polar_to_cart(twist(pp));
				SDL_RenderDrawPoint(renderer, p.x + screen_size/2, p.y + screen_size/2);
				star_count++;
			}
			////Alternative: Set brightness (smooth galaxy), factor is no good
			//int i = 1e6*pdf(pp);
			//SDL_SetRenderDrawColor(renderer, I,I,I,255);
			//struct Point p = polar_to_cart(twist(pp));
			//SDL_RenderDrawPoint(renderer, p.x + screen_size/2, p.y + screen_size/2);
		}
	}

	SDL_RenderPresent(renderer);
	printf("%d stars\n", star_count);

	wait_for_key(SDLK_ESCAPE);
	return 0;
}
