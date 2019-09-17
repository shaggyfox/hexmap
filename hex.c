#include "hex.h"
#include <stdio.h>
#include <math.h>

void cube_round(struct cube_pos *c_pos)
{
  float rx = roundf(c_pos->x);
  float ry = roundf(c_pos->y);
  float rz = roundf(c_pos->z);

  float x_diff = fabs(rx - c_pos->x);
  float y_diff = fabs(ry - c_pos->y);
  float z_diff = fabs(rz - c_pos->z);

  if (x_diff > y_diff && x_diff > z_diff) {
    rx = -ry-rz;
  } else if (y_diff > z_diff) {
    ry = -rx-rz;
  } else {
    rz = -rx-ry;
  }
  c_pos->x = rx;
  c_pos->y = ry;
  c_pos->z = rz;
}

void map_to_offset(struct map_pos *m_pos)
{
  m_pos->x += floorf(m_pos->y / 2);
}

void map_from_offset(struct map_pos *m_pos)
{
  m_pos->x -= floorf(m_pos->y / 2);
}

void cube2screen(struct cube_pos* c_pos, struct screen_pos *s_pos)
{
  s_pos->x = ((c_pos->x - c_pos->y) * WIDTH) / 2;
  s_pos->y = ((c_pos->x + c_pos->y) * 3 * HEIGHT) / 4;
}

void screen2cube(struct screen_pos *s_pos, struct cube_pos *c_pos)
{
  c_pos->x = (float)(2 * s_pos->y) / (float)(3 * HEIGHT) + (float)(s_pos->x / WIDTH);
  c_pos->y = (float)(2 * s_pos->y) / (float)(3 * HEIGHT) - (float)(s_pos->x / WIDTH);
  c_pos->z = -c_pos->x - c_pos->y;
}

void cube2map(struct cube_pos *c_pos, struct map_pos *m_pos)
{
  m_pos->x = c_pos->x + c_pos->z;
  m_pos->y = -c_pos->z;
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
/*
int main()
{
  while(1) {
    struct screen_pos s_pos;
    struct cube_pos c_pos;
    struct map_pos m_pos;

    read_screen_pos(&s_pos);
    screen2cube(&s_pos, &c_pos);
    cube2map(&c_pos, &m_pos);

    print_map_pos(&m_pos);
    print_cube_pos(&c_pos);
    print_screen_pos(&s_pos);

  }
  return 0;
}*/
