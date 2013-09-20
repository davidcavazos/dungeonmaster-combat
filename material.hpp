#ifndef MATERIAL_HPP
#define MATERIAL_HPP

#include <string>

typedef struct {
public:
  std::string name;
  size_t image;
  size_t pos_x;
  size_t pos_y;
  size_t n_x;
  size_t n_y;
  bool is_walkable;
} material;

#endif // MATERIAL_HPP
