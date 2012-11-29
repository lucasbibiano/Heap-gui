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

#define SCREEN_W 900
#define SCREEN_H 600
#define DEFAULT_RADIUS 10

typedef struct Tno
{
   int centro_x;
   int centro_y;
   int raio;
} NO;

ALLEGRO_BITMAP* buffer;
ALLEGRO_DISPLAY* screen;
ALLEGRO_FONT* font;

ALLEGRO_EVENT_QUEUE* event_queue = NULL;
ALLEGRO_TIMER* timer = NULL;
bool redraw = true;

const float FPS = 60;

int _size = 220;
int _angle = 5;

Vector2D camera;

int heap_length;

void update_screen()
{
  al_set_target_bitmap(al_get_backbuffer(screen));
  al_draw_bitmap(buffer, 0, 0, 0);
}
 
bool init_double_buffering()
{
  buffer = al_create_bitmap(3000, SCREEN_H);
  
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

  font = al_load_font("FreeMono.ttf", 15, 0);

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

  timer = al_create_timer(1.0 / FPS);
  if(!timer) {
    fprintf(stderr, "failed to create timer!\n");
    return -1;
  }

  event_queue = al_create_event_queue();
  if(!event_queue) {
    fprintf(stderr, "failed to create event_queue!\n");
    al_destroy_display(screen);
    al_destroy_timer(timer);
    return -1;
  }

  al_install_keyboard();

  al_register_event_source(event_queue, al_get_display_event_source(screen));

  al_register_event_source(event_queue, al_get_timer_event_source(timer));
  
  al_register_event_source(event_queue, al_get_keyboard_event_source());

  al_start_timer(timer);

  return true;
}

void draw_heap(int * heap)
{
  Vector2D start = Vector2D((SCREEN_W - DEFAULT_RADIUS * 2)/2 + DEFAULT_RADIUS, 20) + camera; 
  float angle = _angle;

  int mult = 1;
  int size = _size;
  int i = 1;
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
    al_draw_textf(font, al_map_rgb(0, 0, 0), start.x, start.y - DEFAULT_RADIUS + 1, 
      ALLEGRO_ALIGN_CENTRE, "%02d", i);

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
      angle += 5;
      size *= 0.5;
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

bool is_key_pressed(int key)
{
  ALLEGRO_KEYBOARD_STATE s;
  al_get_keyboard_state(&s);
  return al_key_down(&s, key);
}

int main(int argc, char **argv){

  int heap[] = {0, 70, 67, 35, 48, 21, 23, 15, 8};
  heap_length = 100;

  if (!init())
    return -1;

  if (heap_length <= 63)
  {
    _size = 220;
    _angle = 5;
  }
  else
  {
    _size = 440;
    _angle = 5;
  }
  
  bool left = false;
  bool right = false;

  while (true)
  {
    ALLEGRO_EVENT ev;

    al_wait_for_event(event_queue, &ev);

    if(ev.type == ALLEGRO_EVENT_TIMER) 
    {
      redraw = true;
    }
    else if(ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) 
    {
      break;
    }
    else if (ev.type == ALLEGRO_EVENT_KEY_DOWN)
    {
      if (ev.keyboard.keycode == ALLEGRO_KEY_LEFT)
        left = true;
      else if (ev.keyboard.keycode == ALLEGRO_KEY_RIGHT)
        right = true;
      else if (ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE)
        break;
    }
    else if (ev.type == ALLEGRO_EVENT_KEY_UP)
    {
      if (ev.keyboard.keycode == ALLEGRO_KEY_LEFT)
        left = false;
      else if (ev.keyboard.keycode == ALLEGRO_KEY_RIGHT)
        right = false;
    }

    if (left)
    {
      camera += Vector2D(10, 0);
    }

    if (right)
    {
      camera += Vector2D(-10, 0);
    }

    if(redraw && al_event_queue_is_empty(event_queue)) 
    {
      redraw = false;

      al_set_target_bitmap(buffer);
      al_clear_to_color(al_map_rgb(255, 255, 255));

      draw_heap(heap);
      update_screen();
    }

    al_flip_display();
  }

  al_destroy_timer(timer);
  al_destroy_display(screen);
  al_destroy_font(font);
  al_destroy_event_queue(event_queue);
 
  return 0;
}