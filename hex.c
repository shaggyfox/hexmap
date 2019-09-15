#include <stdio.h>

struct screen_pos {
  int x;
  int y;
};

struct map_pos {
  float x;
  float y;
};

struct cube_pos {
  float x;
  float y;
  float z;
};

#define WIDTH 12
#define HEIGHT 8

void cube2screen(struct cube_pos* c_pos, struct screen_pos *s_pos)
{
  s_pos->x = ((c_pos->x - c_pos->y) * WIDTH) / 2;
  s_pos->y = ((c_pos->x + c_pos->y) * 3 * HEIGHT) / 4;
}

void screen2cube(struct screen_pos *s_pos, struct cube_pos *c_pos)
{
  c_pos->x = (2 * s_pos->y) / (3 * HEIGHT) + (s_pos->x / WIDTH);
  c_pos->y = (2 * s_pos->y) / (3 * HEIGHT) - (s_pos->y / WIDTH);
  c_pos->z = -c_pos->x - c_pos->y;
}

void cube2map(struct cube_pos *c_pos, struct map_pos *m_pos)
{
  m_pos->x = c_pos->x + c_pos->z;
  m_pos->y = -c_pos->y;
}

void map2cube(struct map_pos *m_pos, struct cube_pos *c_pos)
{
  c_pos->x = m_pos->x + m_pos->y;
  c_pos->y = -m_pos->x;
  c_pos->z = -m_pos->y;
}

void print_screen_pos(struct screen_pos *s_pos)
{
  printf("screen: %ix, %iy\n", s_pos->x, s_pos->y);
}

void print_cube_pos(struct cube_pos *c_pos)
{
  printf("cube: %0.2fx, %0.2fy, %0.2fz\n",
      c_pos->x, c_pos->y, c_pos->z);
}

void print_map_pos(struct map_pos *m_pos)
{
  printf("map: %0.2fx, %0.2fy\n", m_pos->x, m_pos->y);
}

void read_screen_pos(struct screen_pos *s_pos)
{
  scanf("%i %i", &s_pos->x, &s_pos->y);
}

void read_cube_pos(struct cube_pos *c_pos)
{
  scanf("%f %f %f", &c_pos->x, &c_pos->y, &c_pos->z);
}

void read_map_pos(struct map_pos *m_pos)
{
  scanf("%f %f", &m_pos->x, &m_pos->y);
}

int main()
{
  while(1) {
    struct screen_pos s_pos;
    struct cube_pos c_pos;
    struct map_pos m_pos;
    read_map_pos(&m_pos);
    map2cube(&m_pos, &c_pos);
    cube2screen(&c_pos, &s_pos);

    print_map_pos(&m_pos);
    print_cube_pos(&c_pos);
    print_screen_pos(&s_pos);

  }
  return 0;
}
