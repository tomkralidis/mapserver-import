#ifndef MAP_H
#define MAP_H

/*
** Main includes. If a particular header was needed by several .c files then
** I just put it here. What the hell, it works and it's all right here. -SDL-
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#if defined(_WIN32) && !defined(__CYGWIN__)
#include <direct.h>
#include <memory.h>
#include <malloc.h>
#else
#include <unistd.h>
#endif

#ifdef USE_MPATROL
#include "mpatrol.h"
#endif

#include "maperror.h"
#include "mapprimitive.h"
#include "mapshape.h"
#include "mapsymbol.h"
#include "maptree.h" // quadtree spatial index
#include "maphash.h" 

#include "mapproject.h"

#include <gd.h>

#include <sys/types.h> /* regular expression support */
#include <regex.h>

#ifdef __cplusplus
extern "C" {
#endif

// General defines, wrapable

#define MS_VERSION "3.5 (pre-alpha)"

#define MS_TRUE 1 /* logical control variables */
#define MS_FALSE 0
#define MS_ON 1
#define MS_OFF 0
#define MS_DEFAULT 2
#define MS_QUERY 3
#define MS_EMBED 4
#define MS_YES 1
#define MS_NO 0

#define MS_SINGLE 0 /* modes for searching (spatial/database) */
#define MS_MULTIPLE 1

// General defines, not wrapable
#ifndef SWIG
#define MS_MAPFILE_EXPR "\\.map$"
#define MS_TEMPLATE_EXPR "\\.(wm|htm)l?$"

#define MS_INDEX_EXTENSION ".qix"
#define MS_QUERY_EXTENSION ".qy"

#define MS_DEG_TO_RAD	.0174532925199432958
#define MS_RAD_TO_DEG   57.29577951

#define MS_RED 0
#define MS_GREEN 1
#define MS_BLUE 2

#define MS_PPI 72 /* number of pixels/inch on a monitor, used for simple scale calculations */

#define MS_MAXCOLORS 256

#define MS_BUFFER_LENGTH 2048 /* maximum input line length */
#define MS_URL_LENGTH 1024

#define MS_MAXCLASSES 50
#define MS_MAXQUERIES MS_MAXCLASSES
#define MS_MAXPROJARGS 20
#define MS_MAXLAYERS 100 /* maximum number of layers in a map file */
#define MS_MAXJOINS 5
#define MS_ITEMNAMELEN 32
#define MS_NAMELEN 20

#define MS_MINSYMBOLSIZE 1   // in pixels
#define MS_MAXSYMBOLSIZE 100

#define MS_URL 0 /* template types */
#define MS_FILE 1

#define MS_MINFONTSIZE 4
#define MS_MAXFONTSIZE 256

#define MS_LABELCACHEINITSIZE 100
#define MS_LABELCACHEINCREMENT 10

#define MS_RESULTCACHEINITSIZE 10
#define MS_RESULTCACHEINCREMENT 10

#define MS_FEATUREINITSIZE 10 /* how many points initially can a feature have */ 
#define MS_FEATUREINCREMENT 10

#define MS_EXPRESSION 2000
#define MS_REGEX 2001
#define MS_STRING 2002
#define MS_NUMBER 2003

#define MS_ALLITEMS 12120

// General macro definitions
#define MS_MIN(a,b)     (((a)<(b))?(a):(b))
#define MS_MAX(a,b)	(((a)>(b))?(a):(b))
#define MS_ABS(a)	(((a)<0) ? -(a) : (a))
#define MS_SGN(a)	(((a)<0) ? -1 : 1)
#define MS_NINT(x)      ((x) >= 0.0 ? ((long) ((x)+.5)) : ((long) ((x)-.5)))
#endif

// General enumerated types - needed by scripts
enum MS_FILE_TYPE {MS_FILE_MAP, MS_FILE_SYMBOL};
enum MS_UNITS {MS_INCHES, MS_FEET, MS_MILES, MS_METERS, MS_KILOMETERS, MS_DD, MS_PIXELS};
enum MS_FEATURE_TYPE {MS_POINT, MS_LINE, MS_POLYGON, MS_POLYLINE, MS_RASTER, MS_ANNOTATION, MS_NULL};
enum MS_FONT_TYPE {MS_TRUETYPE, MS_BITMAP};
enum MS_LABEL_POSITIONS {MS_UL, MS_LR, MS_UR, MS_LL, MS_CR, MS_CL, MS_UC, MS_LC, MS_CC, MS_AUTO, MS_XY}; // arrangement matters for auto placement, don't change it
enum MS_BITMAP_FONT_SIZES {MS_TINY , MS_SMALL, MS_MEDIUM, MS_LARGE, MS_GIANT};
enum MS_QUERYMAP_STYLES {MS_NORMAL, MS_HILITE, MS_SELECTED, MS_INVERTED};
enum MS_CONNECTION_TYPE {MS_INLINE, MS_SHAPEFILE, MS_TILED_SHAPEFILE, MS_SDE, MS_OGR, MS_TILED_OGR};
enum MS_OUTPUT_IMAGE_TYPE {MS_GIF, MS_PNG, MS_JPEG, MS_WBMP};

enum MS_RETURN_VALUE {MS_SUCCESS, MS_FAILURE, MS_DONE};

#define MS_FILE_DEFAULT MS_FILE_MAP

#ifndef SWIG
// FONTSET OBJECT - used to hold aliases for TRUETYPE fonts
typedef struct {
  char *filename;
  hashTableObj fonts;
  int numfonts;
} fontSetObj;
#endif

// FEATURE LIST OBJECT - for inline features, shape caches and queries
typedef struct listNode {
  shapeObj shape;
  struct listNode *next;
} featureListNodeObj;

typedef featureListNodeObj * featureListNodeObjPtr;

// COLOR OBJECT
typedef struct {
  int red;
  int green;
  int blue;
} colorObj;

#ifndef SWIG
// PALETTE OBJECT - used to hold colors while a map file is read
typedef struct {
  colorObj colors[MS_MAXCOLORS-1];
  int numcolors;
} paletteObj;
#endif

// EXPRESSION OBJECT
#ifndef SWIG
typedef struct {
  char *string;
  int type;
  
  // logical expression options
  char **items;
  int *indexes;
  int numitems;

  // regular expression options
  regex_t regex; // compiled regular expression to be matched
} expressionObj;
#endif

#ifndef SWIG
// JOIN OBJECT - simple way to access other XBase files, one-to-one or one-to-many supported
typedef struct {  
  char *name;

  char **items; /* array of item names */
  char numitems;
  char ***data; /* array of data for 1 or more records */
  int numrecords; /* number of records we have data for */

  char *table;
  char *from, *to;
  char *header;
#ifndef __cplusplus
  char *template;
#else
  char *_template;
#endif
  char *footer;
  
  char *match;

  int type;
} joinObj;
#endif

// QUERY MAP OBJECT - used to visualize query results
typedef struct {
  int height, width;
  int status;
  int style; /* HILITE, SELECTED or NORMAL */
  int color;
} queryMapObj;

// LABEL OBJECT - parameters needed to annotate a layer, legend or scalebar
typedef struct {
  char *font;
  enum MS_FONT_TYPE type;

  int color;
  int outlinecolor;

  int shadowcolor;
  int shadowsizex, shadowsizey;

  int backgroundcolor;
  int backgroundshadowcolor;
  int backgroundshadowsizex, backgroundshadowsizey;

  int size;
  int sizescaled;
  int minsize, maxsize;

  int position;
  int offsetx, offsety;

  double angle;
  int autoangle; // true or false 

  int buffer; /* space to reserve around a label */

  int antialias;

  char wrap;

  int minfeaturesize; /* minimum feature size (in pixels) to label */
  int autominfeaturesize; // true or false 

  int mindistance;
  int partials; /* can labels run of an image */

  int force; // labels *must* be drawn
} labelObj;

// WEB OBJECT - holds parameters for a mapserver/mapscript interface
typedef struct {
  char *log;
  char *imagepath, *imageurl;
#ifndef __cplusplus
  char *template;
#else
  char *_template;
#endif
  char *header, *footer;  
  char *empty, *error; /* error handling */
  rectObj extent; /* clipping extent */
  double minscale, maxscale;
  char *mintemplate, *maxtemplate;
} webObj;

// CLASS OBJECT - basic symbolization and classification information
typedef struct {
#ifndef SWIG  
  expressionObj expression; // the expression to be matched
#endif

  int color;
  int backgroundcolor;
  int outlinecolor;
  int overlaycolor;
  int overlaybackgroundcolor;
  int overlayoutlinecolor;

  int symbol;
  char *symbolname;
  int overlaysymbol;
  char *overlaysymbolname;

  int size;
  int sizescaled;
  int minsize, maxsize;

  int overlaysize;
  int overlaysizescaled;
  int overlayminsize, overlaymaxsize;

  labelObj label;
  char *name; /* used for legend labeling */
#ifndef SWIG
  expressionObj text;
#endif

#ifndef __cplusplus
  char *template;
#else
  char *_template;
#endif

  joinObj *joins;
  int numjoins;	
} classObj;

// LABELCACHE OBJECTS - structures to implement label caching and collision avoidance etc
typedef struct {
  int size; /* parameters that can vary for each label/feature */
  double angle;
  char *string;
  double featuresize;

  int layeridx; /* indexes */
  int classidx;
  int tileidx;
  int shapeidx;

  pointObj point; /* label point */
  shapeObj *poly; /* label bounding box */  

  int status; /* has this label been drawn or not */
} labelCacheMemberObj;

typedef struct {
  int id; // corresponding label
  shapeObj *poly; // marker bounding box (POINT layers only)
} markerCacheMemberObj;

typedef struct {
  labelCacheMemberObj *labels;
  int numlabels;  
  int cachesize;
  markerCacheMemberObj *markers;
  int nummarkers;
  int markercachesize;
} labelCacheObj;

typedef struct {
  int shapeindex;
  int tileindex;
  char classindex;
} resultCacheMemberObj;

typedef struct {
  resultCacheMemberObj *results;
  int numresults;  
  int cachesize;
  rectObj bounds;
} resultCacheObj;

// SYMBOLSET OBJECT
typedef struct {
  char *filename;
#ifndef SWIG
  fontSetObj *fontset; // a pointer to the main mapObj version
#endif
  int numsymbols;
  symbolObj symbol[MS_MAXSYMBOLS];

  struct imageCacheObj *imagecache;
  int imagecachesize;
} symbolSetObj;

// REFERENCE MAP OBJECT
typedef struct {
  rectObj extent;
  int height, width;
  colorObj color;
  colorObj outlinecolor;
  char *image;
#ifndef SWIG
  int imagetype;
#endif
  int status;
} referenceMapObj;

// SCALEBAR OBJECT
typedef struct {
  colorObj imagecolor;
  int height, width;
  int style;
  int intervals;
  labelObj label;
  int color;
  int backgroundcolor;
  int outlinecolor;
  int units;
  int status; // ON, OFF or EMBED
  int position; // for embeded scalebars
  int transparent;
  int interlace;
  int postlabelcache;
} scalebarObj;

// LEGEND OBJECT
typedef struct {
  colorObj imagecolor;
  labelObj label;
  int keysizex, keysizey;
  int keyspacingx, keyspacingy;
  int outlinecolor; /* Color of outline of box, -1 for no outline */
  int status; // ON, OFF or EMBED
  int height, width;
  int position; // for embeded legends
  int transparent;
  int interlace;
  int postlabelcache;
} legendObj;

// LAYER OBJECT - basic unit of a map
typedef struct {
  int index;

  char *classitem; /* .DBF item to be used for symbol lookup */
  int classitemindex;

#ifndef __cplusplus
  classObj *class; /* always at least 1 class */
#else
  classObj *_class;
#endif

#ifdef SWIG
%readonly
#endif
  int numclasses;
#ifdef SWIG
%readwrite
#endif

#ifndef SWIG
  char *header, *footer; // only used with multi result queries
#endif

  resultCacheObj *resultcache; // holds the results of a query against this layer

  char *name; // should be unique
  char *group; // shouldn't be unique it's supposed to be a group right?

  char *description; // layer title or description
  char *legend; // legend graphic text

  int status; // on or off
  char *data; // filename, can be relative or full path
  enum MS_FEATURE_TYPE type;
  
  double tolerance; // search buffer for point and line queries (in toleranceunits)
  int toleranceunits;

  double symbolscale; // scale at which symbols are default size
  double minscale, maxscale;
  double labelminscale, labelmaxscale;

  int maxfeatures;

  int offsite; // transparent pixel value for raster images

  int transform; // does this layer have to be transformed to file coordinates

  int labelcache, postlabelcache; // on or off

  char *labelitem, *labelsizeitem, *labelangleitem; 
  int labelitemindex, labelsizeitemindex, labelangleitemindex;

  char *tileitem;
  int tileitemindex;
  char *tileindex; /* layer index file for tiling support */

  projectionObj projection; /* projection information for the layer */

  featureListNodeObjPtr features; // linked list so we don't need a counter
  featureListNodeObjPtr currentfeature; // pointer to the current feature

  char *connection;
  enum MS_CONNECTION_TYPE connectiontype;

  // a variety of data connection objects (probably should be pointers!)
  shapefileObj shpfile;
  shapefileObj tileshpfile;

  // attribute handling components
  char **items;
  int numitems;
  int *itemindexes;

} layerObj;

// MAP OBJECT - encompasses everything used in an Internet mapping application
typedef struct { /* structure for a map */
  char *name; /* small identifier for naming etc. */
  int status; /* is map creation on or off */
  int height, width;

  layerObj *layers;

#ifdef SWIG
%readonly
#endif
  int numlayers; /* number of layers in mapfile */
#ifdef SWIG
%readwrite
#endif

#ifndef SWIG
  symbolSetObj symbolset;
  fontSetObj fontset;
#endif

  labelCacheObj labelcache; /* we need this here so multiple feature processors can access it */

  int transparent;
  int interlace;

  rectObj extent; /* map extent array */
  double cellsize; /* in map units */

  enum MS_UNITS units; /* units of the projection */
  double scale; /* scale of the output image */

  char *shapepath; /* where are the shape files located */

  paletteObj palette; /* holds a map palette */
  colorObj imagecolor; /* holds the initial image color value */

  int imagetype, imagequality;

  projectionObj projection; /* projection information for output map */

  referenceMapObj reference;
  scalebarObj scalebar;
  legendObj legend;

#ifndef SWIG
  queryMapObj querymap;
#endif

  webObj web;

} mapObj;

// Function prototypes, wrapable
#ifndef USE_GD_1_8
int msSaveImage(gdImagePtr img, char *filename, int transparent, int interlace);
#else
int msSaveImage(gdImagePtr img, char *filename, int type, int transparent, int interlace, int quality);
#endif
void msFreeImage(gdImagePtr img);

// Function prototypes, not wrapable

#ifndef SWIG
int msDrawSDELayer(mapObj *map, layerObj *layer, gdImagePtr img); /* in mapsde.c */
int msDrawOGRLayer(mapObj *map, layerObj *layer, gdImagePtr img); /* in mapogr.cpp */

/*
** helper functions not part of the general API but needed in
** a few other places (like mapscript)... found in mapfile.c
*/
char *getString();
int getDouble(double *d);
int getInteger(int *i);
int getSymbol(int n, ...); 
int getCharacter(char *c);

int initMap(mapObj *map);
int initLayer(layerObj *layer);
int initClass(classObj *_class);
void initLabel(labelObj *label);

featureListNodeObjPtr insertFeatureList(featureListNodeObjPtr *list, shapeObj *shape);
void freeFeatureList(featureListNodeObjPtr list);

int loadProjectionString(projectionObj *p, char *value);
int loadExpressionString(expressionObj *exp, char *value);

int getClassIndex(layerObj *layer, char *str);

/*
** Main API Functions
*/
int msGetLayerIndex(mapObj *map, char *name); /* in mapfile.c */
int msGetSymbolIndex(symbolSetObj *set, char *name);
mapObj *msLoadMap(char *filename);
int msSaveMap(mapObj *map, char *filename);
void msFreeMap(mapObj *map);
void msFreeCharArray(char **array, int num_items);
int msLoadPalette(gdImagePtr img, paletteObj *palette, colorObj color);
int msAddColor(mapObj *map, int red, int green, int blue);
int msLoadMapString(mapObj *map, char *object, char *value);

int msShapeGetClass(layerObj *layer, shapeObj *shape); // in maputil.c
char *msShapeGetAnnotation(layerObj *layer, shapeObj *shape);
double msAdjustExtent(rectObj *rect, int width, int height);
int msAdjustImage(rectObj rect, int *width, int *height);
gdImagePtr msDrawMap(mapObj *map);
gdImagePtr msDrawQueryMap(mapObj *map);
int msDrawShape(mapObj *map, layerObj *layer, shapeObj *shape, gdImagePtr img, int overlay);
int msDrawPoint(mapObj *map, layerObj *layer, pointObj *point, gdImagePtr img, int classindex, char *text);
int msDrawLayer(mapObj *map, layerObj *layer, gdImagePtr img);
int msDrawQueryLayer(mapObj *map, layerObj *layer, gdImagePtr img);

gdImagePtr msDrawScalebar(mapObj *map); // in mapscale.c
double msCalculateScale(rectObj extent, int units, int width, int height);
int msEmbedScalebar(mapObj *map, gdImagePtr img);

int msPointInRect(pointObj *p, rectObj *rect); // in mapsearch.c
int msRectOverlap(rectObj *a, rectObj *b);
int msRectContained(rectObj *a, rectObj *b);
void msMergeRect(rectObj *a, rectObj *b);
double msDistanceBetweenPoints(pointObj *a, pointObj *b);
double msDistanceFromPointToLine(pointObj *p, pointObj *a, pointObj *b);
int msPointInPolygon(pointObj *p, lineObj *c);
double msDistanceFromPointToMultipoint(pointObj *p, multipointObj *points);
double msDistanceFromPointToPolyline(pointObj *p, shapeObj *polyline);
double msDistanceFromPointToPolygon(pointObj *p, shapeObj *polyline);
int msIntersectMultipointPolygon(multipointObj *points, shapeObj *polygon);
int msIntersectPointPolygon(pointObj *p, shapeObj *polygon);
int msIntersectPolylinePolygon(shapeObj *line, shapeObj *poly);
int msIntersectPolygons(shapeObj *p1, shapeObj *p2);
int msIntersectPolylines(shapeObj *line1, shapeObj *line2);

int msSaveQuery(mapObj *map, char *filename); // in mapquery.c
int msLoadQuery(mapObj *map, char *filename);

int msQueryByItem(mapObj *map, int qlayer, int mode, char *item, char *value);
int msQueryByPoint(mapObj *map, int qlayer, int mode, pointObj p, double buffer);
int msQueryByRect(mapObj *map, int qlayer, rectObj rect);
int msQueryByFeatures(mapObj *map, int qlayer, int slayer);
int msQueryByShape(mapObj *map, int qlayer, shapeObj *search_shape);

int msJoinDBFTables(joinObj *join, char *path, char *tile);

void trimBlanks(char *string); // in mapstring.c
char *chop(char *string);
void trimEOL(char *string);
char *gsub(char *str, const char *old, const char *sznew);
char *stripPath(char *fn);
char *getPath(char *fn);
char **split(const char *string, char cd, int *num_tokens);
int countChars(char *str, char ch);

#ifdef NEED_STRDUP
char *strdup(char *s);
#endif

#ifdef NEED_STRNCASECMP
int strncasecmp(char *s1, char *s2, int len);
#endif

#ifdef NEED_STRCASECMP
int strcasecmp(char *s1, char *s2);
#endif

int msLoadSymbolSet(symbolSetObj *symbolset); // in mapsymbol.c
void msInitSymbolSet(symbolSetObj *symbolset);
int msAddImageSymbol(symbolSetObj *symbolset, char *filename);
void msFreeSymbolSet(symbolSetObj *symbolset);
void msDrawShadeSymbol(symbolSetObj *symbolset, gdImagePtr img, shapeObj *p, int sy, int fc, int bc, int oc, double sz);
void msGetMarkerSize(symbolSetObj *symbolset, classObj *_class, int *width, int *height);
void msDrawMarkerSymbol(symbolSetObj *symbolset,gdImagePtr img, pointObj *p,  int sy, int fc, int bc, int oc, double sz);
void msDrawLineSymbol(symbolSetObj *symbolset, gdImagePtr img, shapeObj *p, int sy, int fc, int bc, int oc, double sz);

gdImagePtr msDrawLegend(mapObj *map); // in maplegend.c
int msEmbedLegend(mapObj *map, gdImagePtr img);

int msLoadFontSet(fontSetObj *fontSet); // in maplabel.c
int msDrawLabel(gdImagePtr img, pointObj labelPnt, char *string, labelObj *label, fontSetObj *fontset);
int msGetLabelSize(char *string, labelObj *label, rectObj *rect, fontSetObj *fontSet);
int msAddLabel(mapObj *map, int layer, int nclass, int tile, int shape, pointObj point, char *string, double featuresize);
int msDrawLabelCache(gdImagePtr img, mapObj *map);
gdFontPtr msGetBitmapFont(int size);
int msImageTruetypePolyline(gdImagePtr img, shapeObj *p, symbolObj *s, int color, int size, fontSetObj *fontset);
int msImageTruetypeArrow(gdImagePtr img, shapeObj *p, symbolObj *s, int color, int size, fontSetObj *fontset);

  void msFreeShape(shapeObj *shape); // in mapprimative.c
void msInitShape(shapeObj *shape);
int msCopyShape(shapeObj *from, shapeObj *to);
void msComputeBounds(shapeObj *shape);
void msRectToPolygon(rectObj rect, shapeObj *poly);
void msClipPolylineRect(shapeObj *in, rectObj rect, shapeObj *out);
void msClipPolygonRect(shapeObj *in, rectObj rect, shapeObj *out);
void msTransformShape(rectObj extent, double cellsize, shapeObj *shape);
void msImageScanline(gdImagePtr img, int x1, int x2, int y, int c);
void msImagePolyline(gdImagePtr img, shapeObj *p, int c);
void msImageFilledPolygon(gdImagePtr img, shapeObj *p, int c);
int msPolylineLabelPoint(shapeObj *p, pointObj *lp, int min_length, double *angle, double *length);
int msPolygonLabelPoint(shapeObj *p, pointObj *lp, int min_dimension);
int msAddLine(shapeObj *p, lineObj *new_line);

int msDrawRasterLayer(mapObj *map, layerObj *layer, gdImagePtr img); // in mapraster.c
gdImagePtr msDrawReferenceMap(mapObj *map);

size_t msGetBitArraySize(int numbits); // in mapbits.c
char *msAllocBitArray(int numbits);
int msGetBit(char *array, int index);
void msSetBit(char *array, int index, int value);
void msFlipBit(char *array, int index);

int msLayerOpen(layerObj *layer, char *shapepath); // in maplayer.c
void msLayerClose(layerObj *layer);
int msLayerWhichShapes(layerObj *layer, char *shapepath, rectObj rect, projectionObj *out);
int msLayerWhichItems(layerObj *layer, int annotate);
int msLayerNextShape(layerObj *layer, char *shapepath, shapeObj *shape, int attributes);
int msLayerGetShape(layerObj *layer, char *shapepath, shapeObj *shape, int tile, int record, int attributes);

int msTiledSHPOpenFile(layerObj *layer, char *shapepath); // in mapshape.c
int msTiledSHPWhichShapes(layerObj *layer, char *shapepath, rectObj rect, projectionObj *proj);
int msTiledSHPNextShape(layerObj *layer, char *shapepath, shapeObj *shape, int attributes);
int msTiledSHPGetShape(layerObj *layer, char *shapepath, shapeObj *shape, int tile, int record, int attributes);
void msTiledSHPClose(layerObj *layer);

#endif

#ifdef __cplusplus
}
#endif

#endif /* MAP_H */
