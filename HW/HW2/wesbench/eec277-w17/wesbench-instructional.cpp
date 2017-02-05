/*
 * WESBENCH Copyright (c) 2009, The Regents of the University of
 * California, through  Lawrence Berkeley National Laboratory (subject to
 * receipt of any required approvals from the U.S. Dept. of Energy).  All
 * rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * (1) Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * (2) Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * (3) Neither the name of the University of California, Lawrence
 * Berkeley National Laboratory, U.S. Dept. of Energy nor the names of
 * its contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * You are under no obligation whatsoever to provide any bug fixes,
 * patches, or upgrades to the features, functionality or performance of
 * the source code ("Enhancements") to anyone; however, if you choose to
 * make your Enhancements available either publicly, or directly to
 * Lawrence Berkeley National Laboratory, without imposing a separate
 * written license agreement for such Enhancements, then you hereby grant
 * the following license: a  non-exclusive, royalty-free perpetual
 * license to install, use, modify, prepare derivative works, incorporate
 * into other computer software, distribute, and sublicense such
 * enhancements or derivative works thereof, in binary and source code
 * form.
 *
 *
 * This work is supported by the U. S. Department of Energy under contract
 * number DE-AC03-76SF00098 between the U. S. Department of Energy and the
 * University of California.
 *
 *      Author: Wes Bethel
 *              Lawrence Berkeley National Laboratory
 *              Berkeley, California
 *
 *  "this software is 100% hand-crafted by a human being in the USA"
 */
/*
 * $Id: wesBench.c,v 1.3 2011-01-29 14:48:57 wes Exp $
 * Version: $Name: V1-2 $
 * $Revision: 1.3 $
 * $Log: wesBench.c,v $
 * Revision 1.3  2011-01-29 14:48:57  wes
 * Fixes for Win32 platforms migrated from wesBench-instructional.c
 *
 * Revision 1.2  2010-05-20 16:16:19  wes
 * Updated license text from lab TT office
 *
 * Revision 1.1.1.1  2010/01/04 18:16:47  wes
 * Initial entry
 *
 */

#define _CRT_SECURE_NO_WARNINGS

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

/* user settable define's */

/*
 * setting SCREENSHOT_MODE to 1 will cause the triangle benchmark code to
 * render one frame, pause for 5 seconds, then exit.  A value of 0 will
 * result in normal benchmark runs.
 */
#define SCREENSHOT_MODE 0

/*
 * choose an internal texture storage format. This one could be specified by a
 * command-line argument.
 */
#define TEXTURE_STORAGE_MODE GL_RGBA8
/*#define TEXTURE_STORAGE_MODE GL_R3_G3_B2 */

#define GLEW_STATIC 

#include <GL\glew.h>
#include <GLFW\glfw3.h>
#include <stdlib.h>
//#include <unistd.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

void Init (void);
void printInfo (GLFWwindow * window);
void runBenchmark (void);
void Reshape (int, int);
void Key (unsigned char, int, int);
void check_gl_errors (void);

// enable optimus!
extern "C" {
	_declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}



typedef enum
  {
    DISJOINT_TRIANGLES = 0x00,
    TRIANGLE_STRIPS = 0x01,
    INDEXED_DISJOINT_TRIANGLES = 0x02,
    INDEXED_TRIANGLE_STRIPS = 0x03
  } TriangleType;

#define DEFAULT_TRIANGLE_AREA 128.0
#define DEFAULT_TEST_DURATION_SECONDS 5.0
#define DEFAULT_WIN_WIDTH 1024
#define DEFAULT_WIN_HEIGHT 1024
#define DEFAULT_TRIANGLE_LIMIT 1024*1024*1024 /* 1G */
#define DEFAULT_VERTEXBUF_LIMIT 1024*1024*1024 /* 1G */
#define DEFAULT_TEXTURE_SIZE 0
#define DEFAULT_LIGHTING_ENABLED 0
#define DEFAULT_COLOR_MATERIAL 0
#define DEFAULT_DUMP_FILE_NAME "wesBench-dump.txt"
#define DEFAULT_RETAINED_MODE_ENABLED  0
#define DEFAULT_TRIANGLE_TYPE DISJOINT_TRIANGLES;
#define DEFAULT_CLEAR_PER_FRAME 0
#define DEFAULT_OUTLINE_MODE_BOOL 0 /* 0 means draw filled tri's, 1 means outline */

typedef struct
{
  char  *appName;             /* obtained from argv[0] */
  double triangleAreaInPixels; /* set by command line arg -a XXX */
  double testDurationSeconds; /* set by command line arg -s NNNN */
  int    imgWidth, imgHeight; /* set by -w WWW -h HHH -- NOT IMPLEMENTED */

  size_t triangleLimit;       /* set by -tl NNNN  */
  size_t vertexBufLimit;      /* set by -vl NNNN */

  int    retainedModeEnabled; /* set by -retained */

  int    textureSize;         /* set by -tx WWW (WWW needs to be an even power of two), default=0 no texturing */
  int    enableLighting;      /* set by -light, default is no lighting */
  int    enableColorMaterial;         /* set by -cm, default is off */
  TriangleType  triangleType; /* set by -tt (0, 1, 2, 3) 0=disjoint, 1=tstrip, 2=indexed disjoint, 3=indexed tstrip */

  int    outlineMode;         /* set by -line */
  int    clearPerFrame;       /* set by -clear */

  /* these tests are mutually exclusive */
  int    doAreaTest;          /* set by -doareatest. */
  int    doVBufSizeTest;      /* set by -dovbtest. */
  int    doTextureTest;       /* set by -dotxtest  */

  char   *dumpFileName;       /* set by -df fileName */
  char   *progName;           /* wish GLUT had client data, sigh */

  float  computedFPS;
  float  computedTrisPerSecond;
  float  computedMVertexOpsPerSecond;
  float  computedMFragsPerSecond;
  float  computedMTrisPerSecond;
  size_t computedVertsPerArrayCall; /* the number of verts issued in a glDrawArrays call */
  size_t computedIndicesPerArrayCall; /* the number of indices in glDrawElements call*/
} AppState;

/* global: I hate globals. GLUT doesn't appear to have a mechanism to
   store/retrieve client data, so we're stuck with a global.  */
AppState myAppState;

void wesTriangleRateBenchmark(AppState *myAppState);


typedef struct
{
  GLfloat x, y;
} Vertex2D;

typedef struct
{
  GLfloat x, y, z;
} Vertex3D;

typedef struct
{
  GLfloat r, g, b;
} Color3D;

typedef struct
{
  unsigned char r, g, b, a;
} Color4DUbyte;


const char usageString[] =
  {" \
[-a AAAA]\tsets triangle area in pixels (double precision value)\n \
[-tl LLLL]\tsets maximum number of triangles (long int value)\n \
[-s NNNN]\tsets the duration of the test in seconds.\n \
[-w WWW -h HHH]\t sets the display window size.\n \
[-tx WWW]\t sets the texel resolution of the texture, the texture will be WWWxWWW texels. WWW must be an even power of 2. \n \
[-light]\tturns on lighting (no args)\n \
[-cm]\tturns on Color Material/per vertex colors will be used in diffuse term of lighting (no args)\n \
[-df fname] sets the name of the dumpfile for performance statistics.\n \
[-tt (0, 1, 2, 3)]\tset triangle type: 0=disjoint, 1=tstrip, 2=indexed disjoint, 3=indexed tstrip\n \
[-retained]\tUse OpenGL display lists.  \n \
[-clear]\tClear the frame after each drawing. For eye candy purposes. \n \
[-line]\tSet polygon mode to GL_LINE to draw triangle outlines, no fill. \n \
\n \
Automated tests (choose one or none): \n \
[-doareatest]\tWill iterate over a number of triangle area sizes: 1..128K in even power-of-two steps. \n \
[-dovbtest]\tIterate over different buffer sizes/number of verts sent with each glDrawArraysCall. Will result in 'partial rendering' of base mesh. \n \
[-dotxtest]\tIterate over a set of texture image sizes. \n \
\n"};

void
usage(const char *progName)
{
  fprintf(stderr," %s usage:\n %s \n", progName, usageString);
  fflush(stderr);
}

void
parseArgs(int argc,
          char **argv,
          AppState *myAppState)
{
  int i=1;
  argc--;
  while (argc > 0)
    {
      if (strcmp(argv[i],"-a") == 0)
        {
          i++;
          argc--;
          myAppState->triangleAreaInPixels = atof(argv[i]);
        }
      else if (strcmp(argv[i],"-l") == 0)
        {
          i++;
          argc--;
          myAppState->triangleLimit = (size_t)atol(argv[i]);
        }
      else if (strcmp(argv[i],"-s") == 0)
        {
          i++;
          argc--;
          myAppState->testDurationSeconds = atof(argv[i]);
        }
      else if (strcmp(argv[i],"-tl") == 0)
        {
          i++;
          argc--;
          myAppState->triangleLimit = atoi(argv[i]);
        }
      else if (strcmp(argv[i],"-vl") == 0)
        {
          i++;
          argc--;
          myAppState->vertexBufLimit = atoi(argv[i]);
        }
      else if (strcmp(argv[i],"-tx") == 0)
        {
          i++;
          argc--;
          myAppState->textureSize = atoi(argv[i]);
        }
      else if (strcmp(argv[i],"-light") == 0)
        {
          myAppState->enableLighting = 1;
        }
      else if (strcmp(argv[i],"-cm") == 0)
        {
          myAppState->enableColorMaterial = 1;
        }
      else if (strcmp(argv[i],"-df") == 0)
        {
          i++;
          argc--;
          if (myAppState->dumpFileName != NULL)
            free((void *)myAppState->dumpFileName);
          myAppState->dumpFileName = _strdup(argv[i]);
        }
      else if (strcmp(argv[i],"-tt") == 0)
        {
          i++;
          argc--;
          switch(atoi(argv[i]))
            {
            case 0:
              myAppState->triangleType = DISJOINT_TRIANGLES;
              break;
            case 1:
              myAppState->triangleType = TRIANGLE_STRIPS;
              break;
            case 2:
              myAppState->triangleType = INDEXED_DISJOINT_TRIANGLES;
              break;
            case 3:
              myAppState->triangleType = INDEXED_TRIANGLE_STRIPS;
              break;
            default:
              printf(" unrecognized triangle type %d. Valid values are %s \n", atoi(argv[i]), "0, 1, 2, or 3");
              exit(0);
            }
        }
      else if (strcmp(argv[i], "-line") == 0)
        {
          myAppState->outlineMode = 1;
        }
      else if (strcmp(argv[i], "-clear") == 0)
        {
          myAppState->clearPerFrame = 1;
        }
      else if (strcmp(argv[i],"-doareatest") == 0)
        {
          myAppState->doAreaTest = 1;
        }
      else if (strcmp(argv[i],"-dotxtest") == 0)
        {
          myAppState->doTextureTest = 1;
        }
      else if (strcmp(argv[i],"-dovbtest") == 0)
        {
          myAppState->doVBufSizeTest = 1;
        }
      else if (strcmp(argv[i], "-retained") == 0)
        {
          myAppState->retainedModeEnabled = 1;
        }
      else
        {
          fprintf(stderr,"Unrecognized argument: %s \n", argv[i]);
          usage(argv[0]);
          exit(-1);
        }
      i++;
      argc--;
    }
}

void normalizeNormal(Vertex3D *n)
{
  double d = n->x*n->x + n->y*n->y + n->z*n->z;
  d = sqrt(d);
  if (d != 0.0)
    d = 1.0/d;
  n->x *= d;
  n->y *= d;
  n->z *= d;
}

static GLuint textureName;
void
buildAndDownloadTexture(int textureSize)
{
  Color4DUbyte *texturePixels;
  int i, j, indx;;

#if 0
  Color4DUbyte green={0x0, 0xFF, 0x0, 0xFF};
  Color4DUbyte red={0xFF, 0x00, 0x0, 0xFF};
#endif
  Color4DUbyte green={0x0, 0xFF, 0x0, 0x3F};
  Color4DUbyte red={0xFF, 0x00, 0x0, 0x3F};

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  glGenTextures(1, &textureName);
  glBindTexture(GL_TEXTURE_2D, textureName);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);


  texturePixels = (Color4DUbyte *)malloc(sizeof(Color4DUbyte)*textureSize*textureSize);

  indx = 0;
  for (j=0;j<textureSize;j++)
    {
      Color4DUbyte *even, *odd;
      if ((j&1) == 0)                 /* odd */
        {
          even = &green;
          odd = &red;
        }
      else
        {
          even = &red;
          odd = &green;
        }

      for (i=0;i<textureSize;i++)
        {
          if ((i&1) == 0)
            texturePixels[indx] = *even;
          else
            texturePixels[indx] = *odd;
          indx++;
        }
    }

  /* it would be nice to add another parameter to select the internal
     texture format from the command line. */
  glTexImage2D(GL_TEXTURE_2D, 0, TEXTURE_STORAGE_MODE, textureSize, textureSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, (const GLvoid *)texturePixels);

  free((void *)texturePixels);

  glFinish();
  check_gl_errors ();

}

void
cleanupTexture(void)
{
  glDeleteTextures(1, &textureName);
}

void
buildIndexedDisjointTriangleArrays(int nVertsPerAxis,
                                   int triangleLimit,
                                   int *dispatchTriangles,
                                   int *dispatchVertexCount,
                                   Vertex2D *baseVerts,
                                   Color3D *baseColors,
                                   Vertex3D *baseNormals,
                                   Vertex2D *baseTCs,
                                   Vertex2D **dispatchVerts,
                                   Color3D **dispatchColors,
                                   Vertex3D **dispatchNormals,
                                   Vertex2D **dispatchTCs,
                                   GLuint **dispatchIndices)
{
  /* now, create and populate the arrays that will be used to dispatch
     triangle data off to OpenGL */

  int i, j, sIndx, dIndx;
  Vertex2D *dv, *dtc;
  Color3D *dc;
  Vertex3D *dn;
  GLuint *di;
  int nVertsTotal = (nVertsPerAxis+1)*(nVertsPerAxis+1);
  *dispatchTriangles = nVertsPerAxis*nVertsPerAxis*2;

  /*
   * Note: we could sidestep this extra malloc for the dispatch arrays.
   * However, to avoid conditionals later in the code where arrays are
   * freed, we'll just do the extra malloc here.
   */
  dv = *dispatchVerts = (Vertex2D *)malloc(sizeof(Vertex2D)*nVertsTotal);
  memcpy((void *)dv, (void *)baseVerts, sizeof(Vertex2D)*nVertsTotal);

  dc = *dispatchColors = (Color3D *)malloc(sizeof(Color3D)*nVertsTotal);
  memcpy((void *)dc, (void *)baseColors, sizeof(Color3D)*nVertsTotal);

  dn = *dispatchNormals = (Vertex3D *)malloc(sizeof(Vertex3D)*nVertsTotal);
  memcpy((void *)dn, (void *)baseNormals, sizeof(Vertex3D)*nVertsTotal);

  dtc = *dispatchTCs = (Vertex2D *)malloc(sizeof(Vertex2D)*nVertsTotal);
  memcpy((void *)dtc, (void *)baseTCs, sizeof(Vertex2D)*nVertsTotal);

  di = *dispatchIndices = (GLuint *)malloc(sizeof(GLuint)*(*dispatchTriangles)*3);

  dIndx = 0;

  for (j=0;j<nVertsPerAxis;j++)
    {
      for (i=0;i<nVertsPerAxis;i++)
        {
          /* for each mesh quad, copy over verts for two triangles */
          sIndx = j*(nVertsPerAxis+1) + i;

          /* triangle 0 */
          di[dIndx] = sIndx;
          dIndx++;

          di[dIndx] = sIndx+1;
          dIndx++;

          di[dIndx] = sIndx+nVertsPerAxis+1;
          dIndx++;

          /* triangle 1 */
          di[dIndx] = sIndx+nVertsPerAxis+1;
          dIndx++;

          di[dIndx] = sIndx+1;
          dIndx++;

          di[dIndx] = sIndx+1+nVertsPerAxis+1;
          dIndx++;
        }
    }

  /*
   * now we have a big pile of disjoint triangles. how many do we
   * actually send down with each call?
   */
  if (*dispatchTriangles < triangleLimit)
    *dispatchVertexCount = *dispatchTriangles*3;
  else
    *dispatchVertexCount = triangleLimit*3;
  *dispatchTriangles = *dispatchVertexCount/3;
}

void
buildIndexedTriangleStripArrays(int nVertsPerAxis,
                                int triangleLimit,
                                int *dispatchTriangles,
                                int *dispatchVertexCount,
                                Vertex2D *baseVerts,
                                Color3D *baseColors,
                                Vertex3D *baseNormals,
                                Vertex2D *baseTCs,
                                Vertex2D **dispatchVerts,
                                Color3D **dispatchColors,
                                Vertex3D **dispatchNormals,
                                Vertex2D **dispatchTCs,
                                GLuint **dispatchIndices)
{
  /* now, create and populate the arrays that will be used to dispatch
     triangle data off to OpenGL */

  int i, j, sIndx, dIndx;
  Vertex2D *dv, *dtc;
  Color3D *dc;
  Vertex3D *dn;
  GLuint *di;
  int nVertsTotal = (nVertsPerAxis+1)*(nVertsPerAxis+1);
  *dispatchTriangles = (nVertsPerAxis-1)*((nVertsPerAxis+1)*2+2) + (nVertsPerAxis+1)*2 - 2;

  /*
   * Note: we could sidestep this extra malloc for the dispatch arrays.
   * However, to avoid conditionals later in the code where arrays are
   * freed, we'll just do the extra malloc here.
   */
  dv = *dispatchVerts = (Vertex2D *)malloc(sizeof(Vertex2D)*nVertsTotal);
  memcpy((void *)dv, (void *)baseVerts, sizeof(Vertex2D)*nVertsTotal);

  dc = *dispatchColors = (Color3D *)malloc(sizeof(Color3D)*nVertsTotal);
  memcpy((void *)dc, (void *)baseColors, sizeof(Color3D)*nVertsTotal);

  dn = *dispatchNormals = (Vertex3D *)malloc(sizeof(Vertex3D)*nVertsTotal);
  memcpy((void *)dn, (void *)baseNormals, sizeof(Vertex3D)*nVertsTotal);

  dtc = *dispatchTCs = (Vertex2D *)malloc(sizeof(Vertex2D)*nVertsTotal);
  memcpy((void *)dtc, (void *)baseTCs, sizeof(Vertex2D)*nVertsTotal);

  di = *dispatchIndices = (GLuint *)malloc(sizeof(GLuint)*(*dispatchTriangles)+2);

  dIndx = 0;

  for (j=0;j<nVertsPerAxis;j++)
    {
      for (i=0;i<nVertsPerAxis+1;i++)
        {
          /* for each mesh quad, copy over verts for two triangles */
          sIndx = j*(nVertsPerAxis+1) + i;

          /* vertex (i,j) */
          di[dIndx] = sIndx;
          dIndx++;

          /* vertex (i,j+1) */
          di[dIndx] = sIndx+nVertsPerAxis+1;
          dIndx++;
        }

      if (j != nVertsPerAxis-1)
        {
          /* add vertices for degenerate triangles that link up
             one row to another */

          /* add vertex (i, j+1) again */
          di[dIndx] = sIndx+nVertsPerAxis+1;
          dIndx++;

          /* now add vertex (0, j+1) */
          sIndx = (j+1)*(nVertsPerAxis+1);
          di[dIndx] = sIndx;
          dIndx++;
        }
    }


  /*
   * now we have a big pile of disjoint triangles. how many do we
   * actually send down with each call?
   */

  if (*dispatchTriangles < triangleLimit)
    *dispatchVertexCount = *dispatchTriangles+2;
  else
    *dispatchVertexCount = triangleLimit+2;
  *dispatchTriangles = *dispatchVertexCount-2;
}

void
buildDisjointTriangleArrays(int nVertsPerAxis,
                            int triangleLimit,
                            int *dispatchTriangles,
                            int *dispatchVertexCount,
                            Vertex2D *baseVerts,
                            Color3D *baseColors,
                            Vertex3D *baseNormals,
                            Vertex2D *baseTCs,
                            Vertex2D **dispatchVerts,
                            Color3D **dispatchColors,
                            Vertex3D **dispatchNormals,
                            Vertex2D **dispatchTCs)
{

  /* now, create and populate the arrays that will be used to dispatch
     triangle data off to OpenGL */

  int i, j, sIndx, dIndx;
  Vertex2D *dv, *dtc;
  Color3D *dc;
  Vertex3D *dn;
  *dispatchTriangles = nVertsPerAxis*nVertsPerAxis*2;

  dv = *dispatchVerts = (Vertex2D *)malloc(sizeof(Vertex2D)*(*dispatchTriangles)*3);
  dc = *dispatchColors = (Color3D *)malloc(sizeof(Color3D)*(*dispatchTriangles)*3);
  dn = *dispatchNormals = (Vertex3D *)malloc(sizeof(Vertex3D)*(*dispatchTriangles)*3);
  dtc = *dispatchTCs = (Vertex2D *)malloc(sizeof(Vertex2D)*(*dispatchTriangles)*3);

  dIndx = 0;
  for (j=0;j<nVertsPerAxis;j++)
    {
      for (i=0;i<nVertsPerAxis;i++)
        {
          /* for each mesh quad, copy over verts for two triangles */
          sIndx = j*(nVertsPerAxis+1) + i;

          /* triangle 0 */
          dv[dIndx] = baseVerts[sIndx];
          dc[dIndx] = baseColors[sIndx];
          dn[dIndx] = baseNormals[sIndx];
          dtc[dIndx] = baseTCs[sIndx];
          dIndx++;

          dv[dIndx] = baseVerts[sIndx+1];
          dc[dIndx] = baseColors[sIndx+1];
          dn[dIndx] = baseNormals[sIndx+1];
          dtc[dIndx] = baseTCs[sIndx+1];
          dIndx++;

          dv[dIndx] = baseVerts[sIndx+nVertsPerAxis+1];
          dc[dIndx] = baseColors[sIndx+nVertsPerAxis+1];
          dn[dIndx] = baseNormals[sIndx+nVertsPerAxis+1];
          dtc[dIndx] = baseTCs[sIndx+nVertsPerAxis+1];
          dIndx++;

          /* triangle 1 */
          dv[dIndx] = baseVerts[sIndx+nVertsPerAxis+1];
          dc[dIndx] = baseColors[sIndx+nVertsPerAxis+1];
          dn[dIndx] = baseNormals[sIndx+nVertsPerAxis+1];
          dtc[dIndx] = baseTCs[sIndx+nVertsPerAxis+1];
          dIndx++;

          dv[dIndx] = baseVerts[sIndx+1];
          dc[dIndx] = baseColors[sIndx+1];
          dn[dIndx] = baseNormals[sIndx+1];
          dtc[dIndx] = baseTCs[sIndx+1];
          dIndx++;

          dv[dIndx] = baseVerts[sIndx+1+nVertsPerAxis+1];
          dc[dIndx] = baseColors[sIndx+1+nVertsPerAxis+1];
          dn[dIndx] = baseNormals[sIndx+1+nVertsPerAxis+1];
          dtc[dIndx] = baseTCs[sIndx+1+nVertsPerAxis+1];
          dIndx++;
        }
    }

  /*
   * now we have a big pile of disjoint triangles. how many do we
   * actually send down with each call?
   */
  if (*dispatchTriangles < triangleLimit)
    *dispatchVertexCount = *dispatchTriangles*3;
  else
    *dispatchVertexCount = triangleLimit*3;
  *dispatchTriangles = *dispatchVertexCount/3;
}

void
buildTriangleStripArrays(int nVertsPerAxis,
                         int triangleLimit,
                         int *dispatchTriangles,
                         int *dispatchVertexCount,
                         Vertex2D *baseVerts,
                         Color3D *baseColors,
                         Vertex3D *baseNormals,
                         Vertex2D *baseTCs,
                         Vertex2D **dispatchVerts,
                         Color3D **dispatchColors,
                         Vertex3D **dispatchNormals,
                         Vertex2D **dispatchTCs)
{

  /* now, create and populate the arrays that will be used to dispatch
     triangle data off to OpenGL */

  int i, j, sIndx, dIndx;
  int trianglesPerRow = (nVertsPerAxis-1)*2 + 2; /* +2 degenerate tris per row */
  Vertex2D *dv, *dtc;
  Color3D *dc;
  Vertex3D *dn;
  *dispatchTriangles = (nVertsPerAxis-1)*((nVertsPerAxis+1)*2+2) + (nVertsPerAxis+1)*2 - 2;

  dv = *dispatchVerts = (Vertex2D *)malloc(sizeof(Vertex2D)*(*dispatchTriangles+2));
  dc = *dispatchColors = (Color3D *)malloc(sizeof(Color3D)*(*dispatchTriangles+2));
  dn = *dispatchNormals = (Vertex3D *)malloc(sizeof(Vertex3D)*(*dispatchTriangles+2));
  dtc = *dispatchTCs = (Vertex2D *)malloc(sizeof(Vertex2D)*(*dispatchTriangles+2));

  dIndx = 0;

  for (j=0;j<nVertsPerAxis;j++)
    {
      for (i=0;i<nVertsPerAxis+1;i++)
        {
          /* for each mesh quad, copy over verts for two triangles */
          sIndx = j*(nVertsPerAxis+1) + i;

          /* vertex (i,j) */
          dv[dIndx] = baseVerts[sIndx];
          dc[dIndx] = baseColors[sIndx];
          dn[dIndx] = baseNormals[sIndx];
          dtc[dIndx] = baseTCs[sIndx];
          dIndx++;

          /* vertex (i,j+1) */
          dv[dIndx] = baseVerts[sIndx+nVertsPerAxis+1];
          dc[dIndx] = baseColors[sIndx+nVertsPerAxis+1];
          dn[dIndx] = baseNormals[sIndx+nVertsPerAxis+1];
          dtc[dIndx] = baseTCs[sIndx+nVertsPerAxis+1];
          dIndx++;
        }

      if (j != nVertsPerAxis-1)
        {
          /* add vertices for degenerate triangles that link up
             one row to another */

          /* add vertex (i, j+1) again */
          dv[dIndx] = baseVerts[sIndx+nVertsPerAxis+1];
          dc[dIndx] = baseColors[sIndx+nVertsPerAxis+1];
          dn[dIndx] = baseNormals[sIndx+nVertsPerAxis+1];
          dtc[dIndx] = baseTCs[sIndx+nVertsPerAxis+1];
          dIndx++;

          /* now add vertex (0, j+1) */
          sIndx = (j+1)*(nVertsPerAxis+1);
          dv[dIndx] = baseVerts[sIndx];
          dc[dIndx] = baseColors[sIndx];
          dn[dIndx] = baseNormals[sIndx];
          dtc[dIndx] = baseTCs[sIndx];
          dIndx++;
        }
    }

  /*
   * now we have a big pile of disjoint triangles. how many do we
   * actually send down with each call?
   */

  if (*dispatchTriangles < triangleLimit)
    *dispatchVertexCount = *dispatchTriangles+2;
  else
    *dispatchVertexCount = triangleLimit+2;
  *dispatchTriangles = *dispatchVertexCount-2;
#if 0
  printf("tstrip vertex count=%d\n", *dispatchVertexCount);
  printf("nverts per axis = %d\n", nVertsPerAxis);
  printf("ntris per row = %d\n",trianglesPerRow);
#endif
}

void
buildBaseArrays(float triangleAreaPixels,
                int screenWidth,
                int screenHeight,
                int *nVertsPerAxis,
                Vertex2D **baseVerts,
                Color3D **baseColors,
                Vertex3D **baseNormals,
                Vertex2D **baseTCs)
{
  /* construct base arrays for qmesh */

  int usablePixels = screenWidth >> 1;
  int i,j, indx=0;
  float x, y;
  float r, g, dr, dg;
  float s, ds, t, dt;
  Vertex2D *bv, *btc;
  Vertex3D *bn;
  Color3D *bc;

  /*
   * we're going to construct a mesh that has vertex spacing
   * at an interval such that each mesh quad will have two
   * triangles whose area sums to triangleAreaPixels*2.
   */
  double spacing = sqrt(triangleAreaPixels*2.0);
  *nVertsPerAxis = (int)((double)usablePixels/spacing);

  bv = *baseVerts = (Vertex2D *)malloc(sizeof(Vertex2D)*(*nVertsPerAxis+1)*(*nVertsPerAxis+1));
  bc = *baseColors = (Color3D *)malloc(sizeof(Color3D)*(*nVertsPerAxis+1)*(*nVertsPerAxis+1));
  bn = *baseNormals = (Vertex3D *)malloc(sizeof(Vertex3D)*(*nVertsPerAxis+1)*(*nVertsPerAxis+1));
  btc = *baseTCs = (Vertex2D *)malloc(sizeof(Vertex2D)*(*nVertsPerAxis+1)*(*nVertsPerAxis+1));

  y= 0.25F * screenWidth;
  g = 0.0F;

  dg = dr = 1.0F/(float)*nVertsPerAxis;

  t = 0.0F;
  dt = ds = 1.0/(float)*nVertsPerAxis;

  /* red grows along x axis, green along y axis, b=1 constant */

  for (j=0;j<*nVertsPerAxis+1;j++,y+=spacing, g+=dg, t+=dt)
    {
      x = 0.25 * screenWidth;
      r = 0.0;
      s = 0.0F;

      for (i=0;i<*nVertsPerAxis+1;i++, x+=spacing, r+=dr, s+=ds)
        {
          bv[indx].x = x;
          bv[indx].y = y;

          bc[indx].r = r;
          bc[indx].g = g;
          bc[indx].b = 1.0F;

          bn[indx].x = x-(float)screenWidth*0.5;
          bn[indx].y = y-(float)screenHeight*0.5;
          bn[indx].z = (float)(screenWidth+screenHeight)*.25;
          normalizeNormal(bn+indx);

          btc[indx].x = s;
          btc[indx].y = t;

          indx++;
        }
    }
}

void
wesTriangleRateBenchmark(AppState *as)
{
  int startTime, endTime;
  size_t totalTris=0, totalVerts=0;
  float elapsedTimeSeconds, trisPerSecond, vertsPerSecond;
  Vertex2D *baseVerts;
  Color3D *baseColors;
  Vertex3D *baseNormals;
  Vertex2D *baseTCs;
  GLuint trianglesListIndx = 0;

  int nVertsPerAxis;
  int nFrames=0;

  Vertex2D *dispatchVerts=NULL;
  Color3D *dispatchColors=NULL;
  Vertex3D *dispatchNormals=NULL;
  Vertex2D *dispatchTCs=NULL;
  GLuint   *dispatchIndices=NULL;
  int dispatchVertexCount, dispatchTriangles;

  int screenWidth = as->imgWidth;
  GLfloat r=screenWidth;
  int screenHeight = as->imgHeight;
  int testDurationSeconds = as->testDurationSeconds;
  size_t triangleLimit = as->triangleLimit;
  double triangleAreaPixels = as->triangleAreaInPixels;
  int lighting=as->enableLighting;
  int colorMaterial = as->enableColorMaterial;
  int texturingEnabled = (as->textureSize > 0) ? 1 : 0;
  int textureSize = as->textureSize;

  /*
   * Objective(s):
   * 1. Want triangles with a specified area (e.g., 8sq pixels) for the
   * purpose of generating a graphics load with reasonably precise
   * characteristics .
   * 2. Want triangles to *all* remain on-screen so that triangle rate
   * reflects the cost of rendering visible triangles/fragments.
   *
   * Approach:
   * 1. build a base mesh of vertices. This set of verts is positioned
   * so that it will remain entirely within the view frustum no matter
   * how we rotate it (in 2D)
   * 2. dispatch off the mesh vertices as disjoint triangles using
   * vertex arrays. This approach is not the most efficient way to
   * represent densely packed triangles, but has a couple of benefits:
   * - It allows us to easily set the maximum number of triangles per
   *   frame to be dispatched off to the renderer w/o having to resort
   *   to tricks like inserting degenerate triangles in strips to connect
   *   up two disjoint strips.
   * - It allows us to glDrawArrays rather than glDrawElements. The latter
   *   uses one level of indirection, and will be less efficient.
   * - In the end, we care about vertex rate, not triangle rate.
   *
   * An early version of this code used the glDrawElements with indices.
   * Rather than insert compile-time switches, this old code is simply
   * cut-n-pasted and commented out at the end of this file.
   */

  /* Make sure we are drawing to the front buffer */
  glDrawBuffer(GL_FRONT);
  glDisable(GL_DEPTH_TEST);

  /* Clear the screen */
  glClear(GL_COLOR_BUFFER_BIT);

  if (as->outlineMode != 0)
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  else
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  /* Load identities */
  glMatrixMode( GL_PROJECTION );
  glPushMatrix();
  glLoadIdentity( );

  glMatrixMode( GL_MODELVIEW );
  glPushMatrix();
  glLoadIdentity();

  /* change range from -1..1 to 0..[screenWidth, screenHeight] */
  glTranslatef(-1.0, -1.0, 0.0);
  glScalef(2.0/r, 2.0/r, 1.0F);


  if (lighting)
    {
      /* turn on OpenGL lighting stuff */
      GLfloat lightPosition[] = {(float)screenWidth*0.5F, (float)screenHeight*0.5F, 200.0F, 1.0F};
      glEnable(GL_LIGHTING);
      glEnable(GL_LIGHT0);
      glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

      if (colorMaterial)
        {
          glEnable(GL_COLOR_MATERIAL);
          glColorMaterial(GL_FRONT, GL_DIFFUSE);
        }
      /* just use OpenGL's default light values: directional light source */
    }
  else
    {
      glDisable(GL_LIGHTING);
      glDisable(GL_COLOR_MATERIAL);
    }

  if (texturingEnabled)
    {
      buildAndDownloadTexture(textureSize);
      glEnable(GL_TEXTURE_2D);
      glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
    }
  else
    glDisable(GL_TEXTURE_2D);

  /* build the base quadmesh vertex array */
  buildBaseArrays(triangleAreaPixels, as->imgWidth, as->imgHeight,
                  &nVertsPerAxis,
                  &baseVerts, &baseColors, &baseNormals, &baseTCs);

  /* now, repackage that information into bundles suitable for submission
     to GL using the specified primitive type*/
  if (as->triangleType == DISJOINT_TRIANGLES)
    {
      int triangleLimit;

      if ((as->triangleLimit*3) > as->vertexBufLimit)
        triangleLimit = as->vertexBufLimit/3;
      else
        triangleLimit = as->triangleLimit;

      buildDisjointTriangleArrays(nVertsPerAxis,
                                  triangleLimit,
                                  &dispatchTriangles,
                                  &dispatchVertexCount,
                                  baseVerts, baseColors,
                                  baseNormals, baseTCs,
                                  &dispatchVerts,
                                  &dispatchColors,
                                  &dispatchNormals,
                                  &dispatchTCs);
      as->computedVertsPerArrayCall = dispatchVertexCount;
      as->computedIndicesPerArrayCall = 0;
    }
  else if (as->triangleType == TRIANGLE_STRIPS)
    {

      if ((as->triangleLimit+2) > as->vertexBufLimit)
        triangleLimit = as->vertexBufLimit-2;
      else
        triangleLimit = as->triangleLimit;


      buildTriangleStripArrays(nVertsPerAxis,
                               triangleLimit,
                               &dispatchTriangles,
                               &dispatchVertexCount,
                               baseVerts, baseColors,
                               baseNormals, baseTCs,
                               &dispatchVerts,
                               &dispatchColors,
                               &dispatchNormals,
                               &dispatchTCs);
      as->computedVertsPerArrayCall = dispatchVertexCount;
      as->computedIndicesPerArrayCall = 0;
    }
  else if (as->triangleType == INDEXED_DISJOINT_TRIANGLES)
    {
      if ((as->triangleLimit*3) > as->vertexBufLimit)
        triangleLimit = as->vertexBufLimit/3;
      else
        triangleLimit = as->triangleLimit;

      buildIndexedDisjointTriangleArrays(nVertsPerAxis,
                                         triangleLimit,
                                         &dispatchTriangles,
                                         &dispatchVertexCount,
                                         baseVerts, baseColors,
                                         baseNormals, baseTCs,
                                         &dispatchVerts,
                                         &dispatchColors,
                                         &dispatchNormals,
                                         &dispatchTCs,
                                         &dispatchIndices);
      as->computedVertsPerArrayCall = (nVertsPerAxis+1)*(nVertsPerAxis+1);
      as->computedIndicesPerArrayCall = dispatchVertexCount;
    }
  else if (as->triangleType == INDEXED_TRIANGLE_STRIPS)
    {

      if ((as->triangleLimit+2) > as->vertexBufLimit)
        triangleLimit = as->vertexBufLimit-2;
      else
        triangleLimit = as->triangleLimit;

      buildIndexedTriangleStripArrays(nVertsPerAxis,
                                      triangleLimit,
                                      &dispatchTriangles,
                                      &dispatchVertexCount,
                                      baseVerts, baseColors,
                                      baseNormals, baseTCs,
                                      &dispatchVerts,
                                      &dispatchColors,
                                      &dispatchNormals,
                                      &dispatchTCs,
                                      &dispatchIndices);
      as->computedVertsPerArrayCall = (nVertsPerAxis+1)*(nVertsPerAxis+1);
      as->computedIndicesPerArrayCall = dispatchVertexCount;
    }


  /* Set up the pointers */
  glVertexPointer(2, GL_FLOAT, 0, (const GLvoid *)dispatchVerts);
  glEnableClientState(GL_VERTEX_ARRAY);
  glColorPointer(3, GL_FLOAT, 0, (const GLvoid *)dispatchColors);
  glEnableClientState(GL_COLOR_ARRAY);

  if (lighting)
    {
      glNormalPointer(GL_FLOAT, 0, (const GLvoid *)dispatchNormals);
      glEnableClientState(GL_NORMAL_ARRAY);
    }

  if (texturingEnabled)
    {
      glTexCoordPointer(2, GL_FLOAT, 0, (const GLvoid *)dispatchTCs);
      glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    }

  glFinish();                 /* make sure all setup is finished */

  startTime = endTime = glfwGetTime();

  while ((endTime - startTime) < testDurationSeconds)
    {

      int buildingList=0;

      if (as->clearPerFrame != 0)
        glClear(GL_COLOR_BUFFER_BIT); /* tmp for debug */

#if SCREENSHOT_MODE
      /* screenshot mode won't work with -retained */
      /* Clear the screen */
      glClear(GL_COLOR_BUFFER_BIT);
      glClear(GL_COLOR_BUFFER_BIT);
      glDrawArrays(GL_TRIANGLES, 0, dispatchVertexCount);
      glDrawArrays(GL_TRIANGLES, 0, dispatchVertexCount);
      fprintf(stderr," You have screenshot mode enabled! \n");
      fflush(stderr);
      sleep(5);
      break;
#endif

      if (as->retainedModeEnabled != 0)
        {
          if (trianglesListIndx == 0) /* no list yet, build one */
            {
              trianglesListIndx = glGenLists(1);
              /*              glNewList(trianglesListIndx, GL_COMPILE_AND_EXECUTE); */
              glNewList(trianglesListIndx, GL_COMPILE);
              buildingList = 1;
            }
          else
            glCallList(trianglesListIndx);
        }

      if ((buildingList == 0) || (as->retainedModeEnabled != 0))
        {
          switch (as->triangleType)
            {
            case DISJOINT_TRIANGLES:
              glDrawArrays(GL_TRIANGLES, 0, dispatchVertexCount);
              break;
            case TRIANGLE_STRIPS:
              glDrawArrays(GL_TRIANGLE_STRIP, 0, dispatchVertexCount);
              break;
            case INDEXED_DISJOINT_TRIANGLES:
              glDrawElements(GL_TRIANGLES, dispatchVertexCount, GL_UNSIGNED_INT, dispatchIndices);
              break;
            case INDEXED_TRIANGLE_STRIPS:
              glDrawElements(GL_TRIANGLE_STRIP, dispatchVertexCount, GL_UNSIGNED_INT, dispatchIndices);
              break;
            default:
              break;
            }
        }

      glTranslatef(r/2.0, r/2.0, 0.0F);
      glRotatef(0.01F, 0.0F, 0.0F, 1.0F);
      glTranslatef(-r/2.0, -r/2.0, 0.0F);

      if (buildingList == 1)
        {
          glEndList();
          glCallList(trianglesListIndx);

          /* cheat - reset the clock to eliminate the time required
             for creating the display list from the frame rate computation */
          startTime = endTime = glfwGetTime();
        }

      endTime = glfwGetTime();

      nFrames++;
      totalTris += dispatchTriangles;
      totalVerts += as->computedVertsPerArrayCall;

    }

  glFinish();
  endTime = glfwGetTime();

  /* Restore the gl stack */
  glMatrixMode( GL_MODELVIEW );
  glPopMatrix();

  glMatrixMode( GL_PROJECTION );
  glPopMatrix();

  /* Before printing the results, make sure we didn't have
  ** any GL related errors. */
  check_gl_errors();

  /* the pause that refreshes -- */
#ifdef _WIN32
  Sleep(250);
#else
  usleep(250000);
#endif

  
  
  /* TODO: compute/return the FPS */
  
  float myTime = (endTime - startTime) / 1000.0f; //time in sec  
  myAppState.computedMVertexOpsPerSecond = (totalVerts / 1E6) / myTime; /* TODO: compute/return the MVertexOps/sec */ /*DONE*/
  myAppState.computedMTrisPerSecond = (totalTris / 1E6) / myTime; /* TODO: compute/return the Mtri/seconds */ /*DONE*/
  myAppState.computedMFragsPerSecond = myAppState.computedMTrisPerSecond * myAppState.triangleAreaInPixels; /* TODO: compute/return the mFrags/sec */ /*DONE*/
  
  
  printf("verts/frame = %d \n", dispatchVertexCount);
  printf("nframes = %d \n", nFrames);

  free((void *)baseVerts);
  free((void *)baseColors);
  free((void *)baseNormals);
  free((void *)baseTCs);
  free((void *)dispatchVerts);
  free((void *)dispatchColors);
  free((void *)dispatchNormals);
  free((void *)dispatchTCs);

  if (dispatchIndices != NULL)
    free((void *)dispatchIndices);

  if (texturingEnabled)
    cleanupTexture();
}


/* A simple routine which checks for GL errors. */
void check_gl_errors (void) {
  GLenum err;
  err = glGetError();
  if (err != GL_NO_ERROR) {
    fprintf (stderr, "GL Error: ");
    switch (err) {
    case GL_INVALID_ENUM:
      fprintf (stderr, "Invalid Enum\n");
      break;
    case GL_INVALID_VALUE:
      fprintf (stderr, "Invalid Value\n");
      break;
    case GL_INVALID_OPERATION:
      fprintf (stderr, "Invalid Operation\n");
      break;
    case GL_STACK_OVERFLOW:
      fprintf (stderr, "Stack Overflow\n");
      break;
    case GL_STACK_UNDERFLOW:
      fprintf (stderr, "Stack Underflow\n");
      break;
    case GL_OUT_OF_MEMORY:
      fprintf (stderr, "Out of Memory\n");
      break;
    default:
      fprintf (stderr, "Unknown\n");
    }
    exit(1);
  }
}

void Init (void) {
  /* Init does nothing */
}

void Reshape (int x, int y) {
  /* Reshape does nothing */
  (void) x;
  (void) y;
}


void Key(unsigned char a, int x, int y) {
  /* Key does nothing */
  (void) x;
  (void) y;
  (void) a;
}

static void error_callback(int error, const char* description)
{
  fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GLFW_TRUE);
}

int
main(int argc, char **argv)
{
	
  myAppState.appName = _strdup(argv[0]);

  myAppState.triangleAreaInPixels=DEFAULT_TRIANGLE_AREA;
  myAppState.triangleType = DEFAULT_TRIANGLE_TYPE;
  myAppState.testDurationSeconds=DEFAULT_TEST_DURATION_SECONDS;
  myAppState.imgWidth = DEFAULT_WIN_WIDTH;
  myAppState.imgHeight = DEFAULT_WIN_HEIGHT;
  myAppState.triangleLimit = DEFAULT_TRIANGLE_LIMIT;
  myAppState.vertexBufLimit = DEFAULT_VERTEXBUF_LIMIT;
  myAppState.textureSize = DEFAULT_TEXTURE_SIZE;
  myAppState.enableLighting = DEFAULT_LIGHTING_ENABLED;
  myAppState.enableColorMaterial = DEFAULT_COLOR_MATERIAL;
  myAppState.dumpFileName = _strdup(DEFAULT_DUMP_FILE_NAME);
  myAppState.retainedModeEnabled = DEFAULT_RETAINED_MODE_ENABLED;
  myAppState.clearPerFrame = DEFAULT_CLEAR_PER_FRAME;
  myAppState.outlineMode = DEFAULT_OUTLINE_MODE_BOOL;
  myAppState.progName = _strdup(argv[0]);

  glfwSetErrorCallback(error_callback);

  if (!glfwInit())
    exit(EXIT_FAILURE);

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

  parseArgs(argc, argv, &myAppState);

  GLFWwindow* window = glfwCreateWindow(DEFAULT_WIN_WIDTH, DEFAULT_WIN_HEIGHT, argv[0], NULL, NULL);
  if (!window)
    {
      glfwTerminate();
      exit(EXIT_FAILURE);
    }

  glfwSetKeyCallback(window, key_callback);

  glfwMakeContextCurrent(window);
  // start GLEW extension handler
  // glewExperimental = GL_TRUE;
  glewInit();
  glfwSwapInterval(1);

  printInfo(window);

  GLuint program = glCreateProgram();
  // it appears that since we don't use vertex or fragment shaders
  // here, we don't need to link the program. MacOS executables work
  // only when we call this; Windows executables don't work when we
  // call this; so we'll only call it if we're not Windows.
#ifndef _WIN32
  glLinkProgram(program);
#endif

  Init();
  while (!glfwWindowShouldClose(window))
    {
      GLenum err;               /* Everyone is puzzled why this matters. */
      err = glGetError();       /* But this error-checking makes it work */
      glUseProgram(program);    /* on Linux */
      err = glGetError();       /* Weird, huh? */
      runBenchmark();
      glfwSwapBuffers(window);
      glfwPollEvents();
    }
  return 0;
}

void printInfo (GLFWwindow * window) {

  /* Display the gfx card information */
  printf( "--------------------------------------------------\n");
  printf ("Vendor:      %s\n", glGetString(GL_VENDOR));
  printf ("Renderer:    %s\n", glGetString(GL_RENDERER));
  printf ("Version:     %s\n", glGetString(GL_VERSION));
  GLFWmonitor * monitor = glfwGetPrimaryMonitor();
  const GLFWvidmode * vidMode = glfwGetVideoMode(monitor);
  printf( "Visual:      RGBA=<%d,%d,%d,%d>  Z=<%d>  double=%d\n",
          vidMode->redBits,
          vidMode->blueBits,
          vidMode->greenBits,
          GLFW_ALPHA_BITS,
          GLFW_DEPTH_BITS,
          1 );
  int winWidth, winHeight;
  glfwGetFramebufferSize(window, &winWidth, &winHeight);
  int xPos, yPos;
  glfwGetWindowPos(window, &xPos, &yPos);
  printf( "Geometry:    %dx%d+%d+%d\n",
          winWidth,
          winHeight,
          xPos,
          yPos);
  printf( "Screen:      %dx%d\n",
          vidMode->width,
          vidMode->height);
  printf( "--------------------------------------------------\n");


  /* print out the test parameters */

  printf("\n%s Setup parameters\n", myAppState.progName);
  printf("Triangle area\t%4.3f (pixels^2)\n", myAppState.triangleAreaInPixels);
  printf("Test duration\t%f(s)\n", myAppState.testDurationSeconds);
  printf("Screen W/H\t(%d,%d)\n", myAppState.imgWidth, myAppState.imgHeight);
  printf("Triangle limit\t%ld\n", myAppState.triangleLimit);
  printf("VertexBuf limit\t%ld\n", myAppState.vertexBufLimit);
  printf("Texture size\t(%d x %d)\n", myAppState.textureSize, myAppState.textureSize);
  printf("Lighting is \t%s\n", (myAppState.enableLighting != 0) ? "enabled" : "disabled");
  printf("ColorMaterial \t%s\n",(myAppState.enableColorMaterial != 0) ? "enabled" : "disabled");
  printf("Triangle type\t%d \n", myAppState.triangleType);
  printf("Retained mode\t%s\n", (myAppState.retainedModeEnabled != 0) ? "enabled" : "disabled");

}

void runBenchmark(void) {

  /* if not iterating over tests, just execute once */
  if ((myAppState.doAreaTest == 0) && (myAppState.doVBufSizeTest == 0) && (myAppState.doTextureTest == 0))
    {
      wesTriangleRateBenchmark(&myAppState);

      fprintf(stderr," WesBench: area=%2.1f px, tri rate = %3.2f Mtri/sec, vertex rate=%3.2f Mverts/sec, fill rate = %4.2f Mpix/sec, verts/bucket=%zu, indices/bucket=%zu\n", myAppState.triangleAreaInPixels, myAppState.computedMTrisPerSecond, myAppState.computedMVertexOpsPerSecond, myAppState.computedMFragsPerSecond, myAppState.computedVertsPerArrayCall, myAppState.computedIndicesPerArrayCall);
    }
  else
    {
      int i;

      FILE *df = fopen(myAppState.dumpFileName,"w");

      /* dump setup parms to dumpfile */
      fprintf(df, "\n%s Setup parameters\n", myAppState.progName);
      fprintf(df, "Triangle area\t%4.3f (pixels^2)\n", myAppState.triangleAreaInPixels);
      fprintf(df, "Test duration\t%f(s)\n", myAppState.testDurationSeconds);
      fprintf(df, "Screen W/H\t(%d,%d)\n", myAppState.imgWidth, myAppState.imgHeight);
      fprintf(df, "Triangle limit\t%ld\n", myAppState.triangleLimit);
      fprintf(df, "VertexBuf limit\t%ld\n", myAppState.vertexBufLimit);
      fprintf(df, "Texture size\t(%d x %d)\n", myAppState.textureSize, myAppState.textureSize);
      fprintf(df, "Lighting is \t%s\n", (myAppState.enableLighting != 0) ? "enabled" : "disabled");
      fprintf(df, "ColorMaterial \t%s\n",(myAppState.enableColorMaterial != 0) ? "enabled" : "disabled");
      fprintf(df, "Triangle type\t%d \n", myAppState.triangleType);
      fprintf(df, "Retained mode\t%s\n", (myAppState.retainedModeEnabled != 0) ? "enabled" : "disabled");


      if (myAppState.doAreaTest != 0)  /* iterate over triangle area */
        {
          fprintf(df,"Area\tMv/sec\tMF/sec\tMT/sec\tVs/buckt\tIs/buckt\n");
		  /* TODO: Currently we're testing only triangles with area of 1.0 pixels. */ /*DONE*/

		  for (size_t ar = 0; ar <= 17; ar++){ //from 1.... 128K
			  myAppState.triangleAreaInPixels = pow(2.0, double(ar)); //compute different area each time 
			  wesTriangleRateBenchmark(&myAppState);

			  fprintf(stderr, " %s: area=%2.1f px, tri rate = %3.2f Mtri/sec, vertex rate=%3.2f Mverts/sec, fill rate = %4.2f Mpix/sec, verts/bucket=%ld, indices/bucket=%ld\n", myAppState.appName, myAppState.triangleAreaInPixels, myAppState.computedMTrisPerSecond, myAppState.computedMVertexOpsPerSecond, myAppState.computedMFragsPerSecond, myAppState.computedVertsPerArrayCall, myAppState.computedIndicesPerArrayCall);
			  fprintf(df, "%2.1f\t%3.2f\t%4.2f\t%4.2f\t%ld\t%ld\n", myAppState.triangleAreaInPixels, myAppState.computedMVertexOpsPerSecond, myAppState.computedMFragsPerSecond, myAppState.computedMTrisPerSecond, myAppState.computedVertsPerArrayCall, myAppState.computedIndicesPerArrayCall);
		  }


        }
      else if (myAppState.doVBufSizeTest != 0) /* iterate over vbuf size */
        {
          fprintf(df,"Area\tMv/sec\tMF/sec\tMT/sec\tVs/buckt\tIs/buckt\n");
          /* TODO: Currently we're only testing a buffer size of 64 elements. */ /*DONE*/
          /* note: If we have large buffers (say, in the millions of
           * elements) with this program, you need to specify a really
           * small triangle area size, like -a 0.125 (area of each
           * triangle is 0.125 px^2)
           */
		  for (size_t bf = 1; bf <= 23; bf++){ 
			  if (bf >= 10){
				  myAppState.triangleAreaInPixels = 0.125f;//automatically change the triangle size when hits the limit 
				                                          //probably only work for a specific (my) graphic card 
			  }
			  myAppState.vertexBufLimit = pow(2.0,double(bf));//calc different buffer size 
			  wesTriangleRateBenchmark(&myAppState);
			  fprintf(stderr, "buffer size: %d\n", int(myAppState.vertexBufLimit));
			  fprintf(stderr, " %s: area=%2.1f px, tri rate = %3.2f Mtri/sec, vertex rate=%3.2f Mverts/sec, fill rate = %4.2f Mpix/sec, verts/bucket=%ld, indices/bucket=%ld\n", myAppState.appName, myAppState.triangleAreaInPixels, myAppState.computedMTrisPerSecond, myAppState.computedMVertexOpsPerSecond, myAppState.computedMFragsPerSecond, myAppState.computedVertsPerArrayCall, myAppState.computedIndicesPerArrayCall);
			  fprintf(df, "%2.1f\t%3.2f\t%4.2f\t%4.2f\t%ld\t%ld\n", myAppState.triangleAreaInPixels, myAppState.computedMVertexOpsPerSecond, myAppState.computedMFragsPerSecond, myAppState.computedMTrisPerSecond, myAppState.computedVertsPerArrayCall, myAppState.computedIndicesPerArrayCall);
		  }



        }
      else if (myAppState.doTextureTest != 0) /* iterate over texture sizes */
        {
          fprintf(df,"Area\tTxsize\tMv/sec\tMF/sec\tMT/sec\tVs/buckt\tIs/buckt\n");

          /* TODO: Currently we're only testing a texture size of 8 elements on a side. */ /*DONE*/

		  for (size_t tx = 3; tx <= 12; tx++){//start with texture size of 8x8 up to 4Kx4K
			  myAppState.textureSize =pow(2.0,double(tx)); //calc different texture size 
			  wesTriangleRateBenchmark(&myAppState);			  
			  fprintf(stderr, " %s: area=%2.1f px, txsize=%d, tri rate = %3.2f Mtri/sec, vertex rate=%3.2f Mverts/sec, fill rate = %4.2f Mpix/sec, verts/bucket=%ld, indices/bucket=%ld\n", myAppState.appName, myAppState.triangleAreaInPixels, myAppState.textureSize, myAppState.computedMTrisPerSecond, myAppState.computedMVertexOpsPerSecond, myAppState.computedMFragsPerSecond, myAppState.computedVertsPerArrayCall, myAppState.computedIndicesPerArrayCall);
			  fprintf(df, "%2.1f\t%d\t%3.2f\t%4.2f\t%4.2f\t%ld\t%ld\n", myAppState.triangleAreaInPixels, myAppState.textureSize, myAppState.computedMVertexOpsPerSecond, myAppState.computedMFragsPerSecond, myAppState.computedMTrisPerSecond, myAppState.computedVertsPerArrayCall, myAppState.computedIndicesPerArrayCall);
			  fflush(df);
		  }


        }
      fclose(df);
    }

  glfwTerminate();
  exit(0);
}
/* EOF */
