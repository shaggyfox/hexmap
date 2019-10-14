#include <stdio.h>
#include <stdlib.h>
#include <math.h>

struct map {
  int w;
  int h;
  unsigned char *map;
};

static void map_init(struct map *map, int w, int h)
{
  map->w = w;
  map->h = h;
  map->map = calloc(1, w * h);
}

static void map_free(struct map *map)
{
  free(map->map);
}

static int map_normalize_x(struct map* map, int x)
{
  while (x < 0) {
    x = x + map->w;
  }
  while (x >= map->w) {
    x = x - map->w;
  }
  return x;
}

static int map_get(struct map *map, int x, int y)
{
  if (y < 0 || y >= map->h) {
    return -1;
  }
  x = map_normalize_x(map, x);
  return map->map[x + y * map->w];
}

static int map_set(struct map *map, int x, int y, int v)
{
  if ( y < 0 || y >= map->h) {
    return -1;
  }
  x = map_normalize_x(map, x);
  map->map[x + y * map->w] = v;
  return 0;
}

static void map_print(struct map *map)
{
  for (int y = 0; y < map->h; ++y) {
    for (int x = 0; x < map->w; ++x) {
      int v = map->map[y * map->w + x];
      fprintf(stdout, "%x", v);
    }
    fputs("\n", stdout);
  }
}

static void map_random(struct map *map)
{
  int max = map->w * map->h;
  for (int i = 0; i < max; ++i) {
    map->map[i] = random() & 255;
  }
}

struct vector {
  float x;
  float y;
};

static void vector_normalize(struct vector *out)
{
  float length = sqrtf(out->x * out->x + out->y * out->y);
  out->x /= length;
  out->y /= length;
}

static void vector_from_int(int v, struct vector *out)
{
  out->x = (float)(v & 15);
  out->y = (float)((v >> 4) & 15);
  out->x -= 7.5;
  out->y -= 7.5;
  vector_normalize(out);
}

static void perlin_map_get(struct map *map, int x, int y,
    struct vector *a, struct vector *b, struct vector *c, struct vector *d)
{
  vector_from_int(map_get(map, x + 0, y + 0), a);
  vector_from_int(map_get(map, x + 1, y + 0), b);
  vector_from_int(map_get(map, x + 1, y + 1), c);
  vector_from_int(map_get(map, x + 0, y + 1), d);
}

static float dot_product(struct vector* a, struct vector *b)
{
  return a->x * b->x + a->y * b->y;
}

static float lerp(float v0, float v1, float t) {
  return (1.0 - t) * v0 + t * v1;
}

static float fade(float t) {
    return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
}

void perlin_noise2d(int w, int h, int divisor, float *out)
{
  /* first initialize a new map with all perlin vector data */
  struct map vector_data;
  int vector_map_width = w / divisor;
  int vector_map_height = h / divisor;
  map_init(&vector_data, vector_map_width, vector_map_height + 1);
  map_random(&vector_data);
  for (int y = 0; y < vector_map_height; ++y) {
    for (int x = 0; x < vector_map_width; ++x) {
      struct vector a;
      struct vector b;
      struct vector c;
      struct vector d;
      perlin_map_get(&vector_data, x, y, &a, &b, &c, &d);
      for (int sub_y = 0; sub_y < divisor; ++sub_y) {
        for (int sub_x = 0; sub_x < divisor; ++sub_x) {
          struct vector position;
          position.x = sub_x / (float)(divisor - 1);
          position.y = sub_y / (float)(divisor - 1);
          float horizontal_t = fade(position.x);
          float vertical_t = fade(position.y);
          float len = sqrtf(2);
          position.x *= len;
          position.y *= len;
          struct vector position_a;
          struct vector position_b;
          struct vector position_c;
          struct vector position_d;
          position_a.x = position.x;
          position_a.y = position.y;

          position_b.x = position.x - len;
          position_b.y = position.y;

          position_c.x = position.x - len;
          position_c.y = position.y - len;

          position_d.x = position.x;
          position_d.y = position.y - len;

          float dot_a = dot_product(&a, &position_a);
          float dot_b = dot_product(&b, &position_b);
          float dot_c = dot_product(&c, &position_c);
          float dot_d = dot_product(&d, &position_d);

          float dot_ab = lerp(dot_a, dot_b, horizontal_t);
          float dot_dc = lerp(dot_d, dot_c, horizontal_t);
          float result = lerp(dot_ab, dot_dc, vertical_t);
          //map_set(map, x * divisor + sub_x, y * divisor + sub_y, (result + 1.0) / 2.0 * 10);
          out[x * divisor + sub_x + (y * divisor + sub_y) * w] = result;
        }
      }
    }
  }
  map_free(&vector_data);
}
