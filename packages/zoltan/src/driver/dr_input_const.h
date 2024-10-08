// @HEADER
// *****************************************************************************
//  Zoltan Toolkit for Load-balancing, Partitioning, Ordering and Coloring
//
// Copyright 2012 NTESS and the Zoltan contributors.
// SPDX-License-Identifier: BSD-3-Clause
// *****************************************************************************
// @HEADER


#ifndef _DR_INPUT_CONST_H_
#define _DR_INPUT_CONST_H_


#ifdef __cplusplus
/* if C++, define the rest of this header file as extern C */
extern "C" {
#endif

#ifndef _DR_CONST_H
#include "dr_const.h"
#endif
#include "dr_compress_const.h"

/* define the input file types */
#define NO_FILE_POINTS    0
#define NO_FILE_TRIANGLES 1
#define NO_FILE_GRAPH     2
#define NEMESIS_FILE      3
#define CHACO_FILE        4
#define HYPERGRAPH_FILE   5
#define MATRIXMARKET_FILE 6
#define MATRIXMARKET_PLUS_FILE 7

/* define matrix_obj options */
#define ROWS            0
#define COLUMNS         1
#define NONZEROS        2

/* define the initial distribution types - for Chaco file vertices
   and for hypergraph file pins  */

#define INITIAL_FILE   0   /* Chaco vertices or hypergraph pins */
#define INITIAL_LINEAR 1   /* Chaco vertices or hypergraph pins */
#define INITIAL_CYCLIC 2   /* Chaco vertices or hypergraph pins */
#define INITIAL_OWNER  3   /* Chaco vertices */
#define INITIAL_ROW    4   /* hypergraph pins */
#define INITIAL_COL    5   /* hypergraph pins */
#define INITIAL_ZERO   6   /* hypergraph pins */
#define INITIAL_NO_DIST 7   /* distribution not necessary */

#define MAX_INPUT_STR_LN 4096   /* maximum string length for read_string()  */


/* Structure used to store the information necessary for parallel I/O. */
struct Parallel_IO
{
  int     dsk_list_cnt;

  int    *dsk_list;
  int     rdisk;

  int     num_dsk_ctrlrs;            /* The number of disk controllers.     */
  int     pdsk_add_fact;             /* The offset from zero used by the    */
                                     /* the target machine.                 */

  int     zeros;        /* 1 - if the target machine uses leading zeros when */
                        /*     designating the disk number (eg - the paragon */
                        /*     uses /pfs/io_01)                              */
                        /* 0 - if it does not (eg - the tflop uses           */
                        /*     /pfs/tmp_1)                                   */

  int     file_type;    /* input file type */
  ZOLTAN_FILETYPE file_comp;  /* whether the file is compressed and how      */
  int     chunk_reader;   /* for very large matrix market files */
  int     init_dist_type;      /* Flag indicating how input objects
                                  should be initially distributed.     */
  int     init_dist_procs;     /* How many procs to use in 
                                  the initial distribution.            */
  ZOLTAN_ID_TYPE  init_size;           /* For NO_FILE_* (random) input, the 
                                  no. of objects to be created. */
  int     init_dim;            /* For NO_FILE_* (random) input, the 
                                  dimension of the problem (1, 2, or 3D) */
  int     init_vwgt_dim;       /* For NO_FILE_* (random) input, the 
                                  no. of weights per object.           */
  int     matrix_obj;          /* What are the objects to be balanced
                                  for a sparse matrix? (ROWS, COLUMNS) */
  int     init_dist_pins;      /* For hypergraphs, how should pins be  
                                  initially distributed.               */

  /* The root location of the parallel disks */
  char    pdsk_root[FILENAME_MAX+1];

  /* The subdirectory to write files to */
  char    pdsk_subdir[FILENAME_MAX+1];

  /* The base name of the input file. */
  char    pexo_fname[FILENAME_MAX+1];

};
typedef struct Parallel_IO  PARIO_INFO;
typedef struct Parallel_IO *PARIO_INFO_PTR;

/* 
 * Generic list of options for programs that want to use
 * zdrive style of input file.
 */

#define UNDEFINED_LIST_MAX    20
#define UNDEFINED_LENGTH_MAX 128

struct ParameterList
{
  int list_size;
  char line[UNDEFINED_LIST_MAX][UNDEFINED_LENGTH_MAX];
};
typedef struct ParameterList UNDEFINED_INFO;
typedef struct ParameterList *UNDEFINED_INFO_PTR;


/* Function prototypes */
extern int read_cmd_file(
  const char *filename,
  PROB_INFO_PTR prob,
  PARIO_INFO_PTR pio_info,
  UNDEFINED_INFO_PTR undef
);

extern int check_inp(
  PROB_INFO_PTR prob,
  PARIO_INFO_PTR pio_info
);

extern void brdcst_cmd_info(
  int Proc,
  PROB_INFO_PTR prob,
  PARIO_INFO_PTR pio_info,
  MESH_INFO_PTR mesh
);

extern void gen_par_filename(
  char *scalar_fname,
  char *par_fname,
  PARIO_INFO_PTR pio_info,
  int proc_for,
  int nprocs
);

extern int read_exoII_file(
  int Proc,
  int Num_Proc,
  PROB_INFO_PTR prob,
  PARIO_INFO_PTR pio_info,
  MESH_INFO_PTR mesh
);

extern int write_elem_vars(
  int Proc,
  MESH_INFO_PTR mesh,
  PARIO_INFO_PTR pio_info,
  int num_exp,
  ZOLTAN_ID_PTR exp_gids,
  int *exp_procs,
  int *exp_to_part
);

extern int read_chaco_file(
  int Proc,
  int Num_Proc,
  PROB_INFO_PTR prob,
  PARIO_INFO_PTR pio_info,
  MESH_INFO_PTR mesh
);

extern int read_hypergraph_file(
  int Proc,
  int Num_Proc,
  PROB_INFO_PTR prob,
  PARIO_INFO_PTR pio_info,
  MESH_INFO_PTR mesh
);

extern int read_mm_file(
  int Proc,
  int Num_Proc,
  PROB_INFO_PTR prob,
  PARIO_INFO_PTR pio_info,
  MESH_INFO_PTR mesh
);

extern void mm_cleanup(MESH_INFO_PTR);

extern int read_mtxplus_file(
  int Proc,
  int Num_Proc,
  PROB_INFO_PTR prob,
  PARIO_INFO_PTR pio_info,
  MESH_INFO_PTR mesh
);

extern int create_random_triangles(
  int Proc,
  int Num_Proc,
  PROB_INFO_PTR prob,
  PARIO_INFO_PTR pio_info,
  MESH_INFO_PTR mesh
);

extern int create_random_input(
  int Proc,
  int Num_Proc,
  PROB_INFO_PTR prob,
  PARIO_INFO_PTR pio_info,
  MESH_INFO_PTR mesh
);

extern int create_a_graph(
  int Proc,
  int Num_Proc,
  PROB_INFO_PTR prob,
  PARIO_INFO_PTR pio_info,
  MESH_INFO_PTR mesh
);

extern int chaco_fill_elements(int, int, PROB_INFO_PTR,
                         MESH_INFO_PTR, PARIO_INFO_PTR,
                         int, int, int *,
                         int *, int, float *, int, float *, int,
                         float *, float *, float *, short *, int);

extern int chaco_setup_mesh_struct(int, int, PROB_INFO_PTR,
                         MESH_INFO_PTR, PARIO_INFO_PTR ,
                         int, int, int *,
                         int *, int, float *, int, float *, int,
                         float *, float *, float *, short *, int, int);

extern void chaco_init_local_ids(int **, int **, int *, int *, int *, 
                         short *, int);

#ifdef __cplusplus
} /* closing bracket for extern "C" */
#endif

#endif /* _DR_INPUT_CONST_H_ */
