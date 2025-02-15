#ifndef LOAD_OBJ_H
#define LOAD_OBJ_H

/* +-----------------------------------------------------+ *
 * | Author: Hugo Coto Florez                            | *
 * | Contact: hugo.coto@rai.usc.es                       | *
 * | Repo: github.com/hugocotoflorez/load_obj            | *
 * | Licenseless: Free open source without any warranty. | *
 * +-----------------------------------------------------+ */

/* QUIET possible values
 * 1: Supress any output
 * 0: Show loaded object info */
#define QUIET 0

/* DEBUG possible values.
 * 1: show info about loading process
 * 0: do not show debugging info */
#define DEBUG 0

/* Load wavefront (.obj) file (filename) into vao.
 * You can change DEBUG and QUIET variables here */
void load_obj(const char *filename, int *vao);

#endif
