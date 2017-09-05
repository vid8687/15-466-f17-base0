#include "Draw.hpp"
#include "GL.hpp"

#include <SDL.h>
#include <glm/glm.hpp>

#include <set>
#include <random>
#include <chrono>
#include <iostream>

int main(int argc, char **argv) {
	//Configuration:
	struct {
		std::string title = "Game0: ColorSlide ~ Click to play";
		glm::uvec2 size = glm::uvec2(480, 480);
	} config;


	//Game variables:
	uint32_t score = 0;
	bool wait_for_click = true;
	bool angular_missiles = true;
	typedef struct {
		glm::u8vec4 color = glm::u8vec4(0xff,0xff,0xff,0xff);
		glm::vec2 pos = glm::vec2(0, 1);
		glm::vec2 vel;
		float size = 0.5f;
		bool active = true;
		void update(){
			// flip if it hits the left or right end	
			if (pos.x < -1.0f) vel.x = std::abs(vel.x);
			if (pos.x >  1.0f) vel.x =-std::abs(vel.x);
		
		}
		glm::vec2 get_rect_start(){

			return pos;
		}
		glm::vec2 get_rect_end(){
			return pos + glm::vec2(size, size);
		}
	}missile;
	std::vector<missile> missiles;
	//Base plate:
	std::vector<glm::u8vec4> base;
	std::vector<float> height;

	float base_size = 0.125f; 
	// add color list
	// if number of blocks doesn't span the screen
	// we cycle through them (including the height)
	{	
		base.push_back(glm::u8vec4(0xff,0,0,0xff));
		base.push_back(glm::u8vec4(0xff,0xff,0,0xff));
		base.push_back(glm::u8vec4(0x0,0xff,0,0xff));
		base.push_back(glm::u8vec4(0xff,0,0xff,0xff));
		base.push_back(glm::u8vec4(0x0,0,0xff,0xff));
		base.push_back(glm::u8vec4(0x00,0xff,0xff,0xff));
		base.push_back(glm::u8vec4(0x00,0,0xff,0xff));
		base.push_back(glm::u8vec4(0xaa,0xaa,0,0xff));
		base.push_back(glm::u8vec4(0xff,0,0xaa,0xff));
		base.push_back(glm::u8vec4(0xff,0xff,0xaa,0xff));
		for(uint32_t i =0; i <  base.size(); i++){
			height.push_back(base_size);
		}
	}


	//http://en.cppreference.com/w/cpp/numeric/random/uniform_real_distribution
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<> distx(-1.f,1.f);
	std::uniform_real_distribution<> disty(-1.f+base_size*2,1.f);
	std::uniform_real_distribution<> distl(-1.f, 0);
	std::uniform_int_distribution<> distc(0, base.size()-1);
	int32_t index = 0;	
	float freq = 3.f; //time between missiles 
	float released  = freq;
	//------------  initialization ------------

	//Initialize SDL library:
	SDL_Init(SDL_INIT_VIDEO);

	//Ask for an OpenGL context version 3.3, core profile, enable debug:
	SDL_GL_ResetAttributes();
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

	//create window:
	SDL_Window *window = SDL_CreateWindow(
		config.title.c_str(),
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		config.size.x, config.size.y,
		SDL_WINDOW_OPENGL /*| SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI*/
	);

	if (!window) {
		std::cerr << "Error creating SDL window: " << SDL_GetError() << std::endl;
		return 1;
	}

	//Create OpenGL context:
	SDL_GLContext context = SDL_GL_CreateContext(window);

	if (!context) {
		SDL_DestroyWindow(window);
		std::cerr << "Error creating OpenGL context: " << SDL_GetError() << std::endl;
		return 1;
	}

	#ifdef _WIN32
	//On windows, load OpenGL extensions:
	if (!init_gl_shims()) {
		std::cerr << "ERROR: failed to initialize shims." << std::endl;
		return 1;
	}
	#endif

	//Set VSYNC + Late Swap (prevents crazy FPS):
	if (SDL_GL_SetSwapInterval(-1) != 0) {
		std::cerr << "NOTE: couldn't set vsync + late swap tearing (" << SDL_GetError() << ")." << std::endl;
		if (SDL_GL_SetSwapInterval(1) != 0) {
			std::cerr << "NOTE: couldn't set vsync (" << SDL_GetError() << ")." << std::endl;
		}
	}

	//Hide mouse cursor (note: showing can be useful for debugging):
	SDL_ShowCursor(SDL_DISABLE);

	std::cout<<"Click to start. [SpaceBar] toggles missile directions"<<std::endl;

	//------------  game loop ------------

	auto previous_time = std::chrono::high_resolution_clock::now();
	bool should_quit = false;
	while (true) {
		const char *str = ("ColorSlide score: "+std::to_string(score)).c_str();
		if (!wait_for_click){
			SDL_SetWindowTitle(window, str);
		}
		static SDL_Event evt;
		while (SDL_PollEvent(&evt) == 1) {
			//handle input:
			if (evt.type == SDL_MOUSEBUTTONDOWN) {
				wait_for_click = false;
				released = freq;
			} else if (evt.type == SDL_KEYDOWN && evt.key.keysym.sym == SDLK_ESCAPE) {
				should_quit = true;
			} else if (evt.type == SDL_KEYDOWN && evt.key.keysym.sym == SDLK_LEFT){
				index = (index+1)%base.size();
			} else if (evt.type == SDL_KEYDOWN && evt.key.keysym.sym == SDLK_RIGHT){
				index = (base.size()+index-1)%base.size();
			} else if (evt.type == SDL_KEYDOWN && evt.key.keysym.sym == SDLK_SPACE){
				angular_missiles = !angular_missiles;
			}
			else if (evt.type == SDL_QUIT) {
				should_quit = true;
				break;
			}
		}
		if (should_quit) break;

		auto current_time = std::chrono::high_resolution_clock::now();
		float elapsed = std::chrono::duration< float >(current_time - previous_time).count();
		previous_time = current_time;
		released += elapsed;
		
		if(wait_for_click) released = 0;

		//draw output:
		Draw draw;
		glClearColor(0.0, 0.0, 0.0, 0.0);
		glClear(GL_COLOR_BUFFER_BIT);

		//check winning condition
		// If there is a winning condition, like turn all blocks white
		// or reach some height
		/*
		{
			bool win = true;
			for(auto b : base){
				if(b != glm::u8vec4(0xff)){
					win = false;
				}
			}
			if(win){
			std::cout<<"winner."<<std::endl;
			exit(0);
		}
	}
	*/
	// draw base plate
	{
		uint32_t n = std::max(int32_t(base.size()), int32_t(2*(1.0f/base_size)));
		for(int32_t i = 0; i < n; i++){
			draw.add_rectangle( glm::vec2(-1+i*base_size, -1), glm::vec2(-1+(i+1)*base_size, -1+height[(index+i)%base.size()]), base[(index+i)%base.size()]);		
		}
	}

	if( released > freq)
	{ 
		released = 0.f;
		// new source:
		missile m;
		m.size = base_size;
		m.pos  = glm::vec2(std::floor(distx(gen)/base_size)*base_size, 1);
		
		if ( !angular_missiles ){
			m.vel  = glm::vec2(0,-1);
		}
		else {
			m.vel  = glm::vec2(distx(gen), -1);
			m.vel  = glm::normalize(m.vel);
		}
		while(m.color == glm::u8vec4(0xff))
			m.color = base[distc(gen)];// glm::u8vec4(0xff,0x00,0x00,0xff);
		missiles.push_back(m);
	}
	//update missiles
	{
		auto white = glm::u8vec4(0xff,0xff,0xff,0xff);
		auto match = [&](missile &m)->bool{
				if(!m.active) return true;
				auto p = m.pos;
				auto c = m.color;
				uint32_t m_index = (index+uint32_t((p.x+1+base_size/2.f)/(base_size)))%base.size();
				if(p.y >= -1+height[m_index]){
					return true;
				}
				else if(p.y <=-1+1e-2f){
					// todo save highscore to file
					std::cout<<"game over!!! [ Score: "<<score<<"]"<<std::endl;
					exit(1);
				}
				else{
					m.active = false; 
					auto &b = base[m_index];
					if(b == white || c == white) return true; //white always matches
					if(b == c ){
						//b = white;
						//well, the design document increase height so
						//height[m_index] += base_size/3;
						score++;
						// also frequency variation with score
						freq = std::min(2.f, freq - score*0.1f);
						return true;
					}
					else{
						height[m_index] -= base_size/3;
						if(height[m_index] < -1+0.05){
							height[m_index]=0;
						}
						return false;
					}

				}
				return false;
			};
			for(auto &m : missiles){
				if(!m.active) continue;
				m.size = base_size;
				m.update();
				if( match(m))
				{
					draw.add_rectangle( m.get_rect_start(), m.get_rect_end(), m.color );
					// update their position 
					m.pos += m.vel*std::min(0.1f, elapsed); // for now
					// align to tile
					//if(m.pos.x >= -1 && m.pos.x <= 1)
					//	m.pos.x = std::round(m.pos.x/base_size)*base_size;
				}
				
			}

			missiles.erase(std::remove_if(missiles.begin(), 
						missiles.end(),
						[](missile m){return !m.active; }),
					missiles.end());
			
		}

		
		draw.draw();

		SDL_GL_SwapWindow(window);
	}


	//------------  teardown ------------

	SDL_GL_DeleteContext(context);
	context = 0;

	SDL_DestroyWindow(window);
	window = NULL;

	return 0;
}
