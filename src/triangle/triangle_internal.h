#ifndef TRIANGLE_INTERNAL_H
#define TRIANGLE_INTERNAL_H

#include "triangle.h"

#include <stdio.h>
#include <stdlib.h>

#define EXPORTI __declspec(dllexport)

#ifdef __cplusplus
extern "C" {
#endif

/********* User-defined triangle evaluation routine begins here      *********/
/**                                                                         **/

/**
 * Determine if a triangle is unsuitable, and thus must be further refined.
 */
EXPORTI int triunsuitable(vertex triorg, vertex tridest, vertex triapex, REAL area);


/**                                                                         **/
/********* User-defined triangle evaluation routine ends here        *********/

EXPORTI void interpolate(vertex newvertex, vertex org, vertex dest, vertex apex, int nextras);

EXPORTI void behavior_update(behavior *b);

/********* Memory allocation and program exit wrappers begin here    *********/
/**                                                                         **/

EXPORTI void triexit(int status);

EXPORTI void *trimalloc(int size);

EXPORTI void trifree(VOID *memptr);


/**                                                                         **/
/********* Memory allocation and program exit wrappers end here      *********/

/********* User interaction routines begin here                      *********/
/**                                                                         **/

EXPORTI void internalerror();

/**
 * Read the command line, identify switches and set up options.
 */
EXPORTI void parsecommandline(char *options, behavior *b);

/**                                                                         **/
/********* User interaction routines begin here                      *********/

/********* Debugging routines begin here                             *********/
/**                                                                         **/

/**
 * Print out the details of an oriented triangle.
 */
EXPORTI void printtriangle(mesh *m, behavior *b, struct otri *t);

/**
 * Print out the details of an oriented subsegment.
 */
EXPORTI void printsubseg(mesh *m, behavior *b, struct osub *s);

/**                                                                         **/
/********* Debugging routines end here                               *********/

/********* Memory management routines begin here                     *********/
/**                                                                         **/

/**
 * Set all of a pool's fields to zero.
 */
EXPORTI void poolzero(struct memorypool *pool);

/**
 * Deallocate all items in a pool.
 */
EXPORTI void poolrestart(struct memorypool *pool);

/**
 * Initialize a pool of memory for allocation of items.
 */
EXPORTI void poolinit(struct memorypool *pool, int bytecount, int itemcount,
              int firstitemcount, int alignment);

/**
 * Free to the operating system all memory taken by a pool.
 */
EXPORTI void pooldeinit(struct memorypool *pool);

/**
 * Allocate space for an item.
 */
VOID *poolalloc(struct memorypool *pool);

/**
 * Deallocate space for an item.
 */
EXPORTI void pooldealloc(struct memorypool *pool, VOID *dyingitem);

/**
 * Prepare to traverse the entire list of items.
 */
EXPORTI void traversalinit(struct memorypool *pool);

/**
 * Find the next item in the list.
 */
EXPORTI VOID *traverse(struct memorypool *pool);

/**
 * Initialize the triangle that fills "outer space" and the
 * omnipresent subsegment.
 */
EXPORTI void dummyinit(mesh *m, behavior *b, int trianglebytes,
               int subsegbytes);

EXPORTI void initializevertexpool(mesh *m, behavior *b);

EXPORTI void initializetrisubpools(mesh *m, behavior *b);

EXPORTI void triangledealloc(mesh *m, triangle *dyingtriangle);

EXPORTI triangle *triangletraverse(mesh *m);

EXPORTI void subsegdealloc(mesh *m, subseg *dyingsubseg);

EXPORTI subseg *subsegtraverse(mesh *m);

EXPORTI void vertexdealloc(mesh *m, vertex dyingvertex);

EXPORTI vertex vertextraverse(mesh *m);

EXPORTI void badsubsegdealloc(mesh *m, struct badsubseg *dyingseg);

EXPORTI struct badsubseg *badsubsegtraverse(mesh *m);

EXPORTI vertex getvertex(mesh *m, behavior *b, int number);

EXPORTI void triangledeinit(mesh *m, behavior *b);

/**                                                                         **/
/********* Memory management routines end here                       *********/

/********* Constructors begin here                                   *********/
/**                                                                         **/

EXPORTI void maketriangle(mesh *m, behavior *b, struct otri *newotri);

EXPORTI void makesubseg(mesh *m, struct osub *newsubseg);

/**                                                                         **/
/********* Constructors end here                                     *********/

EXPORTI void triangleinit(mesh *m);

EXPORTI unsigned long randomnation(unsigned int choices);

/********* Mesh quality testing routines begin here                  *********/
/**                                                                         **/

EXPORTI int checkmesh(mesh *m, behavior *b);

EXPORTI int checkdelaunay(mesh *m, behavior *b);

#ifndef CDT_ONLY

EXPORTI void enqueuebadtriang(mesh *m, behavior *b,
                      struct badtriang *badtri);

EXPORTI void enqueuebadtri(mesh *m, behavior *b, struct otri *enqtri,
                   REAL minedge, vertex enqapex, vertex enqorg, vertex enqdest);

EXPORTI struct badtriang *dequeuebadtriang(mesh *m);

EXPORTI int checkseg4encroach(mesh *m, behavior *b,
                      struct osub *testsubseg);

EXPORTI void testtriangle(mesh *m, behavior *b, struct otri *testtri);

#endif /* not CDT_ONLY */

/**                                                                         **/
/********* Mesh quality testing routines end here                    *********/

/********* Point location routines begin here                        *********/
/**                                                                         **/

EXPORTI void makevertexmap(mesh *m, behavior *b);

EXPORTI enum locateresult preciselocate(mesh *m, behavior *b,
                                vertex searchpoint, struct otri *searchtri,
                                int stopatsubsegment);

EXPORTI enum locateresult locate(mesh *m, behavior *b,
                         vertex searchpoint, struct otri *searchtri);

/**                                                                         **/
/********* Point location routines end here                          *********/

/********* Mesh transformation routines begin here                   *********/
/**                                                                         **/

EXPORTI void insertsubseg(mesh *m, behavior *b, struct otri *tri,
                  int subsegmark);

EXPORTI void flip(mesh *m, behavior *b, struct otri *flipedge);

EXPORTI void unflip(mesh *m, behavior *b, struct otri *flipedge);

enum insertvertexresult insertvertex(mesh *m, behavior *b,
                                     vertex newvertex, struct otri *searchtri,
                                     struct osub *splitseg,
                                     int segmentflaws, int triflaws,
                                     int attribs);

EXPORTI void triangulatepolygon(mesh *m, behavior *b,
                        struct otri *firstedge, struct otri *lastedge,
                        int edgecount, int doflip, int triflaws);

#ifndef CDT_ONLY

EXPORTI void deletevertex(mesh *m, behavior *b, struct otri *deltri);

EXPORTI void undovertex(mesh *m, behavior *b);

#endif /* not CDT_ONLY */

/**                                                                         **/
/********* Mesh transformation routines end here                     *********/

/********* Divide-and-conquer Delaunay triangulation begins here     *********/
/**                                                                         **/

EXPORTI void vertexsort(vertex *sortarray, int arraysize);

EXPORTI void vertexmedian(vertex *sortarray, int arraysize, int median, int axis);

EXPORTI void alternateaxes(vertex *sortarray, int arraysize, int axis);

EXPORTI void mergehulls(mesh *m, behavior *b, struct otri *farleft,
                struct otri *innerleft, struct otri *innerright,
                struct otri *farright, int axis);
                
EXPORTI void divconqrecurse(mesh *m, behavior *b, vertex *sortarray,
                    int vertices, int axis,
                    struct otri *farleft, struct otri *farright);

EXPORTI long removeghosts(mesh *m, behavior *b, struct otri *startghost);

EXPORTI long divconqdelaunay(mesh *m, behavior *b);

/**                                                                         **/
/********* Divide-and-conquer Delaunay triangulation ends here       *********/

/********* Incremental Delaunay triangulation begins here            *********/
/**                                                                         **/

#ifndef REDUCED

EXPORTI void boundingbox(mesh *m, behavior *b);

EXPORTI long removebox(mesh *m, behavior *b);

EXPORTI long incrementaldelaunay(mesh *m, behavior *b);

#endif /* not REDUCED */

/**                                                                         **/
/********* Incremental Delaunay triangulation ends here              *********/

/********* Sweepline Delaunay triangulation begins here              *********/
/**                                                                         **/

#ifndef REDUCED

EXPORTI void eventheapinsert(struct event **heap, int heapsize, struct event *newevent);

EXPORTI void eventheapify(struct event **heap, int heapsize, int eventnum);

EXPORTI void eventheapdelete(struct event **heap, int heapsize, int eventnum);

EXPORTI void createeventheap(mesh *m, struct event ***eventheap,
                     struct event **events, struct event **freeevents);

EXPORTI int rightofhyperbola(mesh *m, struct otri *fronttri, vertex newsite);

EXPORTI REAL circletop(mesh *m, vertex pa, vertex pb, vertex pc, REAL ccwabc);

EXPORTI void check4deadevent(struct otri *checktri, struct event **freeevents,
                     struct event **eventheap, int *heapsize);

EXPORTI struct splaynode *splay(mesh *m, struct splaynode *splaytree,
                        vertex searchpoint, struct otri *searchtri);

EXPORTI struct splaynode *splayinsert(mesh *m, struct splaynode *splayroot,
                              struct otri *newkey, vertex searchpoint);

EXPORTI struct splaynode *circletopinsert(mesh *m, behavior *b,
                                  struct splaynode *splayroot,
                                  struct otri *newkey,
                                  vertex pa, vertex pb, vertex pc, REAL topy);

EXPORTI struct splaynode *frontlocate(mesh *m, struct splaynode *splayroot,
                              struct otri *bottommost, vertex searchvertex,
                              struct otri *searchtri, int *farright);

EXPORTI long sweeplinedelaunay(mesh *m, behavior *b);

#endif /* not REDUCED */

/**                                                                         **/
/********* Sweepline Delaunay triangulation ends here                *********/

/********* General mesh construction routines begin here             *********/
/**                                                                         **/

EXPORTI long delaunay(mesh *m, behavior *b);

#ifndef CDT_ONLY

EXPORTI int reconstruct(mesh *m, behavior *b, int *trianglelist,
                REAL *triangleattriblist, REAL *trianglearealist,
                int elements, int corners, int attribs,
                int *segmentlist,int *segmentmarkerlist, int numberofsegments);

#endif /* not CDT_ONLY */

/**                                                                         **/
/********* General mesh construction routines end here               *********/

/********* Segment insertion begins here                             *********/
/**                                                                         **/

EXPORTI enum finddirectionresult finddirection(mesh *m, behavior *b,
                                       struct otri *searchtri,
                                       vertex searchpoint, int *status);

EXPORTI void segmentintersection(mesh *m, behavior *b,
                         struct otri *splittri, struct osub *splitsubseg,
                         vertex endpoint2, int *status);

EXPORTI int scoutsegment(mesh *m, behavior *b, struct otri *searchtri,
                 vertex endpoint2, int newmark, int *status);

#ifndef REDUCED
#ifndef CDT_ONLY

EXPORTI void conformingedge(mesh *m, behavior *b, vertex endpoint1, vertex endpoint2,
                    int newmark, int *status);

#endif /* not CDT_ONLY */
#endif /* not REDUCED */

EXPORTI void delaunayfixup(mesh *m, behavior *b,
                   struct otri *fixuptri, int leftside);

EXPORTI void constrainededge(mesh *m, behavior *b,
                     struct otri *starttri, vertex endpoint2, int newmark, int *status);

EXPORTI void insertsegment(mesh *m, behavior *b,
                   vertex endpoint1, vertex endpoint2, int newmark, int *status);

EXPORTI void markhull(mesh *m, behavior *b);

EXPORTI void formskeleton(mesh *m, behavior *b, int *segmentlist,
                  int *segmentmarkerlist, int numberofsegments, int *status);

/**                                                                         **/
/********* Segment insertion ends here                               *********/

/********* Carving out holes and concavities begins here             *********/
/**                                                                         **/

EXPORTI void infecthull(mesh *m, behavior *b);

EXPORTI void plague(mesh *m, behavior *b);

EXPORTI void regionplague(mesh *m, behavior *b,
                  REAL attribute, REAL area);

EXPORTI void carveholes(mesh *m, behavior *b, REAL *holelist, int holes,
                REAL *regionlist, int regions);

/**                                                                         **/
/********* Carving out holes and concavities ends here               *********/

/********* Mesh quality maintenance begins here                      *********/
/**                                                                         **/

#ifndef CDT_ONLY

EXPORTI void tallyencs(mesh *m, behavior *b);

EXPORTI void precisionerror();

EXPORTI void splitencsegs(mesh *m, behavior *b, int triflaws, int *status);

EXPORTI void tallyfaces(mesh *m, behavior *b);

EXPORTI void splittriangle(mesh *m, behavior *b,
                   struct badtriang *badtri);

EXPORTI void enforcequality(mesh *m, behavior *b, int *status);

#endif /* not CDT_ONLY */

/**                                                                         **/
/********* Mesh quality maintenance ends here                        *********/

EXPORTI void highorder(mesh *m, behavior *b);

/********* Array I/O routines begin here                              *********/
/**                                                                         **/

EXPORTI int transfernodes(mesh *m, behavior *b, REAL *pointlist,
                   REAL *pointattriblist, int *pointmarkerlist,
                   int numberofpoints, int numberofpointattribs);

EXPORTI void writenodes(mesh *m, behavior *b, REAL **pointlist,
                REAL **pointattriblist, int **pointmarkerlist);

EXPORTI void numbernodes(mesh *m, behavior *b);

EXPORTI void writeelements(mesh *m, behavior *b,
                   int **trianglelist, REAL **triangleattriblist);

EXPORTI void writepoly(mesh *m, behavior *b,
               int **segmentlist, int **segmentmarkerlist);

EXPORTI void writeedges(mesh *m, behavior *b,
                int **edgelist, int **edgemarkerlist);

EXPORTI void writeneighbors(mesh *m, behavior *b, int **neighborlist);

/**                                                                         **/
/********* Array I/O routines end here                                *********/

/********* File I/O routines begin here                              *********/
/**                                                                         **/

EXPORTI int file_writenodes(mesh *m, behavior *b, FILE *nodefile);

EXPORTI int file_writeelements(mesh *m, behavior *b, FILE *elefile);

EXPORTI int file_writepoly(mesh *m, behavior *b, FILE *polyfile,
				   REAL *holelist, int holes, REAL *regionlist, int regions);

EXPORTI int file_writeedges(mesh *m, behavior *b, FILE *edgefile);

EXPORTI int file_writeneighbors(mesh *m, behavior *b, FILE *neighborfile);

EXPORTI int file_write_eps(mesh* m, behavior *b, FILE *file);

EXPORTI int file_readnodes(FILE *nodefile, triangleio *io, int *firstnode);
				   
EXPORTI int file_readpoly(FILE *nodefile, triangleio *io, int *firstnode);
				   
EXPORTI int file_readelements(FILE *nodefile, triangleio *io);

EXPORTI int file_readelementsarea(FILE *file, triangleio *io);

/**                                                                         **/
/********* File I/O routines end here                                *********/

EXPORTI int quality_statistics(mesh *m, behavior *b, quality *q);

#ifdef __cplusplus
}
#endif

#endif /* TRIANGLE_INTERNAL_H */