#include <stdio.h>
#include <math.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_image.h>

#include <queue>

#include "Vector2D.cpp"

using namespace math;
using namespace std;

#define SCREEN_W 900
#define SCREEN_H 600
#define DEFAULT_RADIUS 12

#define ESPECIFICO 0
#define ALEATORIO 1

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

string mensagem;

int selected;

//posicoes dos nos
Vector2D posNos[105];

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
  selected = -1;

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

  al_init_image_addon();

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

  al_set_new_display_option(ALLEGRO_SAMPLE_BUFFERS, 1, ALLEGRO_SUGGEST);
  al_set_new_display_option(ALLEGRO_SAMPLES, 8, ALLEGRO_SUGGEST);

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

  al_register_event_source(event_queue, al_get_mouse_event_source());


  al_start_timer(timer);

  return true;
}

void draw_heap(int * heap)
{
  Vector2D start = Vector2D((SCREEN_W - DEFAULT_RADIUS * 2)/2 + DEFAULT_RADIUS, 50) + camera; 
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

    if (selected == i)
      al_draw_circle(start.x, start.y, DEFAULT_RADIUS, al_map_rgb(255, 0, 0), 2);   
    else
      al_draw_circle(start.x, start.y, DEFAULT_RADIUS, al_map_rgb(0, 255, 0), 2);   

    al_draw_textf(font, al_map_rgb(0, 0, 0), start.x, start.y - DEFAULT_RADIUS + 3, 
      ALLEGRO_ALIGN_CENTRE, "%02d", i);

    posNos[i] = start;

    if (i * 2 <= heap_length)
    {
      al_draw_line(start.x, start.y + DEFAULT_RADIUS, aux.x, aux.y + DEFAULT_RADIUS,
        al_map_rgb(0, 0, 255), 2);
    }

    toDraw.push(aux + Vector2D(0, DEFAULT_RADIUS * 2));

    aux = start + dirRight;

    if (i * 2 + 1 <= heap_length)
    {
      al_draw_line(start.x, start.y + DEFAULT_RADIUS, aux.x, aux.y + DEFAULT_RADIUS,
        al_map_rgb(0, 0, 255), 2);
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

void draw_menu(int start_y)
{
  if (selected == -1)
  {
    al_draw_textf(font, al_map_rgb(0, 0, 0), 0, start_y, 
      ALLEGRO_ALIGN_LEFT, "Use as setas para rolar a visualização da árvore");

    start_y += 16;

    al_draw_textf(font, al_map_rgb(0, 0, 0), 0, start_y, 
      ALLEGRO_ALIGN_LEFT, "Clique em um nó para selecioná-lo");

    start_y += 16;

    al_draw_textf(font, al_map_rgb(0, 0, 0), 0, start_y, 
      ALLEGRO_ALIGN_LEFT, "Aperte 'R' para adicionar um nó de valor aleatório a árvore");

    start_y += 16;

    al_draw_textf(font, al_map_rgb(0, 0, 0), 0, start_y, 
      ALLEGRO_ALIGN_LEFT, "Aperte 'D' para adicionar um nó de valor específico a árvore");

    start_y += 16;

    al_draw_textf(font, al_map_rgb(0, 0, 0), 0, start_y, 
      ALLEGRO_ALIGN_LEFT, "Aperte 'M' para remover o nó mínimo da árvore");

    start_y += 16;

    al_draw_textf(font, al_map_rgb(0, 0, 0), 0, start_y, 
      ALLEGRO_ALIGN_LEFT, "Aperte 'X' para remover o nó máximo da árvore");
  }
  else
  {
    al_draw_textf(font, al_map_rgb(0, 0, 0), 0, start_y, 
      ALLEGRO_ALIGN_LEFT, "Aperte 'A' para alterar a prioridade do nó");
    
    start_y += 16;

    al_draw_textf(font, al_map_rgb(0, 0, 0), 0, start_y, 
      ALLEGRO_ALIGN_LEFT, "Clique na parte vazia da tela para remover a seleção do nó");

    start_y += 16;

    al_draw_textf(font, al_map_rgb(0, 0, 0), 0, start_y, 
      ALLEGRO_ALIGN_LEFT, "Clique na parte vazia da tela para remover a seleção do nó");
  }
}

void ajustar()
{
  _size = 50;
  _angle = 5;

  if (heap_length > 3 && heap_length <= 7)
  {
    _size = 100;
  }
  else if (heap_length > 7 && heap_length <= 15)
  {
    _size = 200;
  }
  else if (heap_length > 15 && heap_length <= 31)
  {
    _size = 300;
  }
  else if (heap_length > 31 && heap_length <= 63)
  {
    _size = 400;
  }
  else if (heap_length > 63)
  {
    _size = 600;
  }
}

void adicionar_no(int modo)
{
  stringstream ss;
  ss << "Nó de valor ";

  if (heap_length < 10) 
    ss << "0";

  ss << heap_length << " adicionado.";
  
  heap_length++;
  mensagem = ss.str();

  ajustar();
}

int main(int argc, char **argv){

  int heap[] = {-1, 70, 67, 35, 48, 21, 23, 15, 8};
  heap_length = 1;

  if (!init())
    return -1;

  ajustar();

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
      else if (ev.keyboard.keycode == ALLEGRO_KEY_D)
      {
        adicionar_no(ESPECIFICO);
      }
      else if (ev.keyboard.keycode == ALLEGRO_KEY_R)
      {
        adicionar_no(ALEATORIO);
      }
    }
    else if (ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN)
    {
      int newSelected = selected;
      for (int i = 1; i <= heap_length; i++)
      {
        if ((Vector2D(ev.mouse.x, ev.mouse.y) - posNos[i]).size() <= DEFAULT_RADIUS)
        {
          newSelected = i;
          break;
        }
      }

      if (newSelected == selected)
        selected = -1;
      else
        selected = newSelected;
    }

    if (left)
    {
      camera += Vector2D(10, 0);

      if (camera.x > 820)
        camera = Vector2D(820, 0);
    }

    if (right)
    {
      camera += Vector2D(-10, 0);

      if (camera.x < -820)
        camera = Vector2D(-820, 0);
    }

    if(redraw && al_event_queue_is_empty(event_queue)) 
    {
      redraw = false;

      al_set_target_bitmap(buffer);
      al_clear_to_color(al_map_rgb(255, 255, 255));

      al_draw_textf(font, al_map_rgb(0, 0, 0), (SCREEN_W - DEFAULT_RADIUS * 2)/2, 0, 
        ALLEGRO_ALIGN_CENTRE, "Heap min-max");

      draw_menu(430);

      al_draw_textf(font, al_map_rgb(0, 0, 0), 0, SCREEN_H - 16, 
        ALLEGRO_ALIGN_LEFT, "Aperte 'ESC' para sair");

      al_draw_textf(font, al_map_rgb(255, 0, 0), 900, SCREEN_H - 16, 
        ALLEGRO_ALIGN_RIGHT, mensagem.c_str());

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