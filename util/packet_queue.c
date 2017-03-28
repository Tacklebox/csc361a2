#include "packet_queue.h"


static int cmp_pri(pqueue_pri_t next, pqueue_pri_t curr) {
  return (next > curr);
}

static pqueue_pri_t get_pri(void *a) {
  return ((node_t *) a)->pri;
}

static void set_pri(void *a, pqueue_pri_t pri) {
  ((node_t *) a)->pri = pri;
}

static size_t get_pos(void *a) {
  return ((node_t *) a)->pos;
}

static void set_pos(void *a, size_t pos) {
  ((node_t *) a)->pos = pos;
}

int initialise_queue() {
  pq = pqueue_init(32, cmp_pri, get_pri, set_pri, get_pos, set_pos);
  return (pq != NULL);
}
