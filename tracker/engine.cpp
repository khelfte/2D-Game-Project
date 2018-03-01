#include <iostream>
#include <algorithm>
#include <sstream>
#include <string>
#include <random>
#include <iomanip>
#include "sprite.h"
#include "multisprite.h"
#include "twowayMultisprite.h"
#include "gamedata.h"
#include "engine.h"
#include "frameGenerator.h"
#include "unistd.h"

Engine::~Engine() { 
  for(Drawable* s : sprites){
	  delete s;
  }
  std::cout << "Terminating program" << std::endl;
}

Engine::Engine() :
  rc( RenderContext::getInstance() ),
  io( IoMod::getInstance() ),
  clock( Clock::getInstance() ),
  renderer( rc->getRenderer() ),
  //loader( "Loadscreen", Gamedata::getInstance().getXmlInt("Loadscreen/factor") ),
  sky( "sky", Gamedata::getInstance().getXmlInt("sky/factor") ),
  stars( "stars", Gamedata::getInstance().getXmlInt("stars/factor") ),
  field( "field", Gamedata::getInstance().getXmlInt("field/factor") ),
  viewport( Viewport::getInstance() ),
  sprites(),
  currentSprite(0),
  makeVideo( false )
{
  int NumSprites = Gamedata::getInstance().getXmlInt("NumSprites");
  
  sprites.reserve((2*NumSprites)+4);
  sprites.emplace_back(new Sprite("Moon"));
  sprites.emplace_back(new Sprite("Trees"));
  sprites.emplace_back(new TwoWayMultiSprite("Ruby"));
  
  for(int i =0; i<NumSprites; i++){
    sprites.emplace_back(new TwoWayMultiSprite("Crow"));
    sprites.emplace_back(new TwoWayMultiSprite("Wolf"));
  }
  Viewport::getInstance().setObjectToTrack(sprites[2]);
  std::cout << "Loading complete" << std::endl;
}

void Engine::draw() const {
  sky.draw();
  stars.draw();
  field.draw();
  for(Drawable* s : sprites){
	  s->draw();
  }
  SDL_Color c = {0,0,0,255};
  std::stringstream strm;
  strm << clock.getFps();
  string fps("FPS: " + strm.str());
  io.writeText("Kaitlin E. Helfter",30, 640,c);
  io.writeText(fps,30, 680,c);
  
  viewport.draw();
  SDL_RenderPresent(renderer);
}

void Engine::update(Uint32 ticks) {
  for(Drawable* s : sprites){
    s->update(ticks);
  }
  sky.update();
  stars.update();
  field.update();
  viewport.update(); // always update viewport last
}

void Engine::switchSprite(){
  ++currentSprite;
  currentSprite = currentSprite % 2;
  if ( currentSprite ) {
    Viewport::getInstance().setObjectToTrack(sprites[2]);
  }
  else {
    Viewport::getInstance().setObjectToTrack(sprites[3]);
  }
}

void Engine::play() {
  SDL_Event event;
  const Uint8* keystate;
  bool done = false;
  Uint32 ticks = clock.getElapsedTicks();
  FrameGenerator frameGen;

  while ( !done ) {
    // The next loop polls for events, guarding against key bounce:
    while ( SDL_PollEvent(&event) ) {
      keystate = SDL_GetKeyboardState(NULL);
      if (event.type ==  SDL_QUIT) { done = true; break; }
      if(event.type == SDL_KEYDOWN) {
        if (keystate[SDL_SCANCODE_ESCAPE] || keystate[SDL_SCANCODE_Q]) { // perhaps add a key press that gets rid of load screen? enter?
          done = true;
          break;
        }
        if ( keystate[SDL_SCANCODE_P] ) {
          if ( clock.isPaused() ) clock.unpause();
          else clock.pause();
        }
        if ( keystate[SDL_SCANCODE_T] ) {
          switchSprite();
        }
        if (keystate[SDL_SCANCODE_F4] && !makeVideo) {
          std::cout << "Initiating frame capture" << std::endl;
          makeVideo = true;
        }
        else if (keystate[SDL_SCANCODE_F4] && makeVideo) {
          std::cout << "Terminating frame capture" << std::endl;
          makeVideo = false;
        }
      }
    }

    // In this section of the event loop we allow key bounce:

    ticks = clock.getElapsedTicks();
    if ( ticks > 0 ) {
      clock.incrFrame();
      draw();
      update(ticks);
      if ( makeVideo ) {
        frameGen.makeFrame();
      }
    }
  }
}
