#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "hashtable.h"
#include "holdall.h"

#define STR(s)  #s
#define XSTR(s) STR(s)

#define WORD_LENGTH_MAX       63
#define COUNT_MAX             999
#define DEFAULT_WORDS_DISPLAY 30
#define OPTION_WORDS_DISPLAY  "-t"
#define TYPE_OF_FILE          ".txt"
#define IN_FILE               "x"
#define NOT_IN_FILE           "-"

//  str_hashfun : l'une des fonctions de pré-hachage conseillées par Kernighan
//    et Pike pour les chaines de caractères
static size_t str_hashfun(const char *s);

//  scptr_display : affiche sur la sortie standard *cptr, le caractère
//    tabulation, la chaine de caractères pointée par s et la fin de ligne.
//    Renvoie zéro en cas de succès, une valeur non nulle en cas d'échec
static int scptr_display(const char *count, const char *s, const char *cptr);

//  rfree : libère la zone mémoire pointée par ptr et renvoie zéro
static int rfree(void *ptr);

// compar : fonction de comparaison
static int compar(const void *ptr1, const void *ptr2);

hashtable *ht;
size_t nb_words_display = DEFAULT_WORDS_DISPLAY;

int main(int argc, char *argv[]) {
  char count[COUNT_MAX + 1];
  hashtable *ht_all = hashtable_empty((int (*)(const void *,
          const void *))strcmp,
          (size_t (*)(const void *))str_hashfun);
  ht = hashtable_empty((int (*)(const void *, const void *))strcmp,
          (size_t (*)(const void *))str_hashfun);
  holdall *has_all = holdall_empty();
  holdall *has = holdall_empty();
  size_t nb_files = 0;
  if (ht_all == NULL || ht == NULL || has == NULL || has_all == NULL) {
    goto error_capacity;
  }
  for (int i = 1; i < argc; ++i) {
    if (strstr(argv[i], TYPE_OF_FILE) != NULL) {
      FILE *F = fopen(argv[i], "r");
      if (F == NULL) {
        goto error_open_file;
      }
      ++nb_files;
      char w[WORD_LENGTH_MAX + 1];
      // Ajout de tout les mots lus dans le fichier F dans la table de hashage
      // ht et dans le fourretout has s'il apparait déjà dans ht_all
      if (nb_files > 1) {
        while (fscanf(F, "%" XSTR(WORD_LENGTH_MAX) "s", w) == 1) {
          if (strlen(w) == WORD_LENGTH_MAX) {
            int c = fgetc(F);
            if (!isspace(c)) {
              while (c != EOF && !isspace(c)) {
                c = fgetc(F);
              }
              fprintf(stderr, "*** Warning: Word '%s...' is truncated.\n", w);
            }
          }
          char *cptr = (char *) hashtable_search(ht_all, w);
          if (cptr != NULL) {
            if (cptr - count < COUNT_MAX) {
              char *s = malloc(strlen(w) + 1);
              if (s == NULL) {
                free(s);
                goto error_capacity;
              }
              strcpy(s, w);
              if (hashtable_search(ht, s) == NULL) {
                if (holdall_put(has, s) != 0) {
                  free(s);
                  goto error_capacity;
                }
              }
              if (hashtable_add(ht, s, cptr + 1) == NULL) {
                goto error_capacity;
              }
            }
          }
        }
        if (i != argc - 1) {
          rewind(F);
        }
      }
      // Ajout de tous les mots lus sur le fichier F dans la table ht_all, et
      // ajout de tous les mots différents lus sur le fichier F dans le
      // fourretout has_all
      if (i != argc - 1) {
        while (fscanf(F, "%" XSTR(WORD_LENGTH_MAX) "s", w) == 1) {
          if (strlen(w) == WORD_LENGTH_MAX) {
            int c = fgetc(F);
            if (!isspace(c)) {
              while (c != EOF && !isspace(c)) {
                c = fgetc(F);
              }
              fprintf(stderr, "*** Warning: Word '%s...' is truncated.\n", w);
            }
          }
          char *cptr = (char *) hashtable_search(ht_all, w);
          if (cptr != NULL) {
            if (cptr - count < COUNT_MAX) {
              if (hashtable_add(ht_all, w, cptr + 1) == NULL) {
                goto error_capacity;
              }
            }
          } else {
            char *s = malloc(strlen(w) + 1);
            if (s == NULL) {
              free(s);
              goto error_capacity;
            }
            strcpy(s, w);
            if (holdall_put(has_all, s) != 0) {
              free(s);
              goto error_capacity;
            }
            if (hashtable_add(ht_all, s, count + 1) == NULL) {
              goto error_capacity;
            }
          }
        }
      }
      if (!feof(F)) {
        goto error_read_file;
      }
      if (fclose(F) != 0) {
        goto error_close_file;
      }
    }
  }
  if (holdall_sort(has, (int (*)(const void *, const void *))compar) != 0) {
    goto error_capacity;
  }
  if (holdall_apply_context2(has,
      ht, (void *(*)(void *, void *))hashtable_search,
      count, (int (*)(void *, void *, void *))scptr_display) != 0) {
    goto error_write;
  }
#ifdef HASHTABLE_CHECKUP
  hashtable_display_checkup(ht, stderr);
#endif
  int r = EXIT_SUCCESS;
  goto dispose;
error_read_file:
  fprintf(stderr, "*** Error: A read error occurs on file\n");
  r = EXIT_FAILURE;
  goto dispose;
error_open_file:
  fprintf(stderr, "*** Error: Cannot open file.\n");
  r = EXIT_FAILURE;
  goto dispose;
error_close_file:
  fprintf(stderr, "*** Error: Cannot close file.\n");
  r = EXIT_FAILURE;
  goto dispose;
error_capacity:
  fprintf(stderr, "*** Error: Not enough memory.\n");
  r = EXIT_FAILURE;
  goto dispose;
error_write:
  fprintf(stderr, "*** Error: A write error occurs.\n");
  r = EXIT_FAILURE;
  goto dispose;
dispose:
  hashtable_dispose(&ht_all);
  hashtable_dispose(&ht);
  if (has_all != NULL) {
    holdall_apply(has_all, rfree);
  }
  holdall_dispose(&has_all);
  if (has != NULL) {
    holdall_apply(has, rfree);
  }
  holdall_dispose(&has);
  return r;
}

size_t str_hashfun(const char *s) {
  size_t h = 0;
  for (const unsigned char *p = (const unsigned char *) s; *p != '\0'; ++p) {
    h = 37 * h + *p;
  }
  return h;
}

int scptr_display(const char *count, const char *s, const char *cptr) {
  if (nb_words_display == 0) {
    return 0;
  }
  --nb_words_display;
  if (cptr - count == COUNT_MAX) {
    return printf("*** Warning: Count overflow :\t%d\t%s\n", COUNT_MAX, s) < 0;
  }
  return printf("%td\t%s\n", cptr - count, s) < 0;
}

int rfree(void *ptr) {
  free(ptr);
  return 0;
}

int compar(const void *ptr1, const void *ptr2) {
  if (hashtable_search(ht, ptr1) < hashtable_search(ht, ptr2)) {
    return 1;
  }
  if (hashtable_search(ht, ptr1) == hashtable_search(ht, ptr2)) {
    if (strcmp(ptr1, ptr2) > 0) {
      return 1;
    }
    if (strcmp(ptr1, ptr2) < 0) {
      return -1;
    }
    return 0;
  }
  return -1;
}
