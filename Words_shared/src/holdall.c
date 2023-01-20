//  Partie implantation du module holdall

#include "holdall.h"

//  struct holdall, holdall : implantation par liste dynamique simplement
//    chainée. L'insertion a lieu en queue si la macroconstante
//    HOLDALL_INSERT_TAIL est définie, en tête sinon

typedef struct choldall choldall;

struct choldall {
  void *value;
  choldall *next;
};

struct holdall {
  choldall *head;
#ifdef HOLDALL_INSERT_TAIL
  choldall *tail;
#endif
  size_t count;
};

holdall *holdall_empty(void) {
  holdall *ha = malloc(sizeof *ha);
  if (ha == NULL) {
    return NULL;
  }
  ha->head = NULL;
#ifdef HOLDALL_INSERT_TAIL
  ha->tail = NULL;
#endif
  ha->count = 0;
  return ha;
}

int holdall_put(holdall *ha, void *ptr) {
  choldall *p = malloc(sizeof *p);
  if (p == NULL) {
    return -1;
  }
  p->value = ptr;
#ifdef HOLDALL_INSERT_TAIL
  p->next = NULL;
  if (ha->tail == NULL) {
    ha->head = p;
  } else {
    ha->tail->next = p;
  }
  ha->tail = p;
#else
  p->next = ha->head;
  ha->head = p;
#endif
  ha->count += 1;
  return 0;
}

size_t holdall_count(holdall *ha) {
  return ha->count;
}

int holdall_apply(holdall *ha, int (*fun)(void *)) {
  const choldall *p = ha->head;
  while (p != NULL) {
    int r = fun(p->value);
    if (r != 0) {
      return r;
    }
    p = p->next;
  }
  return 0;
}

int holdall_apply_context(holdall *ha,
    void *context, void *(*fun1)(void *context, void *ptr),
    int (*fun2)(void *ptr, void *resultfun1)) {
  const choldall *p = ha->head;
  while (p != NULL) {
    int r = fun2(p->value, fun1(context, p->value));
    if (r != 0) {
      return r;
    }
    p = p->next;
  }
  return 0;
}

int holdall_apply_context2(holdall *ha,
    void *context1, void *(*fun1)(void *context1, void *ptr),
    void *context2, int (*fun2)(void *context2, void *ptr, void *resultfun1)) {
  const choldall *p = ha->head;
  while (p != NULL) {
    int r = fun2(context2, p->value, fun1(context1, p->value));
    if (r != 0) {
      return r;
    }
    p = p->next;
  }
  return 0;
}

void holdall_dispose(holdall **haptr) {
  if (*haptr == NULL) {
    return;
  }
  choldall *p = (*haptr)->head;
  while (p != NULL) {
    choldall *t = p;
    p = p->next;
    free(t);
  }
  free(*haptr);
  *haptr = NULL;
}

// choldall__split : sépare la liste src en deux listes dest1 et dest2 de taille
//                  égale à un élément près
// AE : src != NULL
void choldall__split(choldall *src, choldall **dest1, choldall **dest2) {
  choldall *slow = src;
  choldall *fast = src->next;
  while (fast != NULL) {
    fast = fast->next;
    if (fast != NULL) {
      slow = slow->next;
      fast = fast->next;
    }
  }
  *dest1 = src;
  *dest2 = slow->next;
  slow->next = NULL;
}

// choldall__merge : fusionne les deux listes src1 et src2 dans la liste dest
//                   en triant les éléments selon compar
void choldall__merge(choldall *src1, choldall *src2, choldall **dest,
    int (*compar)(const void *, const void *)) {
  while (src1 != NULL || src2 != NULL) {
    if (src1 == NULL) {
      *dest = src2;
      src2 = src2->next;
    } else if (src2 == NULL) {
      *dest = src1;
      src1 = src1->next;
    } else if (compar(src1->value, src2->value) <= 0) {
      *dest = src1;
      src1 = src1->next;
    } else {
      *dest = src2;
      src2 = src2->next;
    }
    dest = &(*dest)->next;
  }
}

// choldall__sort : sépare la liste src en plusieurs listes d'au plus 2 éléments
//                  puis fais appel à la procédure choldall__merge
void choldall__sort(choldall **src, int (*compar)(const void *, const void *)) {
  if (*src == NULL || (*src)->next == NULL) {
    return;
  }
  choldall *p1;
  choldall *p2;
  choldall__split(*src, &p1, &p2);
  choldall__sort(&p1, compar);
  choldall__sort(&p2, compar);
  choldall__merge(p1, p2, src, compar);
}

int holdall_sort(holdall *ha, int (*compar)(const void *, const void *)) {
  choldall__sort(&ha->head, compar);
  return 0;
}
