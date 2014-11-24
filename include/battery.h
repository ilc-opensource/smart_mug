#ifndef BATTERY_H
#define BATTERY_H

typedef struct _V2P_t {
  int percent;
  int v;
}V2P_t;

V2P_t v2p_table[] = {
  { 99 , 2772 },
  { 95 , 2735 },
  { 90 , 2705 },
  { 85 , 2677 },
  { 80 , 2648 },
  { 75 , 2623 },
  { 70 , 2599 },
  { 65 , 2573 },
  { 60 , 2554 },
  { 55 , 2538 },
  { 50 , 2524 },
  { 45 , 2511 },
  { 40 , 2501 },
  { 35 , 2491 },
  { 30 , 2481 },
  { 25 , 2473 },
  { 20 , 2462 },
  { 15 , 2448 },
  { 10 , 2431 },
  { 5 , 2418 },
  { 2 , 2333 },
  { 1 , 2267 },
};

const int v2p_table_len = sizeof(v2p_table) / sizeof(V2P_t);

#endif
