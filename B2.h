#ifndef B2_H
#define B2_H
void B2();

#ifndef STATIC_LIBRARY
#define B2_CPP "B2.cpp"
#include B2_CPP
#endif

#endif
