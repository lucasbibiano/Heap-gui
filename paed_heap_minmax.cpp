#include <stdio.h>
#include <math.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_image.h>

#include <queue>

#include <time.h>

#include "Vector2D.cpp"

using namespace math;
using namespace std;

#define SCREEN_W 900
#define SCREEN_H 600
#define DEFAULT_RADIUS 12

#define NON_RANDOM 0
#define RANDOM 1
#define PARAM 2

//<variaveis de desenho>
ALLEGRO_DISPLAY* screen; //tela
ALLEGRO_FONT* font; //fonte usada

ALLEGRO_EVENT_QUEUE* event_queue = NULL; //fila de eventos
ALLEGRO_TIMER* timer = NULL; //timer
bool redraw = true; //se a tela precisa ser redesenhada

const float FPS = 60; //frames por segundo

int _size; //tamanho da linha que liga dois nos
int _angle; //angulo da linha em relacao a uma reta paralela a x

Vector2D camera; //camera para o scroll

string message; //mensagem no canto inferior

int selected; //no selecionado
int highlighted; //no destacado

Vector2D posNos[105]; //posicoes dos nos na tela para selecao
//</variaveis de desenho>

//<heap>
int heap[105];
int heap_length;
//</heap>

void heap_swap(int x, int y)
{
  int aux = heap[x];
  heap[x] = heap[y];
  heap[y] = aux;
}
//recupera o nivel de um no na posicao "i"
int nivel(int i)
{
  return (int) log2(i) + 1;
}

int maxDescendant(int i)
{
  int m = -1;

  if (i * 2 <= heap_length)
  {
    m = i * 2;

    if (heap[m + 1] > heap[m])
      m++;

    for (int k = i * 4; (k <= i * 4 + 3) && k <= heap_length; k++)
    {
      if (heap[k] > heap[m])
        m = k;
    }
  }

  return m;
}

int minDescendant(int i)
{
  int m = -1;

  if (i * 2 <= heap_length)
  {
    m = i * 2;

    if (heap[m + 1] < heap[m])
      m++;

    for (int k = i * 4; (k <= i * 4 + 3) && (k <= heap_length); k++)
    {
      if (heap[k] < heap[m])
        m = k;
    }
  }

  return m;
}

//"desce" um no de maximo, soh seria util se fosse possivel alterar a prioridade de um no
void descendMax(int i)
{
  if (i * 2 <= heap_length)
  {
    int m = maxDescendant(i);

    if (heap[i] < heap[m])
    {
      heap_swap(i, m);

      if (m >= i * 4)
      {
        int parent = m / 2;

        if (heap[parent] > heap[m])
          heap_swap(parent, m);

        descendMax(m);
      }
    }
  }
}

//"desce" um no de minimo
void descendMin(int i)
{
  if (i * 2 <= heap_length)
  {
    int m = minDescendant(i);

    if (heap[i] > heap[m])
    {
      heap_swap(i, m);

      if (m >= i * 4)
      {
        int parent = m / 2;

        if (heap[parent] < heap[m])
          heap_swap(parent, m);

        descendMin(m);
      }
    }
  }
}

void ascendMin(int i)
{
  int grand = i / 4;

  if (grand >= 1)
  {
    if (heap[i] < heap[grand])
    {
      heap_swap(grand, i);
      ascendMin(grand);
    }
  }
}

void ascendMax(int i)
{
  int grand = i / 4;

  if (grand >= 1)
  {
    if (heap[i] > heap[grand])
    {
      heap_swap(grand, i);
      ascendMax(grand);
    }
  }
}

void ascend(int i)
{
  int parent = i / 2;

  if (nivel(i) % 2 == 1)
  {
    if (parent >= 1)
    {
      if (heap[i] > heap[parent])
      {
        heap_swap(i, parent);
        ascendMax(parent);
      }
      else
      {
        ascendMin(i);
      }
    }
  }
  else
  {
    if (parent >= 1)
    {
      if (heap[i] < heap[parent])
      {
        heap_swap(i, parent);
        ascendMin(parent);
      }
      else
      {
        ascendMax(i);
      }
    }
  }
}

//inicializa o programa, retorna false em caso de falha
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

//dado um vetor representando o heap, desenha-o na tela, e guarda as posicoes
//onde os nos foram desenhados na variavel posNos
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
    else if (highlighted == i)
      al_draw_circle(start.x, start.y, DEFAULT_RADIUS, al_map_rgb(255, 255, 0), 2);   
    else
      al_draw_circle(start.x, start.y, DEFAULT_RADIUS, al_map_rgb(0, 255, 0), 2);   

    al_draw_textf(font, al_map_rgb(0, 0, 0), start.x, start.y - DEFAULT_RADIUS + 3, 
      ALLEGRO_ALIGN_CENTRE, "%02d", heap[i]);

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

//desenha o menu
void draw_menu(int start_y)
{
  if (selected == -1)
  {
    al_draw_text(font, al_map_rgb(0, 0, 0), 0, start_y, 
      ALLEGRO_ALIGN_LEFT, "Use as setas para rolar a visualização da árvore");

    start_y += 16;

    al_draw_text(font, al_map_rgb(0, 0, 0), 0, start_y, 
      ALLEGRO_ALIGN_LEFT, "Clique em um nó para selecioná-lo");

    start_y += 16;

    al_draw_text(font, al_map_rgb(0, 0, 0), 0, start_y, 
      ALLEGRO_ALIGN_LEFT, "Aperte 'R' para adicionar um nó de valor aleatório a árvore");

    start_y += 16;

    al_draw_text(font, al_map_rgb(0, 0, 0), 0, start_y, 
      ALLEGRO_ALIGN_LEFT, "Aperte 'D' para adicionar um nó de valor específico a árvore");

    start_y += 16;

    al_draw_text(font, al_map_rgb(0, 0, 0), 0, start_y, 
      ALLEGRO_ALIGN_LEFT, "Aperte 'M' para remover o nó mínimo da árvore");

    start_y += 16;

    al_draw_text(font, al_map_rgb(0, 0, 0), 0, start_y, 
      ALLEGRO_ALIGN_LEFT, "Aperte 'X' para remover o nó máximo da árvore");

    start_y += 16;

    al_draw_text(font, al_map_rgb(0, 0, 0), 0, start_y, 
      ALLEGRO_ALIGN_LEFT, "Aperte 'K' para criar o heap padrão {70, 67, 35, 48, 21, 23, 15 e 8}");
  }
  else
  {
    al_draw_text(font, al_map_rgb(0, 0, 0), 0, start_y, 
      ALLEGRO_ALIGN_LEFT, "Nenhuma operação pode ser realizada sobre o nó");
  }
}

//solicita entrada do usuario
int input(string message, int start_y)
{
  int str_width = al_get_text_width(font, message.c_str());
  ALLEGRO_TIMER* aux_timer = al_create_timer(1.0/FPS);
  char in[3]; 
  bool aux_redraw = true;
  int ticks = 0;
  bool draw_cursor = true;

  bool left = false;
  bool right = false;

  int index = 0;

  in[index] = '\0';

  while (true)
  {
    ALLEGRO_EVENT ev;

    al_wait_for_event(event_queue, &ev);

    if(ev.type == ALLEGRO_EVENT_TIMER) 
    {
      aux_redraw = true;
      ticks++;

      if (ticks >= (FPS/2))
      {
        ticks = 0;
        draw_cursor = !draw_cursor;
      }
    }
    else if (ev.type == ALLEGRO_EVENT_KEY_DOWN)
    {
      if (ev.keyboard.keycode == ALLEGRO_KEY_LEFT)
      {
        left = true;
      }
      else if (ev.keyboard.keycode == ALLEGRO_KEY_RIGHT)
      {
        right = true;
      }
      else
      {
        if (ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE)
          return -1;
      }
    }
    else if (ev.type == ALLEGRO_EVENT_KEY_UP)
    {
      if (ev.keyboard.keycode == ALLEGRO_KEY_LEFT)
        left = false;
      else if (ev.keyboard.keycode == ALLEGRO_KEY_RIGHT)
        right = false;
      else
      {
        int keycode = ev.keyboard.keycode;

        if (keycode >= ALLEGRO_KEY_0 && keycode <= ALLEGRO_KEY_9)
        {
          if (index < 2)
          {
            in[index] = (char) (keycode + 21);
            index++;
            in[index] = '\0';
          }
        }
        else if (keycode == ALLEGRO_KEY_BACKSPACE)
        {
          if (index > 0)
          {
            index--;
            in[index] = '\0';
          }
        }
        else if (keycode == ALLEGRO_KEY_ENTER)
        {
          break;
        }
      }
    }

    if (left)
    {
      camera += Vector2D(10, 0) * (30/FPS);

      if (camera.x > 820)
        camera = Vector2D(820, 0);
    }

    if (right)
    {
      camera += Vector2D(-10, 0) * (30/FPS);

      if (camera.x < -820)
        camera = Vector2D(-820, 0);
    }

    if(aux_redraw && al_event_queue_is_empty(event_queue)) 
    {
      aux_redraw = false;
      al_set_target_bitmap(al_get_backbuffer(screen));
      al_clear_to_color(al_map_rgb(255, 255, 255));

      al_draw_text(font, al_map_rgb(0, 0, 0), (SCREEN_W - DEFAULT_RADIUS * 2)/2, 0, 
        ALLEGRO_ALIGN_CENTRE, "Heap min-max");

      al_draw_text(font, al_map_rgb(0, 0, 0), 0, SCREEN_H - 16, 
        ALLEGRO_ALIGN_LEFT, "Aperte 'ESC' para cancelar");

      draw_heap(heap);

      al_draw_text(font, al_map_rgb(0, 0, 0), 0, start_y, 
        ALLEGRO_ALIGN_LEFT, message.c_str());

      al_draw_text(font, al_map_rgb(0, 0, 0), str_width, start_y, 
        ALLEGRO_ALIGN_LEFT, in);

      if (draw_cursor)
      {
        al_draw_text(font, al_map_rgb(0, 0, 0), str_width + (index * 10), start_y, 
          ALLEGRO_ALIGN_LEFT, "|");   
      }

      al_flip_display();
    }
  }

  al_destroy_timer(aux_timer);

  return index == 2 ? (in[0] - 48) * 10 + (in[1] - 48) : in[0] - 48;
}

//ajusta tamanho e angulo do traco que liga dois nos de acordo com a quantidade
//de nos
void adjust()
{
  _size = 50;
  _angle = 5;

  if (heap_length > 3 && heap_length <= 7)
  {
    _size = 75;
  }
  else if (heap_length > 7 && heap_length <= 15)
  {
    _size = 125;
  }
  else if (heap_length > 15 && heap_length <= 31)
  {
    _size = 230;
  }
  else if (heap_length > 31 && heap_length <= 63)
  {
    _size = 350;
  }
  else if (heap_length > 63)
  {
    _size = 570;
  }
} 


int random_number()
{
  return rand() % 100;
}
//adiciona um no de valor aleatorio ou um no especifico de acordo
//com o valor de "mode" (RANDOM ou NON_RANDOM, PARAM)
void add_node(int mode, int param)
{
  if (heap_length > 100)
  {
    message = "Árvore cheia.";
  }
  else
  {
    int r;
    stringstream ss;

    if (mode == RANDOM)
    {
      r = random_number();
    }
    else if (mode == NON_RANDOM)
    {
      r = input("Digite o valor do novo nó:", 416);

      if (r == -1)
        return;
    }
    else if (mode == PARAM)
    {
      r = param;
    }

    ss << "Nó de valor ";

    if (r < 10) 
      ss << "0";

    ss << r << " adicionado.";
    message = ss.str();
    
    heap_length++;
    heap[heap_length] = r;

    ascend(heap_length);

    adjust();
  }
}

//desenha o bitmap na memoria
void draw()
{
  al_set_target_bitmap(al_get_backbuffer(screen));
  al_clear_to_color(al_map_rgb(255, 255, 255));

  al_draw_textf(font, al_map_rgb(0, 0, 0), (SCREEN_W - DEFAULT_RADIUS * 2)/2, 0, 
    ALLEGRO_ALIGN_CENTRE, "Heap min-max");

  draw_menu(430);

  al_draw_textf(font, al_map_rgb(0, 0, 0), 0, SCREEN_H - 16, 
    ALLEGRO_ALIGN_LEFT, "Aperte 'ESC' para sair");

  al_draw_text(font, al_map_rgb(255, 0, 0), 900, SCREEN_H - 16, 
    ALLEGRO_ALIGN_RIGHT, message.c_str());

  draw_heap(heap);
}

int pop_min()
{
  int result = -1;

  if (heap_length > 0)
  {
    stringstream ss;
    result = heap[1];
    heap[1] = heap[heap_length];

    /* 
    //Esse codigo da uma parada da hora de descer o elemento
    //E atualiza a mensagem
    ss << "Ultimo elemento de valor " << (heap[1] < 10 ? "0" : "") << heap[1];
    ss << " foi para a raiz da árvore. Executando descida.";

    message = ss.str();

    draw();
    al_flip_display();

    al_rest(5.0);
    */

    heap_length--;
    descendMin(1);
  }
  else
  {
    message = "Árvore vazia, nada para remover.";
  }

  adjust();

  return result;
}

int pop_max()
{
  int result = -1;

  if (heap_length > 0)
  {
    stringstream ss;

    if (heap[2] > heap[3])
    {
      result = heap[2];
      heap[2] = heap[heap_length];

      heap_length--;
      descendMax(2);
    }
    else
    {
      result = heap[3];
      heap[3] = heap[heap_length];

      heap_length--;
      descendMax(3);
    }    
  }
  else
  {
    message = "Árvore vazia, nada para remover.";
  }

  adjust();

  return result;
}

int main(int argc, char **argv){

  srand(time(NULL));

  heap_length = 0;

  if (!init())
    return -1;

  adjust();

  bool left = false; //seta pra esquerda pressionada
  bool right = false; //seta pra direita pressionada

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
      {
        left = true;
      }
      else if (ev.keyboard.keycode == ALLEGRO_KEY_RIGHT)
      {
        right = true;
      }
      else
      {
        if (selected == -1)
        {
          if (ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE)
            break;
        }
      }
    }
    else if (ev.type == ALLEGRO_EVENT_KEY_UP)
    {
      if (ev.keyboard.keycode == ALLEGRO_KEY_LEFT)
        left = false;
      else if (ev.keyboard.keycode == ALLEGRO_KEY_RIGHT)
        right = false;
      else
      {
        if (selected == -1)
        {
          if (ev.keyboard.keycode == ALLEGRO_KEY_R)
          {
            add_node(RANDOM, 0);
          }
          else if (ev.keyboard.keycode == ALLEGRO_KEY_D)
          {
            add_node(NON_RANDOM, 0);
          }  
          else if (ev.keyboard.keycode == ALLEGRO_KEY_K)
          {
            int heap_padrao[] = {70, 67, 35, 48, 21, 23, 15, 8};
            heap_length = 0;

            for (int i = 0; i < 8; i++)
              add_node(PARAM, heap_padrao[i]);
          }  
          else if (ev.keyboard.keycode == ALLEGRO_KEY_M)
          {
            int value = pop_min();

            if (value != -1)
            {
              stringstream ss;

              ss << "Nó de valor " << (value < 10 ? "0" : "") << value << " removido.";

              message = ss.str();   
            }

          }  
          else if (ev.keyboard.keycode == ALLEGRO_KEY_X)
          {
            int value = pop_max();

            if (value != -1)
            {
              stringstream ss;

              ss << "Nó de valor " << (value < 10 ? "0" : "") << value << " removido.";

              message = ss.str();   
            }
          }  

        }
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
      camera += Vector2D(10, 0) * (30/FPS);

      if (camera.x > 820)
        camera = Vector2D(820, 0);
    }

    if (right)
    {
      camera += Vector2D(-10, 0) * (30/FPS);

      if (camera.x < -820)
        camera = Vector2D(-820, 0);
    }

    if(redraw && al_event_queue_is_empty(event_queue)) 
    {
      redraw = false;
      draw();
      al_flip_display();
    }
  }

  al_destroy_timer(timer);
  al_destroy_display(screen);
  al_destroy_font(font);
  al_destroy_event_queue(event_queue);
 
  return 0;
}