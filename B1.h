#ifndef B1_H
#define B1_H
void B1();

#ifndef STATIC_LIBRARY
#define B1_CPP "B1.cpp"
#include B1_CPP
#endif

#endif
