#include <stdio.h>
#include <math.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>

#include <queue>

#include "Vector2D.cpp"

using namespace math;
using namespace std;

#define SCREEN_W 800
#define SCREEN_H 640
#define DEFAULT_RADIUS 16

typedef struct Tno
{
   int centro_x;
   int centro_y;
   int raio;
} NO;

ALLEGRO_BITMAP* buffer;
ALLEGRO_DISPLAY* screen;
ALLEGRO_FONT *font;

int heap_length;

void update_screen()
{
  al_set_target_bitmap(al_get_backbuffer(screen));
  al_draw_bitmap(buffer, 0, 0, 0);
}
 
bool init_double_buffering()
{
  buffer = al_create_bitmap(SCREEN_W, SCREEN_H);
  
  if(!buffer)
     return false;
  
  return true;
}

bool init()
{
  if(!al_init()) 
  {
     fprintf(stderr, "failed to initialize allegro!\n");
     return false;
  }

  if (!al_init_primitives_addon())   
  {
     fprintf(stderr, "failed to initiate allegro_primitives!\n");
     return false;
  }
  
  al_init_font_addon();

  if (!al_init_ttf_addon())
  {
    fprintf(stderr, "Falha ao inicializar add-on allegro_ttf.\n");
    return -1;
  }

  font = al_load_font("FreeMono.ttf", 24, 0);

  if (!font)
  {
    fprintf(stderr, "failed to create font!\n");
    return false;
  }

  screen = al_create_display(SCREEN_W, SCREEN_H);
  if(!screen) 
  {
     fprintf(stderr, "failed to create display!\n");
     return false;
  }

  if (!init_double_buffering())
  {
     fprintf(stderr, "failed to create buffer!\n");
     return false;
  }

  if(!al_install_mouse()) {
    printf("Error installing mouse.\n");
    return 1;
  }

  return true;
}

void draw_heap(int * heap)
{
  Vector2D start = Vector2D((SCREEN_W - DEFAULT_RADIUS * 2)/2 + DEFAULT_RADIUS, 20); 
  float angle = 10;

  int mult = 1;
  int i = 1;
  int size = 200;
  int soma = 0;

  queue<Vector2D> toDraw;

  toDraw.push(start);

  while (i <= heap_length)
  {
    Vector2D dirRight = start.newBySizeAngle(size, angle * M_PI/180);
    Vector2D dirLeft = start.newBySizeAngle(size, (180 - angle) * M_PI/180);

    start = toDraw.front();
    toDraw.pop();

    Vector2D aux = start + dirLeft;

    al_draw_circle(start.x, start.y, DEFAULT_RADIUS, al_map_rgb(0, 255, 0), 1);   
    al_draw_textf(font, al_map_rgb(0, 0, 0), start.x, start.y - DEFAULT_RADIUS, 
      ALLEGRO_ALIGN_CENTRE, "%02d", heap[i]);

    if (i * 2 <= heap_length)
    {
      al_draw_line(start.x, start.y + DEFAULT_RADIUS, aux.x, aux.y + DEFAULT_RADIUS,
        al_map_rgb(0,0,255), 1);
    }

    toDraw.push(aux + Vector2D(0, DEFAULT_RADIUS * 2));

    aux = start + dirRight;

    if (i * 2 + 1 <= heap_length)
    {
      al_draw_line(start.x, start.y + DEFAULT_RADIUS, aux.x, aux.y + DEFAULT_RADIUS,
        al_map_rgb(0,0,255), 1);
    }

    toDraw.push(aux + Vector2D(0, DEFAULT_RADIUS * 2));

    mult = 1;
    soma = 1;

    while (soma < i)
    {
      mult *= 2;
      soma += mult;
    }

    if (soma == i)
    {
      angle += 30;
      size -= 30;
    }

    i++;
  }
}

void mouse()
{
  ALLEGRO_MOUSE_STATE state;

  al_get_mouse_state(&state);
  if (state.buttons & 1) {
    /* Primary (e.g. left) mouse button is held. */
    printf("Mouse position: (%d, %d)\n", state.x, state.y);
  }
  if (state.buttons & 2) {
    /* Secondary (e.g. right) mouse button is held. */
  }
  if (state.buttons & 4) {
    /* Tertiary (e.g. middle) mouse button is held. */
  }
}

int main(int argc, char **argv){

  int heap[] = {0, 70, 67, 35, 48, 21, 23, 15, 8};
  heap_length = 8;

  if (!init())
    return -1;
 
  al_set_target_bitmap(buffer);
  al_clear_to_color(al_map_rgb(255, 255, 255));
  draw_heap(heap);

  update_screen();
  al_flip_display();
 
  al_rest(15.0);

  al_destroy_display(screen);
  al_destroy_font(font);
 
  return 0;
}