#ifndef LAGOR_H
#define LAGOR_H

class Lagor {
 private:
  int dio;

 public:
  Lagor();
  Lagor(int d);
  const char* get();
};

#endif