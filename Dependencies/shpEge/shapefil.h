#ifndef SHAPEFILE_H_INCLUDED
#define SHAPEFILE_H_INCLUDED

/******************************************************************************
 *
 * Project:  Shapelib
 * Purpose:  Primary include file for Shapelib.
 * Author:   Frank Warmerdam, warmerdam@pobox.com
 *
 ******************************************************************************
 * Copyright (c) 1999, Frank Warmerdam
 * Copyright (c) 2012-2016, Even Rouault <even dot rouault at spatialys.com>
 *
 * SPDX-License-Identifier: MIT OR LGPL-2.0-or-later
 ******************************************************************************
 *
 */

#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifdef USE_CPL
#include "cpl_conv.h"
#endif




/************************************************************************/
/*           Version related macros (added in 1.6.0)                    */
/************************************************************************/

#define SHAPELIB_VERSION_MAJOR 1
#define SHAPELIB_VERSION_MINOR 6
#define SHAPELIB_VERSION_MICRO 0

#define SHAPELIB_MAKE_VERSION_NUMBER(major, minor, micro) \
    ((major) * 10000 + (minor) * 100 + (micro))

#define SHAPELIB_VERSION_NUMBER \
    SHAPELIB_MAKE_VERSION_NUMBER(SHAPELIB_VERSION_MAJOR, SHAPELIB_VERSION_MINOR, SHAPELIB_VERSION_MICRO)

#define SHAPELIB_AT_LEAST(major, minor, micro) \
    (SHAPELIB_VERSION_NUMBER >= SHAPELIB_MAKE_VERSION_NUMBER(major, minor, micro))

/************************************************************************/
/*                        Configuration options.                        */
/************************************************************************/

/* -------------------------------------------------------------------- */
/*      Should the DBFReadStringAttribute() strip leading and           */
/*      trailing white space?                                           */
/* -------------------------------------------------------------------- */
#define TRIM_DBF_WHITESPACE

/* -------------------------------------------------------------------- */
/*      Should we write measure values to the Multipatch object?        */
/*      Reportedly ArcView crashes if we do write it, so for now it     */
/*      is disabled.                                                    */
/* -------------------------------------------------------------------- */
#define DISABLE_MULTIPATCH_MEASURE




#ifndef FALSE
#define FALSE 0
#define TRUE 1
#endif

#define ByteCopy(a, b, c) memcpy(b, a, c)
#ifndef MAX
#define MIN(a, b) ((a < b) ? a : b)
#define MAX(a, b) ((a > b) ? a : b)
#endif

#ifndef USE_CPL
#if defined(_MSC_VER)
#if _MSC_VER < 1900
#define snprintf _snprintf
#endif
#elif defined(WIN32) || defined(_WIN32)
#ifndef snprintf
#define snprintf _snprintf
#endif
#endif
#endif

#ifndef bBigEndian
#if defined(CPL_LSB)
#define bBigEndian false
#elif defined(CPL_MSB)
#define bBigEndian true
#else
#ifndef static_var_bBigEndian_defined
#define static_var_bBigEndian_defined
static bool bBigEndian = false;
#endif
#endif
#endif

#ifdef __cplusplus
#define STATIC_CAST(type, x) static_cast<type>(x)
#define SHPLIB_NULLPTR nullptr
#else
#define STATIC_CAST(type, x) ((type)(x))
#define SHPLIB_NULLPTR NULL
#endif

#ifndef SwapWord_defined
#define SwapWord_defined
static void SwapWord(int length, void *wordP)
{
    for (int i = 0; i < length / 2; i++)
    {
        const unsigned char temp = STATIC_CAST(unsigned char *, wordP)[i];
        STATIC_CAST(unsigned char *, wordP)
        [i] = STATIC_CAST(unsigned char *, wordP)[length - i - 1];
        STATIC_CAST(unsigned char *, wordP)[length - i - 1] = temp;
    }
}
#endif
    /* -------------------------------------------------------------------- */
    /*      IO/Error hook functions.                                        */
    /* -------------------------------------------------------------------- */
    typedef int *SAFile;

#ifndef SAOffset
#if defined(_MSC_VER) && _MSC_VER >= 1400
    typedef unsigned __int64 SAOffset;
#else
    typedef unsigned long SAOffset;
#endif
#endif

    typedef struct
    {
        SAFile (*FOpen)(const char *filename, const char *access,
                        void *pvUserData);
        SAOffset (*FRead)(void *p, SAOffset size, SAOffset nmemb, SAFile file);
        SAOffset (*FWrite)(const void *p, SAOffset size, SAOffset nmemb,
                           SAFile file);
        SAOffset (*FSeek)(SAFile file, SAOffset offset, int whence);
        SAOffset (*FTell)(SAFile file);
        int (*FFlush)(SAFile file);
        int (*FClose)(SAFile file);
        int (*Remove)(const char *filename, void *pvUserData);

        void (*Error)(const char *message);
        double (*Atof)(const char *str);
        void *pvUserData;
    } SAHooks;


    /************************************************************************/
    /*                             SHP Support.                             */
    /************************************************************************/
    typedef struct tagSHPObject SHPObject;

    typedef struct
    {
        SAHooks sHooks;

        SAFile fpSHP;
        SAFile fpSHX;

        int nShapeType; /* SHPT_* */

        unsigned int nFileSize; /* SHP file */

        int nRecords;
        int nMaxRecords;
        unsigned int *panRecOffset;
        unsigned int *panRecSize;

        double adBoundsMin[4];
        double adBoundsMax[4];

        int bUpdated;

        unsigned char *pabyRec;
        int nBufSize;

        int bFastModeReadObject;
        unsigned char *pabyObjectBuf;
        int nObjectBufSize;
        SHPObject *psCachedObject;
    } SHPInfo;

    typedef SHPInfo *SHPHandle;

/* -------------------------------------------------------------------- */
/*      Shape types (nSHPType)                                          */
/* -------------------------------------------------------------------- */
#define SHPT_NULL 0
#define SHPT_POINT 1
#define SHPT_ARC 3
#define SHPT_POLYGON 5
#define SHPT_MULTIPOINT 8
#define SHPT_POINTZ 11
#define SHPT_ARCZ 13
#define SHPT_POLYGONZ 15
#define SHPT_MULTIPOINTZ 18
#define SHPT_POINTM 21
#define SHPT_ARCM 23
#define SHPT_POLYGONM 25
#define SHPT_MULTIPOINTM 28
#define SHPT_MULTIPATCH 31

    /* -------------------------------------------------------------------- */
    /*      Part types - everything but SHPT_MULTIPATCH just uses           */
    /*      SHPP_RING.                                                      */
    /* -------------------------------------------------------------------- */

#define SHPP_TRISTRIP 0
#define SHPP_TRIFAN 1
#define SHPP_OUTERRING 2
#define SHPP_INNERRING 3
#define SHPP_FIRSTRING 4
#define SHPP_RING 5

    /* -------------------------------------------------------------------- */
    /*      SHPObject - represents on shape (without attributes) read       */
    /*      from the .shp file.                                             */
    /* -------------------------------------------------------------------- */
    struct tagSHPObject
    {
        int nSHPType;

        int nShapeId; /* -1 is unknown/unassigned */

        int nParts;
        int *panPartStart;
        int *panPartType;

        int nVertices;
        double *padfX;
        double *padfY;
        double *padfZ;
        double *padfM;

        double dfXMin;
        double dfYMin;
        double dfZMin;
        double dfMMin;

        double dfXMax;
        double dfYMax;
        double dfZMax;
        double dfMMax;

        int bMeasureIsUsed;
        int bFastModeReadObject;
    };

/* this can be two or four for binary or quad tree */
#define MAX_SUBNODE 4

/* upper limit of tree levels for automatic estimation */
#define MAX_DEFAULT_TREE_DEPTH 12

    typedef struct shape_tree_node
    {
        /* region covered by this node */
        double adfBoundsMin[4];
        double adfBoundsMax[4];

        /* list of shapes stored at this node.  The papsShapeObj pointers
           or the whole list can be NULL */
        int nShapeCount;
        int *panShapeIds;
        SHPObject **papsShapeObj;

        int nSubNodes;
        struct shape_tree_node *apsSubNode[MAX_SUBNODE];

    } SHPTreeNode;

    typedef struct
    {
        SHPHandle hSHP;

        int nMaxDepth;
        int nDimension;
        int nTotalCount;

        SHPTreeNode *psRoot;
    } SHPTree;

    typedef struct SBNSearchInfo *SBNSearchHandle;

    typedef struct
    {
        SAHooks sHooks;

        SAFile fp;

        int nRecords;

        int nRecordLength; /* Must fit on uint16 */
        int nHeaderLength; /* File header length (32) + field
                              descriptor length + spare space.
                              Must fit on uint16 */
        int nFields;
        int *panFieldOffset;
        int *panFieldSize;
        int *panFieldDecimals;
        char *pachFieldType;

        char *pszHeader; /* Field descriptors */

        int nCurrentRecord;
        int bCurrentRecordModified;
        char *pszCurrentRecord;

        int nWorkFieldLength;
        char *pszWorkField;

        int bNoHeader;
        int bUpdated;

        union
        {
            double dfDoubleField;
            int nIntField;
        } fieldValue;

        int iLanguageDriver;
        char *pszCodePage;

        int nUpdateYearSince1900; /* 0-255 */
        int nUpdateMonth;         /* 1-12 */
        int nUpdateDay;           /* 1-31 */

        int bWriteEndOfFileChar; /* defaults to TRUE */

        int bRequireNextWriteSeek;
    } DBFInfo;

    typedef DBFInfo *DBFHandle;

    typedef enum
    {
        FTString,
        FTInteger,
        FTDouble,
        FTLogical,
        FTDate,
        FTInvalid
    } DBFFieldType;
    typedef struct
    {
        int year;
        int month;
        int day;
    } SHPDate;
/* Field descriptor/header size */
#define XBASE_FLDHDR_SZ 32
/* Shapelib read up to 11 characters, even if only 10 should normally be used */
#define XBASE_FLDNAME_LEN_READ 11
/* On writing, we limit to 10 characters */
#define XBASE_FLDNAME_LEN_WRITE 10
/* Normally only 254 characters should be used. We tolerate 255 historically */
#define XBASE_FLD_MAX_WIDTH 255

#define HEADER_RECORD_TERMINATOR 0x0D
#define XBASE_FILEHDR_SZ 32
#define END_OF_FILE_CHARACTER 0x1A
#define REINTERPRET_CAST(type, x) ((type)(x))
#define REINTERPRET_CAST(type, x) reinterpret_cast<type>(x)
#define CONST_CAST(type, x) const_cast<type>(x)
#define CPLsnprintf snprintf
#ifdef USE_CPL
    CPL_INLINE static void CPL_IGNORE_RET_VAL_INT(CPL_UNUSED int unused)
    {
    }
#else
#define CPL_IGNORE_RET_VAL_INT(x) x
#endif
#if defined(_MSC_VER)
#define STRCASECMP(a, b) (_stricmp(a, b))
#elif defined(_WIN32)
#define STRCASECMP(a, b) (stricmp(a, b))
#else
#include <strings.h>
#define STRCASECMP(a, b) (strcasecmp(a, b))
#endif
//Prototypes
    void  SHPWriteHeader(SHPHandle psSHP);
    SHPHandle  SHPOpen(const char* pszLayer, const char* pszAccess);
    int SHPGetLenWithoutExtension(const char* pszBasename);
    SHPHandle  SHPOpenLL(const char* pszLayer, const char* pszAccess, const SAHooks* psHooks);
    SHPHandle  SHPOpenLLEx(const char* pszLayer, const char* pszAccess, const SAHooks* psHooks, int bRestoreSHX);
    int  SHPRestoreSHX(const char* pszLayer, const char* pszAccess, const SAHooks* psHooks);
    void  SHPClose(SHPHandle psSHP);
    void  SHPSetFastModeReadObject(SHPHandle hSHP, int bFastMode);
    SAFile SADFOpen(const char* pszFilename, const char* pszAccess, void* pvUserData);
    void  SHPGetInfo(SHPHandle psSHP, int* pnEntities, int* pnShapeType, double* padfMinBound, double* padfMaxBound);
    SAOffset SADFRead(void* p, SAOffset size, SAOffset nmemb, SAFile file);
    SAOffset SADFWrite(const void* p, SAOffset size, SAOffset nmemb, SAFile file);
    SAOffset SADFSeek(SAFile file, SAOffset offset, int whence);
    SAOffset SADFTell(SAFile file);
    int SADFFlush(SAFile file);
    int SADFClose(SAFile file);
    int SADRemove(const char* filename, void* pvUserData);
    void SADError(const char* message);
    void SASetupDefaultHooks(SAHooks* psHooks);
    SHPHandle  SHPCreate(const char* pszLayer, int nShapeType);
    SHPHandle  SHPCreateLL(const char* pszLayer, int nShapeType, const SAHooks* psHooks);
    void _SHPSetBounds(unsigned char* pabyRec, const SHPObject* psShape);
    void  SHPComputeExtents(SHPObject* psObject);
    SHPObject *SHPCreateObject(int nSHPType, int nShapeId, int nParts,const int* panPartStart, const int* panPartType,int nVertices, const double* padfX, const double* padfY,const double* padfZ, const double* padfM);
    SHPObject*
        SHPCreateSimpleObject(int nSHPType, int nVertices, const double* padfX,
            const double* padfY, const double* padfZ);
    int  SHPWriteObject(SHPHandle psSHP, int nShapeId,
        SHPObject* psObject);
    void* SHPAllocBuffer(unsigned char** pBuffer, int nSize);
    unsigned char* SHPReallocObjectBufIfNecessary(SHPHandle psSHP,
        int nObjectBufSize);
    SHPObject* SHPReadObject(SHPHandle psSHP, int hEntity);
    const char* SHPTypeName(int nSHPType);
    const char* SHPPartTypeName(int nPartType);
    void  SHPDestroyObject(SHPObject* psShape);
    int SHPGetPartVertexCount(const SHPObject* psObject, int iPart);
    int SHPRewindIsInnerRing(const SHPObject* psObject, int iOpRing,
        double dfTestX, double dfTestY,
        double dfRelativeTolerance, int bSameZ,
        double dfTestZ);
    void DBFWriteHeader(DBFHandle psDBF);
    bool DBFFlushRecord(DBFHandle psDBF);
    bool DBFLoadRecord(DBFHandle psDBF, int iRecord);
    void  DBFUpdateHeader(DBFHandle psDBF);
    void  DBFSetLastModifiedDate(DBFHandle psDBF, int nYYSince1900,
        int nMM, int nDD);
    DBFHandle  DBFOpen(const char* pszFilename, const char* pszAccess);
    int DBFGetLenWithoutExtension(const char* pszBasename);
    DBFHandle  DBFOpenLL(const char* pszFilename, const char* pszAccess,
        const SAHooks* psHooks);
    void  DBFClose(DBFHandle psDBF);
    DBFHandle  DBFCreate(const char* pszFilename);
    DBFHandle  DBFCreateEx(const char* pszFilename,
        const char* pszCodePage);
    DBFHandle  DBFCreateLL(const char* pszFilename,
        const char* pszCodePage,
        const SAHooks* psHooks);
    int  DBFAddField(DBFHandle psDBF, const char* pszFieldName,
        DBFFieldType eType, int nWidth, int nDecimals);
    char DBFGetNullCharacter(char chType);
    int  DBFAddNativeFieldType(DBFHandle psDBF, const char* pszFieldName,
        char chType, int nWidth, int nDecimals);
    void* DBFReadAttribute(DBFHandle psDBF, int hEntity, int iField,
        char chReqType);
    int  DBFReadIntegerAttribute(DBFHandle psDBF, int iRecord,
        int iField);
    double  DBFReadDoubleAttribute(DBFHandle psDBF, int iRecord,
        int iField);
    const char*
        DBFReadStringAttribute(DBFHandle psDBF, int iRecord, int iField);
    const char*
        DBFReadLogicalAttribute(DBFHandle psDBF, int iRecord, int iField);
    bool DBFIsValueNULL(char chType, const char* pszValue);
    int  DBFIsAttributeNULL(DBFHandle psDBF, int iRecord, int iField);
    int  DBFGetFieldCount(DBFHandle psDBF);
    int  DBFGetRecordCount(DBFHandle psDBF);
    DBFFieldType  DBFGetFieldInfo(const DBFHandle psDBF, int iField,
        char* pszFieldName, int* pnWidth,
        int* pnDecimals);
    bool DBFWriteAttribute(DBFHandle psDBF, int hEntity, int iField,
        void* pValue);
    int  DBFWriteAttributeDirectly(DBFHandle psDBF, int hEntity,
        int iField, const void* pValue);
    int  DBFWriteDoubleAttribute(DBFHandle psDBF, int iRecord,
        int iField, double dValue);
    int  DBFWriteIntegerAttribute(DBFHandle psDBF, int iRecord,
        int iField, int nValue);
    int  DBFWriteStringAttribute(DBFHandle psDBF, int iRecord,
        int iField, const char* pszValue);
    int  DBFWriteNULLAttribute(DBFHandle psDBF, int iRecord, int iField);
    int  DBFWriteLogicalAttribute(DBFHandle psDBF, int iRecord,
        int iField, const char lValue);
    int  DBFWriteDateAttribute(DBFHandle psDBF, int iRecord, int iField,
        const SHPDate* lValue);
    int  DBFWriteTuple(DBFHandle psDBF, int hEntity,
        const void* pRawTuple);
    const char* DBFReadTuple(DBFHandle psDBF, int hEntity);
    DBFHandle  DBFCloneEmpty(const DBFHandle psDBF,
        const char* pszFilename);
    char  DBFGetNativeFieldType(const DBFHandle psDBF, int iField);
    int  DBFGetFieldIndex(const DBFHandle psDBF,
        const char* pszFieldName);
    int  DBFIsRecordDeleted(const DBFHandle psDBF, int iShape);
    int  DBFMarkRecordDeleted(DBFHandle psDBF, int iShape,
        int bIsDeleted);
    const char* DBFGetCodePage(const DBFHandle psDBF);
    int  DBFDeleteField(DBFHandle psDBF, int iField);
    int  DBFReorderFields(DBFHandle psDBF, const int* panMap);
    void  DBFSetWriteEndOfFileChar(DBFHandle psDBF, int bWriteFlag);
    int  DBFAlterFieldDefn(DBFHandle psDBF, int iField,
        const char* pszFieldName, char chType,
        int nWidth, int nDecimals);
    //Functions




void  SHPWriteHeader(SHPHandle psSHP)
{
    if (psSHP->fpSHX == SHPLIB_NULLPTR)
    {
        psSHP->sHooks.Error("SHPWriteHeader failed : SHX file is closed");
        return;
    }

    /* -------------------------------------------------------------------- */
    /*      Prepare header block for .shp file.                             */
    /* -------------------------------------------------------------------- */

    unsigned char abyHeader[100] = {0};
    abyHeader[2] = 0x27; /* magic cookie */
    abyHeader[3] = 0x0a;

    uint32_t i32 = psSHP->nFileSize / 2; /* file size */
    ByteCopy(&i32, abyHeader + 24, 4);
    if (!bBigEndian)
        SwapWord(4, abyHeader + 24);

    i32 = 1000; /* version */
    ByteCopy(&i32, abyHeader + 28, 4);
    if (bBigEndian)
        SwapWord(4, abyHeader + 28);

    i32 = psSHP->nShapeType; /* shape type */
    ByteCopy(&i32, abyHeader + 32, 4);
    if (bBigEndian)
        SwapWord(4, abyHeader + 32);

    double dValue = psSHP->adBoundsMin[0]; /* set bounds */
    ByteCopy(&dValue, abyHeader + 36, 8);
    if (bBigEndian)
        SwapWord(8, abyHeader + 36);

    dValue = psSHP->adBoundsMin[1];
    ByteCopy(&dValue, abyHeader + 44, 8);
    if (bBigEndian)
        SwapWord(8, abyHeader + 44);

    dValue = psSHP->adBoundsMax[0];
    ByteCopy(&dValue, abyHeader + 52, 8);
    if (bBigEndian)
        SwapWord(8, abyHeader + 52);

    dValue = psSHP->adBoundsMax[1];
    ByteCopy(&dValue, abyHeader + 60, 8);
    if (bBigEndian)
        SwapWord(8, abyHeader + 60);

    dValue = psSHP->adBoundsMin[2]; /* z */
    ByteCopy(&dValue, abyHeader + 68, 8);
    if (bBigEndian)
        SwapWord(8, abyHeader + 68);

    dValue = psSHP->adBoundsMax[2];
    ByteCopy(&dValue, abyHeader + 76, 8);
    if (bBigEndian)
        SwapWord(8, abyHeader + 76);

    dValue = psSHP->adBoundsMin[3]; /* m */
    ByteCopy(&dValue, abyHeader + 84, 8);
    if (bBigEndian)
        SwapWord(8, abyHeader + 84);

    dValue = psSHP->adBoundsMax[3];
    ByteCopy(&dValue, abyHeader + 92, 8);
    if (bBigEndian)
        SwapWord(8, abyHeader + 92);

    /* -------------------------------------------------------------------- */
    /*      Write .shp file header.                                         */
    /* -------------------------------------------------------------------- */
    if (psSHP->sHooks.FSeek(psSHP->fpSHP, 0, 0) != 0 ||
        psSHP->sHooks.FWrite(abyHeader, 100, 1, psSHP->fpSHP) != 1)
    {
        char szErrorMsg[200];

        snprintf(szErrorMsg, sizeof(szErrorMsg),
                 "Failure writing .shp header: %s", strerror(errno));
        szErrorMsg[sizeof(szErrorMsg) - 1] = '\0';
        psSHP->sHooks.Error(szErrorMsg);
        return;
    }

    /* -------------------------------------------------------------------- */
    /*      Prepare, and write .shx file header.                            */
    /* -------------------------------------------------------------------- */
    i32 = (psSHP->nRecords * 2 * sizeof(uint32_t) + 100) / 2; /* file size */
    ByteCopy(&i32, abyHeader + 24, 4);
    if (!bBigEndian)
        SwapWord(4, abyHeader + 24);

    if (psSHP->sHooks.FSeek(psSHP->fpSHX, 0, 0) != 0 ||
        psSHP->sHooks.FWrite(abyHeader, 100, 1, psSHP->fpSHX) != 1)
    {
        char szErrorMsg[200];

        snprintf(szErrorMsg, sizeof(szErrorMsg),
                 "Failure writing .shx header: %s", strerror(errno));
        szErrorMsg[sizeof(szErrorMsg) - 1] = '\0';
        psSHP->sHooks.Error(szErrorMsg);

        return;
    }

    /* -------------------------------------------------------------------- */
    /*      Write out the .shx contents.                                    */
    /* -------------------------------------------------------------------- */
    uint32_t *panSHX =
        STATIC_CAST(uint32_t *, malloc(sizeof(uint32_t) * 2 * psSHP->nRecords));
    if (panSHX == SHPLIB_NULLPTR)
    {
        psSHP->sHooks.Error("Failure allocatin panSHX");
        return;
    }

    for (int i = 0; i < psSHP->nRecords; i++)
    {
        panSHX[i * 2] = psSHP->panRecOffset[i] / 2;
        panSHX[i * 2 + 1] = psSHP->panRecSize[i] / 2;
        if (!bBigEndian)
            SwapWord(4, panSHX + i * 2);
        if (!bBigEndian)
            SwapWord(4, panSHX + i * 2 + 1);
    }

    if (STATIC_CAST(int, psSHP->sHooks.FWrite(panSHX, sizeof(uint32_t) * 2,
                                              psSHP->nRecords, psSHP->fpSHX)) !=
        psSHP->nRecords)
    {
        char szErrorMsg[200];

        snprintf(szErrorMsg, sizeof(szErrorMsg),
                 "Failure writing .shx contents: %s", strerror(errno));
        szErrorMsg[sizeof(szErrorMsg) - 1] = '\0';
        psSHP->sHooks.Error(szErrorMsg);
    }

    free(panSHX);

    /* -------------------------------------------------------------------- */
    /*      Flush to disk.                                                  */
    /* -------------------------------------------------------------------- */
    psSHP->sHooks.FFlush(psSHP->fpSHP);
    psSHP->sHooks.FFlush(psSHP->fpSHX);
}

/************************************************************************/
/*                              SHPOpen()                               */
/************************************************************************/

SHPHandle  SHPOpenSHPOpen(const char *pszLayer, const char *pszAccess)
{
    SAHooks sHooks;

    SASetupDefaultHooks(&sHooks);

    return SHPOpenLL(pszLayer, pszAccess, &sHooks);
}

/************************************************************************/
/*                      SHPGetLenWithoutExtension()                     */
/************************************************************************/

int SHPGetLenWithoutExtension(const char *pszBasename)
{
    const int nLen = STATIC_CAST(int, strlen(pszBasename));
    for (int i = nLen - 1;
         i > 0 && pszBasename[i] != '/' && pszBasename[i] != '\\'; i--)
    {
        if (pszBasename[i] == '.')
        {
            return i;
        }
    }
    return nLen;
}

/************************************************************************/
/*                              SHPOpen()                               */
/*                                                                      */
/*      Open the .shp and .shx files based on the basename of the       */
/*      files or either file name.                                      */
/************************************************************************/

SHPHandle  SHPOpenLL(const char *pszLayer, const char *pszAccess,const SAHooks *psHooks)
{
    /* -------------------------------------------------------------------- */
    /*      Ensure the access string is one of the legal ones.  We          */
    /*      ensure the result string indicates binary to avoid common       */
    /*      problems on Windows.                                            */
    /* -------------------------------------------------------------------- */
    bool bLazySHXLoading = false;
    if (strcmp(pszAccess, "rb+") == 0 || strcmp(pszAccess, "r+b") == 0 ||
        strcmp(pszAccess, "r+") == 0)
    {
        pszAccess = "r+b";
    }
    else
    {
        bLazySHXLoading = strchr(pszAccess, 'l') != SHPLIB_NULLPTR;
        pszAccess = "rb";
    }

/* -------------------------------------------------------------------- */
/*  Establish the byte order on this machine.           */
/* -------------------------------------------------------------------- */
#if !defined(bBigEndian)
    {
        int i = 1;
        if (*((unsigned char *)&i) == 1)
            bBigEndian = false;
        else
            bBigEndian = true;
    }
#endif

    /* -------------------------------------------------------------------- */
    /*  Initialize the info structure.                  */
    /* -------------------------------------------------------------------- */
    SHPHandle psSHP = STATIC_CAST(SHPHandle, calloc(sizeof(SHPInfo), 1));

    psSHP->bUpdated = FALSE;
    memcpy(&(psSHP->sHooks), psHooks, sizeof(SAHooks));

    /* -------------------------------------------------------------------- */
    /*  Open the .shp and .shx files.  Note that files pulled from  */
    /*  a PC to Unix with upper case filenames won't work!      */
    /* -------------------------------------------------------------------- */
    const int nLenWithoutExtension = SHPGetLenWithoutExtension(pszLayer);
    char *pszFullname = STATIC_CAST(char *, malloc(nLenWithoutExtension + 5));
    memcpy(pszFullname, pszLayer, nLenWithoutExtension);
    memcpy(pszFullname + nLenWithoutExtension, ".shp", 5);
    psSHP->fpSHP =
        psSHP->sHooks.FOpen(pszFullname, pszAccess, psSHP->sHooks.pvUserData);
    if (psSHP->fpSHP == SHPLIB_NULLPTR)
    {
        memcpy(pszFullname + nLenWithoutExtension, ".SHP", 5);
        psSHP->fpSHP = psSHP->sHooks.FOpen(pszFullname, pszAccess,
                                           psSHP->sHooks.pvUserData);
    }

    if (psSHP->fpSHP == SHPLIB_NULLPTR)
    {
        const size_t nMessageLen = strlen(pszFullname) * 2 + 256;
        char *pszMessage = STATIC_CAST(char *, malloc(nMessageLen));
        pszFullname[nLenWithoutExtension] = 0;
        snprintf(pszMessage, nMessageLen,
                 "Unable to open %s.shp or %s.SHP in %s mode.", pszFullname,
                 pszFullname, pszAccess);
        psHooks->Error(pszMessage);
        free(pszMessage);

        free(psSHP);
        free(pszFullname);

        return SHPLIB_NULLPTR;
    }

    memcpy(pszFullname + nLenWithoutExtension, ".shx", 5);
    psSHP->fpSHX =
        psSHP->sHooks.FOpen(pszFullname, pszAccess, psSHP->sHooks.pvUserData);
    if (psSHP->fpSHX == SHPLIB_NULLPTR)
    {
        memcpy(pszFullname + nLenWithoutExtension, ".SHX", 5);
        psSHP->fpSHX = psSHP->sHooks.FOpen(pszFullname, pszAccess,
                                           psSHP->sHooks.pvUserData);
    }

    if (psSHP->fpSHX == SHPLIB_NULLPTR)
    {
        const size_t nMessageLen = strlen(pszFullname) * 2 + 256;
        char *pszMessage = STATIC_CAST(char *, malloc(nMessageLen));
        pszFullname[nLenWithoutExtension] = 0;
        snprintf(pszMessage, nMessageLen,
                 "Unable to open %s.shx or %s.SHX. "
                 "Set SHAPE_RESTORE_SHX config option to YES to restore or "
                 "create it.",
                 pszFullname, pszFullname);
        psHooks->Error(pszMessage);
        free(pszMessage);

        psSHP->sHooks.FClose(psSHP->fpSHP);
        free(psSHP);
        free(pszFullname);
        return SHPLIB_NULLPTR;
    }

    free(pszFullname);

    /* -------------------------------------------------------------------- */
    /*  Read the file size from the SHP file.               */
    /* -------------------------------------------------------------------- */
    unsigned char *pabyBuf = STATIC_CAST(unsigned char *, malloc(100));
    if (psSHP->sHooks.FRead(pabyBuf, 100, 1, psSHP->fpSHP) != 1)
    {
        psSHP->sHooks.Error(".shp file is unreadable, or corrupt.");
        psSHP->sHooks.FClose(psSHP->fpSHP);
        psSHP->sHooks.FClose(psSHP->fpSHX);
        free(pabyBuf);
        free(psSHP);

        return SHPLIB_NULLPTR;
    }

    psSHP->nFileSize = (STATIC_CAST(unsigned int, pabyBuf[24]) << 24) |
                       (pabyBuf[25] << 16) | (pabyBuf[26] << 8) | pabyBuf[27];
    if (psSHP->nFileSize < UINT_MAX / 2)
        psSHP->nFileSize *= 2;
    else
        psSHP->nFileSize = (UINT_MAX / 2) * 2;

    /* -------------------------------------------------------------------- */
    /*  Read SHX file Header info                                           */
    /* -------------------------------------------------------------------- */
    if (psSHP->sHooks.FRead(pabyBuf, 100, 1, psSHP->fpSHX) != 1 ||
        pabyBuf[0] != 0 || pabyBuf[1] != 0 || pabyBuf[2] != 0x27 ||
        (pabyBuf[3] != 0x0a && pabyBuf[3] != 0x0d))
    {
        psSHP->sHooks.Error(".shx file is unreadable, or corrupt.");
        psSHP->sHooks.FClose(psSHP->fpSHP);
        psSHP->sHooks.FClose(psSHP->fpSHX);
        free(pabyBuf);
        free(psSHP);

        return SHPLIB_NULLPTR;
    }

    psSHP->nRecords = pabyBuf[27] | (pabyBuf[26] << 8) | (pabyBuf[25] << 16) |
                      ((pabyBuf[24] & 0x7F) << 24);
    psSHP->nRecords = (psSHP->nRecords - 50) / 4;

    psSHP->nShapeType = pabyBuf[32];

    if (psSHP->nRecords < 0 || psSHP->nRecords > 256000000)
    {
        char szErrorMsg[200];

        snprintf(szErrorMsg, sizeof(szErrorMsg),
                 "Record count in .shx header is %d, which seems\n"
                 "unreasonable.  Assuming header is corrupt.",
                 psSHP->nRecords);
        szErrorMsg[sizeof(szErrorMsg) - 1] = '\0';
        psSHP->sHooks.Error(szErrorMsg);
        psSHP->sHooks.FClose(psSHP->fpSHP);
        psSHP->sHooks.FClose(psSHP->fpSHX);
        free(psSHP);
        free(pabyBuf);

        return SHPLIB_NULLPTR;
    }

    /* If a lot of records are advertized, check that the file is big enough */
    /* to hold them */
    if (psSHP->nRecords >= 1024 * 1024)
    {
        psSHP->sHooks.FSeek(psSHP->fpSHX, 0, 2);
        const SAOffset nFileSize = psSHP->sHooks.FTell(psSHP->fpSHX);
        if (nFileSize > 100 &&
            nFileSize / 2 < STATIC_CAST(SAOffset, psSHP->nRecords * 4 + 50))
        {
            psSHP->nRecords = STATIC_CAST(int, (nFileSize - 100) / 8);
        }
        psSHP->sHooks.FSeek(psSHP->fpSHX, 100, 0);
    }

    /* -------------------------------------------------------------------- */
    /*      Read the bounds.                                                */
    /* -------------------------------------------------------------------- */
    double dValue;

    if (bBigEndian)
        SwapWord(8, pabyBuf + 36);
    memcpy(&dValue, pabyBuf + 36, 8);
    psSHP->adBoundsMin[0] = dValue;

    if (bBigEndian)
        SwapWord(8, pabyBuf + 44);
    memcpy(&dValue, pabyBuf + 44, 8);
    psSHP->adBoundsMin[1] = dValue;

    if (bBigEndian)
        SwapWord(8, pabyBuf + 52);
    memcpy(&dValue, pabyBuf + 52, 8);
    psSHP->adBoundsMax[0] = dValue;

    if (bBigEndian)
        SwapWord(8, pabyBuf + 60);
    memcpy(&dValue, pabyBuf + 60, 8);
    psSHP->adBoundsMax[1] = dValue;

    if (bBigEndian)
        SwapWord(8, pabyBuf + 68); /* z */
    memcpy(&dValue, pabyBuf + 68, 8);
    psSHP->adBoundsMin[2] = dValue;

    if (bBigEndian)
        SwapWord(8, pabyBuf + 76);
    memcpy(&dValue, pabyBuf + 76, 8);
    psSHP->adBoundsMax[2] = dValue;

    if (bBigEndian)
        SwapWord(8, pabyBuf + 84); /* z */
    memcpy(&dValue, pabyBuf + 84, 8);
    psSHP->adBoundsMin[3] = dValue;

    if (bBigEndian)
        SwapWord(8, pabyBuf + 92);
    memcpy(&dValue, pabyBuf + 92, 8);
    psSHP->adBoundsMax[3] = dValue;

    free(pabyBuf);

    /* -------------------------------------------------------------------- */
    /*  Read the .shx file to get the offsets to each record in     */
    /*  the .shp file.                          */
    /* -------------------------------------------------------------------- */
    psSHP->nMaxRecords = psSHP->nRecords;

    psSHP->panRecOffset =
        STATIC_CAST(unsigned int *,
                    malloc(sizeof(unsigned int) * MAX(1, psSHP->nMaxRecords)));
    psSHP->panRecSize =
        STATIC_CAST(unsigned int *,
                    malloc(sizeof(unsigned int) * MAX(1, psSHP->nMaxRecords)));
    if (bLazySHXLoading)
        pabyBuf = SHPLIB_NULLPTR;
    else
        pabyBuf =
            STATIC_CAST(unsigned char *, malloc(8 * MAX(1, psSHP->nRecords)));

    if (psSHP->panRecOffset == SHPLIB_NULLPTR ||
        psSHP->panRecSize == SHPLIB_NULLPTR ||
        (!bLazySHXLoading && pabyBuf == SHPLIB_NULLPTR))
    {
        char szErrorMsg[200];

        snprintf(
            szErrorMsg, sizeof(szErrorMsg),
            "Not enough memory to allocate requested memory (nRecords=%d).\n"
            "Probably broken SHP file",
            psSHP->nRecords);
        szErrorMsg[sizeof(szErrorMsg) - 1] = '\0';
        psSHP->sHooks.Error(szErrorMsg);
        psSHP->sHooks.FClose(psSHP->fpSHP);
        psSHP->sHooks.FClose(psSHP->fpSHX);
        if (psSHP->panRecOffset)
            free(psSHP->panRecOffset);
        if (psSHP->panRecSize)
            free(psSHP->panRecSize);
        if (pabyBuf)
            free(pabyBuf);
        free(psSHP);
        return SHPLIB_NULLPTR;
    }

    if (bLazySHXLoading)
    {
        memset(psSHP->panRecOffset, 0,
               sizeof(unsigned int) * MAX(1, psSHP->nMaxRecords));
        memset(psSHP->panRecSize, 0,
               sizeof(unsigned int) * MAX(1, psSHP->nMaxRecords));
        free(pabyBuf);  // sometimes make cppcheck happy, but
        return (psSHP);
    }

    if (STATIC_CAST(int, psSHP->sHooks.FRead(pabyBuf, 8, psSHP->nRecords,
                                             psSHP->fpSHX)) != psSHP->nRecords)
    {
        char szErrorMsg[200];

        snprintf(szErrorMsg, sizeof(szErrorMsg),
                 "Failed to read all values for %d records in .shx file: %s.",
                 psSHP->nRecords, strerror(errno));
        szErrorMsg[sizeof(szErrorMsg) - 1] = '\0';
        psSHP->sHooks.Error(szErrorMsg);

        /* SHX is short or unreadable for some reason. */
        psSHP->sHooks.FClose(psSHP->fpSHP);
        psSHP->sHooks.FClose(psSHP->fpSHX);
        free(psSHP->panRecOffset);
        free(psSHP->panRecSize);
        free(pabyBuf);
        free(psSHP);

        return SHPLIB_NULLPTR;
    }

    /* In read-only mode, we can close the SHX now */
    if (strcmp(pszAccess, "rb") == 0)
    {
        psSHP->sHooks.FClose(psSHP->fpSHX);
        psSHP->fpSHX = SHPLIB_NULLPTR;
    }

    for (int i = 0; i < psSHP->nRecords; i++)
    {
        unsigned int nOffset;
        memcpy(&nOffset, pabyBuf + i * 8, 4);
        if (!bBigEndian)
            SwapWord(4, &nOffset);

        unsigned int nLength;
        memcpy(&nLength, pabyBuf + i * 8 + 4, 4);
        if (!bBigEndian)
            SwapWord(4, &nLength);

        if (nOffset > STATIC_CAST(unsigned int, INT_MAX))
        {
            char str[128];
            snprintf(str, sizeof(str), "Invalid offset for entity %d", i);
            str[sizeof(str) - 1] = '\0';

            psSHP->sHooks.Error(str);
            SHPClose(psSHP);
            free(pabyBuf);
            return SHPLIB_NULLPTR;
        }
        if (nLength > STATIC_CAST(unsigned int, INT_MAX / 2 - 4))
        {
            char str[128];
            snprintf(str, sizeof(str), "Invalid length for entity %d", i);
            str[sizeof(str) - 1] = '\0';

            psSHP->sHooks.Error(str);
            SHPClose(psSHP);
            free(pabyBuf);
            return SHPLIB_NULLPTR;
        }
        psSHP->panRecOffset[i] = nOffset * 2;
        psSHP->panRecSize[i] = nLength * 2;
    }
    free(pabyBuf);

    return (psSHP);
}

/************************************************************************/
/*                              SHPOpenLLEx()                           */
/*                                                                      */
/*      Open the .shp and .shx files based on the basename of the       */
/*      files or either file name. It generally invokes SHPRestoreSHX() */
/*      in case when bRestoreSHX equals true.                           */
/************************************************************************/

SHPHandle  SHPOpenLLEx(const char *pszLayer, const char *pszAccess,const SAHooks *psHooks, int bRestoreSHX)
{
    if (!bRestoreSHX)
        return SHPOpenLL(pszLayer, pszAccess, psHooks);
    else
    {
        if (SHPRestoreSHX(pszLayer, pszAccess, psHooks))
        {
            return SHPOpenLL(pszLayer, pszAccess, psHooks);
        }
    }

    return SHPLIB_NULLPTR;
}

/************************************************************************/
/*                              SHPRestoreSHX()                         */
/*                                                                      */
/*      Restore .SHX file using associated .SHP file.                   */
/*                                                                      */
/************************************************************************/

int  SHPRestoreSHX(const char *pszLayer, const char *pszAccess,const SAHooks *psHooks)
{
    /* -------------------------------------------------------------------- */
    /*      Ensure the access string is one of the legal ones.  We          */
    /*      ensure the result string indicates binary to avoid common       */
    /*      problems on Windows.                                            */
    /* -------------------------------------------------------------------- */
    if (strcmp(pszAccess, "rb+") == 0 || strcmp(pszAccess, "r+b") == 0 ||
        strcmp(pszAccess, "r+") == 0)
    {
        pszAccess = "r+b";
    }
    else
    {
        pszAccess = "rb";
    }

/* -------------------------------------------------------------------- */
/*  Establish the byte order on this machine.                           */
/* -------------------------------------------------------------------- */
#if !defined(bBigEndian)
    {
        int i = 1;
        if (*((unsigned char *)&i) == 1)
            bBigEndian = false;
        else
            bBigEndian = true;
    }
#endif

    /* -------------------------------------------------------------------- */
    /*  Open the .shp file.  Note that files pulled from                    */
    /*  a PC to Unix with upper case filenames won't work!                  */
    /* -------------------------------------------------------------------- */
    const int nLenWithoutExtension = SHPGetLenWithoutExtension(pszLayer);
    char *pszFullname = STATIC_CAST(char *, malloc(nLenWithoutExtension + 5));
    memcpy(pszFullname, pszLayer, nLenWithoutExtension);
    memcpy(pszFullname + nLenWithoutExtension, ".shp", 5);
    SAFile fpSHP = psHooks->FOpen(pszFullname, pszAccess, psHooks->pvUserData);
    if (fpSHP == SHPLIB_NULLPTR)
    {
        memcpy(pszFullname + nLenWithoutExtension, ".SHP", 5);
        fpSHP = psHooks->FOpen(pszFullname, pszAccess, psHooks->pvUserData);
    }

    if (fpSHP == SHPLIB_NULLPTR)
    {
        const size_t nMessageLen = strlen(pszFullname) * 2 + 256;
        char *pszMessage = STATIC_CAST(char *, malloc(nMessageLen));

        pszFullname[nLenWithoutExtension] = 0;
        snprintf(pszMessage, nMessageLen, "Unable to open %s.shp or %s.SHP.",
                 pszFullname, pszFullname);
        psHooks->Error(pszMessage);
        free(pszMessage);

        free(pszFullname);

        return (0);
    }

    /* -------------------------------------------------------------------- */
    /*  Read the file size from the SHP file.                               */
    /* -------------------------------------------------------------------- */
    unsigned char *pabyBuf = STATIC_CAST(unsigned char *, malloc(100));
    if (psHooks->FRead(pabyBuf, 100, 1, fpSHP) != 1)
    {
        psHooks->Error(".shp file is unreadable, or corrupt.");
        psHooks->FClose(fpSHP);

        free(pabyBuf);
        free(pszFullname);

        return (0);
    }

    unsigned int nSHPFilesize = (STATIC_CAST(unsigned int, pabyBuf[24]) << 24) |
                                (pabyBuf[25] << 16) | (pabyBuf[26] << 8) |
                                pabyBuf[27];
    if (nSHPFilesize < UINT_MAX / 2)
        nSHPFilesize *= 2;
    else
        nSHPFilesize = (UINT_MAX / 2) * 2;

    memcpy(pszFullname + nLenWithoutExtension, ".shx", 5);
    const char pszSHXAccess[] = "w+b";
    SAFile fpSHX =
        psHooks->FOpen(pszFullname, pszSHXAccess, psHooks->pvUserData);
    if (fpSHX == SHPLIB_NULLPTR)
    {
        size_t nMessageLen = strlen(pszFullname) * 2 + 256;
        char *pszMessage = STATIC_CAST(char *, malloc(nMessageLen));
        pszFullname[nLenWithoutExtension] = 0;
        snprintf(pszMessage, nMessageLen,
                 "Error opening file %s.shx for writing", pszFullname);
        psHooks->Error(pszMessage);
        free(pszMessage);

        psHooks->FClose(fpSHP);

        free(pabyBuf);
        free(pszFullname);

        return (0);
    }

    /* -------------------------------------------------------------------- */
    /*  Open SHX and create it using SHP file content.                      */
    /* -------------------------------------------------------------------- */
    psHooks->FSeek(fpSHP, 100, 0);
    char *pabySHXHeader = STATIC_CAST(char *, malloc(100));
    memcpy(pabySHXHeader, pabyBuf, 100);
    psHooks->FWrite(pabySHXHeader, 100, 1, fpSHX);
    free(pabyBuf);

    // unsigned int nCurrentRecordOffset = 0;
    unsigned int nCurrentSHPOffset = 100;
    unsigned int nRealSHXContentSize = 100;
    int nRetCode = TRUE;
    unsigned int nRecordOffset = 50;

    while (nCurrentSHPOffset < nSHPFilesize)
    {
        unsigned int niRecord = 0;
        unsigned int nRecordLength = 0;
        int nSHPType;

        if (psHooks->FRead(&niRecord, 4, 1, fpSHP) == 1 &&
            psHooks->FRead(&nRecordLength, 4, 1, fpSHP) == 1 &&
            psHooks->FRead(&nSHPType, 4, 1, fpSHP) == 1)
        {
            char abyReadRecord[8];
            unsigned int nRecordOffsetBE = nRecordOffset;

            if (!bBigEndian)
                SwapWord(4, &nRecordOffsetBE);
            memcpy(abyReadRecord, &nRecordOffsetBE, 4);
            memcpy(abyReadRecord + 4, &nRecordLength, 4);

            if (!bBigEndian)
                SwapWord(4, &nRecordLength);

            if (bBigEndian)
                SwapWord(4, &nSHPType);

            // Sanity check on record length
            if (nRecordLength < 1 ||
                nRecordLength > (nSHPFilesize - (nCurrentSHPOffset + 8)) / 2)
            {
                char szErrorMsg[200];
                snprintf(szErrorMsg, sizeof(szErrorMsg),
                         "Error parsing .shp to restore .shx. "
                         "Invalid record length = %u at record starting at "
                         "offset %u",
                         nSHPType, nCurrentSHPOffset);
                psHooks->Error(szErrorMsg);

                nRetCode = FALSE;
                break;
            }

            // Sanity check on record type
            if (nSHPType != SHPT_NULL && nSHPType != SHPT_POINT &&
                nSHPType != SHPT_ARC && nSHPType != SHPT_POLYGON &&
                nSHPType != SHPT_MULTIPOINT && nSHPType != SHPT_POINTZ &&
                nSHPType != SHPT_ARCZ && nSHPType != SHPT_POLYGONZ &&
                nSHPType != SHPT_MULTIPOINTZ && nSHPType != SHPT_POINTM &&
                nSHPType != SHPT_ARCM && nSHPType != SHPT_POLYGONM &&
                nSHPType != SHPT_MULTIPOINTM && nSHPType != SHPT_MULTIPATCH)
            {
                char szErrorMsg[200];
                snprintf(szErrorMsg, sizeof(szErrorMsg),
                         "Error parsing .shp to restore .shx. "
                         "Invalid shape type = %d at record starting at "
                         "offset %u",
                         nSHPType, nCurrentSHPOffset);
                psHooks->Error(szErrorMsg);

                nRetCode = FALSE;
                break;
            }

            psHooks->FWrite(abyReadRecord, 8, 1, fpSHX);

            nRecordOffset += nRecordLength + 4;
            // nCurrentRecordOffset += 8;
            nCurrentSHPOffset += 8 + nRecordLength * 2;

            psHooks->FSeek(fpSHP, nCurrentSHPOffset, 0);
            nRealSHXContentSize += 8;
        }
        else
        {
            char szErrorMsg[200];
            snprintf(szErrorMsg, sizeof(szErrorMsg),
                     "Error parsing .shp to restore .shx. "
                     "Cannot read first bytes of record starting at "
                     "offset %u",
                     nCurrentSHPOffset);
            psHooks->Error(szErrorMsg);

            nRetCode = FALSE;
            break;
        }
    }
    if (nRetCode && nCurrentSHPOffset != nSHPFilesize)
    {
        psHooks->Error("Error parsing .shp to restore .shx. "
                       "Not expected number of bytes");

        nRetCode = FALSE;
    }

    nRealSHXContentSize /= 2;  // Bytes counted -> WORDs
    if (!bBigEndian)
        SwapWord(4, &nRealSHXContentSize);
    psHooks->FSeek(fpSHX, 24, 0);
    psHooks->FWrite(&nRealSHXContentSize, 4, 1, fpSHX);

    psHooks->FClose(fpSHP);
    psHooks->FClose(fpSHX);

    free(pszFullname);
    free(pabySHXHeader);

    return nRetCode;
}

/************************************************************************/
/*                              SHPClose()                              */
/*                                                                      */
/*      Close the .shp and .shx files.                                  */
/************************************************************************/

void  SHPClose(SHPHandle psSHP)
{
    if (psSHP == SHPLIB_NULLPTR)
        return;

    /* -------------------------------------------------------------------- */
    /*      Update the header if we have modified anything.                 */
    /* -------------------------------------------------------------------- */
    if (psSHP->bUpdated)
        SHPWriteHeader(psSHP);

    /* -------------------------------------------------------------------- */
    /*      Free all resources, and close files.                            */
    /* -------------------------------------------------------------------- */
    free(psSHP->panRecOffset);
    free(psSHP->panRecSize);

    if (psSHP->fpSHX != SHPLIB_NULLPTR)
        psSHP->sHooks.FClose(psSHP->fpSHX);
    psSHP->sHooks.FClose(psSHP->fpSHP);

    if (psSHP->pabyRec != SHPLIB_NULLPTR)
    {
        free(psSHP->pabyRec);
    }

    if (psSHP->pabyObjectBuf != SHPLIB_NULLPTR)
    {
        free(psSHP->pabyObjectBuf);
    }
    if (psSHP->psCachedObject != SHPLIB_NULLPTR)
    {
        free(psSHP->psCachedObject);
    }

    free(psSHP);
}

/************************************************************************/
/*                    SHPSetFastModeReadObject()                        */
/************************************************************************/

/* If setting bFastMode = TRUE, the content of SHPReadObject() is owned by the SHPHandle. */
/* So you cannot have 2 valid instances of SHPReadObject() simultaneously. */
/* The SHPObject padfZ and padfM members may be NULL depending on the geometry */
/* type. It is illegal to free at hand any of the pointer members of the SHPObject structure */
void  SHPSetFastModeReadObject(SHPHandle hSHP, int bFastMode)
{
    if (bFastMode)
    {
        if (hSHP->psCachedObject == SHPLIB_NULLPTR)
        {
            hSHP->psCachedObject =
                STATIC_CAST(SHPObject *, calloc(1, sizeof(SHPObject)));
            assert(hSHP->psCachedObject != SHPLIB_NULLPTR);
        }
    }

    hSHP->bFastModeReadObject = bFastMode;
}

/************************************************************************/
/*                             SHPGetInfo()                             */
/*                                                                      */
/*      Fetch general information about the shape file.                 */
/************************************************************************/

void  SHPGetInfo(SHPHandle psSHP, int *pnEntities, int *pnShapeType,double *padfMinBound, double *padfMaxBound)
{
    if (psSHP == SHPLIB_NULLPTR)
        return;

    if (pnEntities != SHPLIB_NULLPTR)
        *pnEntities = psSHP->nRecords;

    if (pnShapeType != SHPLIB_NULLPTR)
        *pnShapeType = psSHP->nShapeType;

    for (int i = 0; i < 4; i++)
    {
        if (padfMinBound != SHPLIB_NULLPTR)
            padfMinBound[i] = psSHP->adBoundsMin[i];
        if (padfMaxBound != SHPLIB_NULLPTR)
            padfMaxBound[i] = psSHP->adBoundsMax[i];
    }
}

/************************************************************************/
/*                             SHPCreate()                              */
/*                                                                      */
/*      Create a new shape file and return a handle to the open         */
/*      shape file with read/write access.                              */
/************************************************************************/

 SAFile SADFOpen(const char* pszFilename, const char* pszAccess,void* pvUserData)
{
    (void)pvUserData;
    return (SAFile)fopen(pszFilename, pszAccess);
}

 SAOffset SADFRead(void* p, SAOffset size, SAOffset nmemb, SAFile file)
{
    return (SAOffset)fread(p, (size_t)size, (size_t)nmemb, (FILE*)file);
}

 SAOffset SADFWrite(const void* p, SAOffset size, SAOffset nmemb,SAFile file)
{
    return (SAOffset)fwrite(p, (size_t)size, (size_t)nmemb, (FILE*)file);
}

 SAOffset SADFSeek(SAFile file, SAOffset offset, int whence)
{
#if defined(_MSC_VER) && _MSC_VER >= 1400
    return (SAOffset)_fseeki64((FILE*)file, (__int64)offset, whence);
#else
    return (SAOffset)fseek((FILE*)file, (long)offset, whence);
#endif
}

 SAOffset SADFTell(SAFile file)
{
#if defined(_MSC_VER) && _MSC_VER >= 1400
    return (SAOffset)_ftelli64((FILE*)file);
#else
    return (SAOffset)ftell((FILE*)file);
#endif
}

 int SADFFlush(SAFile file)
{
    return fflush((FILE*)file);
}

 int SADFClose(SAFile file)
{
    return fclose((FILE*)file);
}

 int SADRemove(const char* filename, void* pvUserData)
{
    (void)pvUserData;
    return remove(filename);
}

 void SADError(const char* message)
{
    fprintf(stderr, "%s\n", message);
}

void SASetupDefaultHooks(SAHooks* psHooks)
{
    psHooks->FOpen = SADFOpen;
    psHooks->FRead = SADFRead;
    psHooks->FWrite = SADFWrite;
    psHooks->FSeek = SADFSeek;
    psHooks->FTell = SADFTell;
    psHooks->FFlush = SADFFlush;
    psHooks->FClose = SADFClose;
    psHooks->Remove = SADRemove;

    psHooks->Error = SADError;
    psHooks->Atof = atof;
    psHooks->pvUserData = NULL;
}

SHPHandle  SHPCreate(const char *pszLayer, int nShapeType)
{
    SAHooks sHooks;

    SASetupDefaultHooks(&sHooks);

    return SHPCreateLL(pszLayer, nShapeType, &sHooks);
}

/************************************************************************/
/*                             SHPCreate()                              */
/*                                                                      */
/*      Create a new shape file and return a handle to the open         */
/*      shape file with read/write access.                              */
/************************************************************************/

SHPHandle  SHPCreateLL(const char *pszLayer, int nShapeType,const SAHooks *psHooks)
{
/* -------------------------------------------------------------------- */
/*      Establish the byte order on this system.                        */
/* -------------------------------------------------------------------- */
#if !defined(bBigEndian)
    {
        int i = 1;
        if (*((unsigned char *)&i) == 1)
            bBigEndian = false;
        else
            bBigEndian = true;
    }
#endif

    /* -------------------------------------------------------------------- */
    /*      Open the two files so we can write their headers.               */
    /* -------------------------------------------------------------------- */
    const int nLenWithoutExtension = SHPGetLenWithoutExtension(pszLayer);
    char *pszFullname = STATIC_CAST(char *, malloc(nLenWithoutExtension + 5));
    memcpy(pszFullname, pszLayer, nLenWithoutExtension);
    memcpy(pszFullname + nLenWithoutExtension, ".shp", 5);
    SAFile fpSHP = psHooks->FOpen(pszFullname, "w+b", psHooks->pvUserData);
    if (fpSHP == SHPLIB_NULLPTR)
    {
        char szErrorMsg[200];
        snprintf(szErrorMsg, sizeof(szErrorMsg), "Failed to create file %s: %s",
                 pszFullname, strerror(errno));
        psHooks->Error(szErrorMsg);

        free(pszFullname);
        return NULL;
    }

    memcpy(pszFullname + nLenWithoutExtension, ".shx", 5);
    SAFile fpSHX = psHooks->FOpen(pszFullname, "w+b", psHooks->pvUserData);
    if (fpSHX == SHPLIB_NULLPTR)
    {
        char szErrorMsg[200];
        snprintf(szErrorMsg, sizeof(szErrorMsg), "Failed to create file %s: %s",
                 pszFullname, strerror(errno));
        psHooks->Error(szErrorMsg);

        free(pszFullname);
        psHooks->FClose(fpSHP);
        return NULL;
    }

    free(pszFullname);
    pszFullname = SHPLIB_NULLPTR;

    /* -------------------------------------------------------------------- */
    /*      Prepare header block for .shp file.                             */
    /* -------------------------------------------------------------------- */
    unsigned char abyHeader[100];
    memset(abyHeader, 0, sizeof(abyHeader));

    abyHeader[2] = 0x27; /* magic cookie */
    abyHeader[3] = 0x0a;

    uint32_t i32 = 50; /* file size */
    ByteCopy(&i32, abyHeader + 24, 4);
    if (!bBigEndian)
        SwapWord(4, abyHeader + 24);

    i32 = 1000; /* version */
    ByteCopy(&i32, abyHeader + 28, 4);
    if (bBigEndian)
        SwapWord(4, abyHeader + 28);

    i32 = nShapeType; /* shape type */
    ByteCopy(&i32, abyHeader + 32, 4);
    if (bBigEndian)
        SwapWord(4, abyHeader + 32);

    double dValue = 0.0; /* set bounds */
    ByteCopy(&dValue, abyHeader + 36, 8);
    ByteCopy(&dValue, abyHeader + 44, 8);
    ByteCopy(&dValue, abyHeader + 52, 8);
    ByteCopy(&dValue, abyHeader + 60, 8);

    /* -------------------------------------------------------------------- */
    /*      Write .shp file header.                                         */
    /* -------------------------------------------------------------------- */
    if (psHooks->FWrite(abyHeader, 100, 1, fpSHP) != 1)
    {
        char szErrorMsg[200];

        snprintf(szErrorMsg, sizeof(szErrorMsg),
                 "Failed to write .shp header: %s", strerror(errno));
        szErrorMsg[sizeof(szErrorMsg) - 1] = '\0';
        psHooks->Error(szErrorMsg);

        free(pszFullname);
        psHooks->FClose(fpSHP);
        psHooks->FClose(fpSHX);
        return NULL;
    }

    /* -------------------------------------------------------------------- */
    /*      Prepare, and write .shx file header.                            */
    /* -------------------------------------------------------------------- */
    i32 = 50; /* file size */
    ByteCopy(&i32, abyHeader + 24, 4);
    if (!bBigEndian)
        SwapWord(4, abyHeader + 24);

    if (psHooks->FWrite(abyHeader, 100, 1, fpSHX) != 1)
    {
        char szErrorMsg[200];

        snprintf(szErrorMsg, sizeof(szErrorMsg),
                 "Failure writing .shx header: %s", strerror(errno));
        szErrorMsg[sizeof(szErrorMsg) - 1] = '\0';
        psHooks->Error(szErrorMsg);

        free(pszFullname);
        psHooks->FClose(fpSHP);
        psHooks->FClose(fpSHX);
        return NULL;
    }

    SHPHandle psSHP = STATIC_CAST(SHPHandle, calloc(sizeof(SHPInfo), 1));

    psSHP->bUpdated = FALSE;
    memcpy(&(psSHP->sHooks), psHooks, sizeof(SAHooks));

    psSHP->fpSHP = fpSHP;
    psSHP->fpSHX = fpSHX;
    psSHP->nShapeType = nShapeType;
    psSHP->nFileSize = 100;
    psSHP->panRecOffset =
        STATIC_CAST(unsigned int *, malloc(sizeof(unsigned int)));
    psSHP->panRecSize =
        STATIC_CAST(unsigned int *, malloc(sizeof(unsigned int)));

    if (psSHP->panRecOffset == SHPLIB_NULLPTR ||
        psSHP->panRecSize == SHPLIB_NULLPTR)
    {
        psSHP->sHooks.Error("Not enough memory to allocate requested memory");
        psSHP->sHooks.FClose(psSHP->fpSHP);
        psSHP->sHooks.FClose(psSHP->fpSHX);
        if (psSHP->panRecOffset)
            free(psSHP->panRecOffset);
        if (psSHP->panRecSize)
            free(psSHP->panRecSize);
        free(psSHP);
        return SHPLIB_NULLPTR;
    }

    return psSHP;
}

/************************************************************************/
/*                           _SHPSetBounds()                            */
/*                                                                      */
/*      Compute a bounds rectangle for a shape, and set it into the     */
/*      indicated location in the record.                               */
/************************************************************************/

 void _SHPSetBounds(unsigned char *pabyRec, const SHPObject *psShape)
{
    ByteCopy(&(psShape->dfXMin), pabyRec + 0, 8);
    ByteCopy(&(psShape->dfYMin), pabyRec + 8, 8);
    ByteCopy(&(psShape->dfXMax), pabyRec + 16, 8);
    ByteCopy(&(psShape->dfYMax), pabyRec + 24, 8);

    if (bBigEndian)
    {
        SwapWord(8, pabyRec + 0);
        SwapWord(8, pabyRec + 8);
        SwapWord(8, pabyRec + 16);
        SwapWord(8, pabyRec + 24);
    }
}

/************************************************************************/
/*                         SHPComputeExtents()                          */
/*                                                                      */
/*      Recompute the extents of a shape.  Automatically done by        */
/*      SHPCreateObject().                                              */
/************************************************************************/

void  SHPComputeExtents(SHPObject *psObject)
{
    /* -------------------------------------------------------------------- */
    /*      Build extents for this object.                                  */
    /* -------------------------------------------------------------------- */
    if (psObject->nVertices > 0)
    {
        psObject->dfXMin = psObject->dfXMax = psObject->padfX[0];
        psObject->dfYMin = psObject->dfYMax = psObject->padfY[0];
        psObject->dfZMin = psObject->dfZMax = psObject->padfZ[0];
        psObject->dfMMin = psObject->dfMMax = psObject->padfM[0];
    }

    for (int i = 0; i < psObject->nVertices; i++)
    {
        psObject->dfXMin = MIN(psObject->dfXMin, psObject->padfX[i]);
        psObject->dfYMin = MIN(psObject->dfYMin, psObject->padfY[i]);
        psObject->dfZMin = MIN(psObject->dfZMin, psObject->padfZ[i]);
        psObject->dfMMin = MIN(psObject->dfMMin, psObject->padfM[i]);

        psObject->dfXMax = MAX(psObject->dfXMax, psObject->padfX[i]);
        psObject->dfYMax = MAX(psObject->dfYMax, psObject->padfY[i]);
        psObject->dfZMax = MAX(psObject->dfZMax, psObject->padfZ[i]);
        psObject->dfMMax = MAX(psObject->dfMMax, psObject->padfM[i]);
    }
}

/************************************************************************/
/*                          SHPCreateObject()                           */
/*                                                                      */
/*      Create a shape object.  It should be freed with                 */
/*      SHPDestroyObject().                                             */
/************************************************************************/

SHPObject *
    SHPCreateObject(int nSHPType, int nShapeId, int nParts,
                    const int *panPartStart, const int *panPartType,
                    int nVertices, const double *padfX, const double *padfY,
                    const double *padfZ, const double *padfM)
{
    SHPObject *psObject =
        STATIC_CAST(SHPObject *, calloc(1, sizeof(SHPObject)));
    psObject->nSHPType = nSHPType;
    psObject->nShapeId = nShapeId;
    psObject->bMeasureIsUsed = FALSE;

    /* -------------------------------------------------------------------- */
    /*      Establish whether this shape type has M, and Z values.          */
    /* -------------------------------------------------------------------- */
    bool bHasM;
    bool bHasZ;

    if (nSHPType == SHPT_ARCM || nSHPType == SHPT_POINTM ||
        nSHPType == SHPT_POLYGONM || nSHPType == SHPT_MULTIPOINTM)
    {
        bHasM = true;
        bHasZ = false;
    }
    else if (nSHPType == SHPT_ARCZ || nSHPType == SHPT_POINTZ ||
             nSHPType == SHPT_POLYGONZ || nSHPType == SHPT_MULTIPOINTZ ||
             nSHPType == SHPT_MULTIPATCH)
    {
        bHasM = true;
        bHasZ = true;
    }
    else
    {
        bHasM = false;
        bHasZ = false;
    }

    /* -------------------------------------------------------------------- */
    /*      Capture parts.  Note that part type is optional, and            */
    /*      defaults to ring.                                               */
    /* -------------------------------------------------------------------- */
    if (nSHPType == SHPT_ARC || nSHPType == SHPT_POLYGON ||
        nSHPType == SHPT_ARCM || nSHPType == SHPT_POLYGONM ||
        nSHPType == SHPT_ARCZ || nSHPType == SHPT_POLYGONZ ||
        nSHPType == SHPT_MULTIPATCH)
    {
        psObject->nParts = MAX(1, nParts);

        psObject->panPartStart =
            STATIC_CAST(int *, calloc(sizeof(int), psObject->nParts));
        psObject->panPartType =
            STATIC_CAST(int *, malloc(sizeof(int) * psObject->nParts));

        psObject->panPartStart[0] = 0;
        psObject->panPartType[0] = SHPP_RING;

        for (int i = 0; i < nParts; i++)
        {
            if (panPartStart != SHPLIB_NULLPTR)
                psObject->panPartStart[i] = panPartStart[i];

            if (panPartType != SHPLIB_NULLPTR)
                psObject->panPartType[i] = panPartType[i];
            else
                psObject->panPartType[i] = SHPP_RING;
        }

        if (psObject->panPartStart[0] != 0)
            psObject->panPartStart[0] = 0;
    }

    /* -------------------------------------------------------------------- */
    /*      Capture vertices.  Note that X, Y, Z and M are optional.        */
    /* -------------------------------------------------------------------- */
    if (nVertices > 0)
    {
        const size_t nSize = sizeof(double) * nVertices;
        psObject->padfX =
            STATIC_CAST(double *, padfX ? malloc(nSize)
                                        : calloc(sizeof(double), nVertices));
        psObject->padfY =
            STATIC_CAST(double *, padfY ? malloc(nSize)
                                        : calloc(sizeof(double), nVertices));
        psObject->padfZ = STATIC_CAST(
            double *,
            padfZ &&bHasZ ? malloc(nSize) : calloc(sizeof(double), nVertices));
        psObject->padfM = STATIC_CAST(
            double *,
            padfM &&bHasM ? malloc(nSize) : calloc(sizeof(double), nVertices));
        if (padfX != SHPLIB_NULLPTR)
            memcpy(psObject->padfX, padfX, nSize);
        if (padfY != SHPLIB_NULLPTR)
            memcpy(psObject->padfY, padfY, nSize);
        if (padfZ != SHPLIB_NULLPTR && bHasZ)
            memcpy(psObject->padfZ, padfZ, nSize);
        if (padfM != SHPLIB_NULLPTR && bHasM)
        {
            memcpy(psObject->padfM, padfM, nSize);
            psObject->bMeasureIsUsed = TRUE;
        }
    }

    /* -------------------------------------------------------------------- */
    /*      Compute the extents.                                            */
    /* -------------------------------------------------------------------- */
    psObject->nVertices = nVertices;
    SHPComputeExtents(psObject);

    return (psObject);
}

/************************************************************************/
/*                       SHPCreateSimpleObject()                        */
/*                                                                      */
/*      Create a simple (common) shape object.  Destroy with            */
/*      SHPDestroyObject().                                             */
/************************************************************************/

SHPObject *
    SHPCreateSimpleObject(int nSHPType, int nVertices, const double *padfX,
                          const double *padfY, const double *padfZ)
{
    return (SHPCreateObject(nSHPType, -1, 0, SHPLIB_NULLPTR, SHPLIB_NULLPTR,
                            nVertices, padfX, padfY, padfZ, SHPLIB_NULLPTR));
}

/************************************************************************/
/*                           SHPWriteObject()                           */
/*                                                                      */
/*      Write out the vertices of a new structure.  Note that it is     */
/*      only possible to write vertices at the end of the file.         */
/************************************************************************/

int  SHPWriteObject(SHPHandle psSHP, int nShapeId,
                               SHPObject *psObject)
{
    psSHP->bUpdated = TRUE;

    /* -------------------------------------------------------------------- */
    /*      Ensure that shape object matches the type of the file it is     */
    /*      being written to.                                               */
    /* -------------------------------------------------------------------- */
    assert(psObject->nSHPType == psSHP->nShapeType ||
           psObject->nSHPType == SHPT_NULL);

    /* -------------------------------------------------------------------- */
    /*      Ensure that -1 is used for appends.  Either blow an             */
    /*      assertion, or if they are disabled, set the shapeid to -1       */
    /*      for appends.                                                    */
    /* -------------------------------------------------------------------- */
    assert(nShapeId == -1 || (nShapeId >= 0 && nShapeId < psSHP->nRecords));

    if (nShapeId != -1 && nShapeId >= psSHP->nRecords)
        nShapeId = -1;

    /* -------------------------------------------------------------------- */
    /*      Add the new entity to the in memory index.                      */
    /* -------------------------------------------------------------------- */
    if (nShapeId == -1 && psSHP->nRecords + 1 > psSHP->nMaxRecords)
    {
        /* This cannot overflow given that we check that the file size does
         * not grow over 4 GB, and the minimum size of a record is 12 bytes,
         * hence the maximm value for nMaxRecords is 357,913,941
         */
        int nNewMaxRecords = psSHP->nMaxRecords + psSHP->nMaxRecords / 3 + 100;
        unsigned int *panRecOffsetNew;
        unsigned int *panRecSizeNew;

        panRecOffsetNew = STATIC_CAST(
            unsigned int *, realloc(psSHP->panRecOffset,
                                    sizeof(unsigned int) * nNewMaxRecords));
        if (panRecOffsetNew == SHPLIB_NULLPTR)
        {
            psSHP->sHooks.Error("Failed to write shape object. "
                                "Memory allocation error.");
            return -1;
        }
        psSHP->panRecOffset = panRecOffsetNew;

        panRecSizeNew = STATIC_CAST(
            unsigned int *,
            realloc(psSHP->panRecSize, sizeof(unsigned int) * nNewMaxRecords));
        if (panRecSizeNew == SHPLIB_NULLPTR)
        {
            psSHP->sHooks.Error("Failed to write shape object. "
                                "Memory allocation error.");
            return -1;
        }
        psSHP->panRecSize = panRecSizeNew;

        psSHP->nMaxRecords = nNewMaxRecords;
    }

    /* -------------------------------------------------------------------- */
    /*      Initialize record.                                              */
    /* -------------------------------------------------------------------- */

    /* The following computation cannot overflow on 32-bit platforms given that
     * the user had to allocate arrays of at least that size. */
    size_t nRecMaxSize =
        psObject->nVertices * 4 * sizeof(double) + psObject->nParts * 8;
    /* But the following test could trigger on 64-bit platforms on huge
     * geometries. */
    const unsigned nExtraSpaceForGeomHeader = 128;
    if (nRecMaxSize > UINT_MAX - nExtraSpaceForGeomHeader)
    {
        psSHP->sHooks.Error("Failed to write shape object. Too big geometry.");
        return -1;
    }
    nRecMaxSize += nExtraSpaceForGeomHeader;
    unsigned char *pabyRec = STATIC_CAST(unsigned char *, malloc(nRecMaxSize));
    if (pabyRec == SHPLIB_NULLPTR)
    {
        psSHP->sHooks.Error("Failed to write shape object. "
                            "Memory allocation error.");
        return -1;
    }

    /* -------------------------------------------------------------------- */
    /*      Extract vertices for a Polygon or Arc.                          */
    /* -------------------------------------------------------------------- */
    unsigned int nRecordSize = 0;
    const bool bFirstFeature = psSHP->nRecords == 0;

    if (psObject->nSHPType == SHPT_POLYGON ||
        psObject->nSHPType == SHPT_POLYGONZ ||
        psObject->nSHPType == SHPT_POLYGONM || psObject->nSHPType == SHPT_ARC ||
        psObject->nSHPType == SHPT_ARCZ || psObject->nSHPType == SHPT_ARCM ||
        psObject->nSHPType == SHPT_MULTIPATCH)
    {
        uint32_t nPoints = psObject->nVertices;
        uint32_t nParts = psObject->nParts;

        _SHPSetBounds(pabyRec + 12, psObject);

        if (bBigEndian)
            SwapWord(4, &nPoints);
        if (bBigEndian)
            SwapWord(4, &nParts);

        ByteCopy(&nPoints, pabyRec + 40 + 8, 4);
        ByteCopy(&nParts, pabyRec + 36 + 8, 4);

        nRecordSize = 52;

        /*
         * Write part start positions.
         */
        ByteCopy(psObject->panPartStart, pabyRec + 44 + 8,
                 4 * psObject->nParts);
        for (int i = 0; i < psObject->nParts; i++)
        {
            if (bBigEndian)
                SwapWord(4, pabyRec + 44 + 8 + 4 * i);
            nRecordSize += 4;
        }

        /*
         * Write multipatch part types if needed.
         */
        if (psObject->nSHPType == SHPT_MULTIPATCH)
        {
            memcpy(pabyRec + nRecordSize, psObject->panPartType,
                   4 * psObject->nParts);
            for (int i = 0; i < psObject->nParts; i++)
            {
                if (bBigEndian)
                    SwapWord(4, pabyRec + nRecordSize);
                nRecordSize += 4;
            }
        }

        /*
         * Write the (x,y) vertex values.
         */
        for (int i = 0; i < psObject->nVertices; i++)
        {
            ByteCopy(psObject->padfX + i, pabyRec + nRecordSize, 8);
            ByteCopy(psObject->padfY + i, pabyRec + nRecordSize + 8, 8);

            if (bBigEndian)
                SwapWord(8, pabyRec + nRecordSize);

            if (bBigEndian)
                SwapWord(8, pabyRec + nRecordSize + 8);

            nRecordSize += 2 * 8;
        }

        /*
         * Write the Z coordinates (if any).
         */
        if (psObject->nSHPType == SHPT_POLYGONZ ||
            psObject->nSHPType == SHPT_ARCZ ||
            psObject->nSHPType == SHPT_MULTIPATCH)
        {
            ByteCopy(&(psObject->dfZMin), pabyRec + nRecordSize, 8);
            if (bBigEndian)
                SwapWord(8, pabyRec + nRecordSize);
            nRecordSize += 8;

            ByteCopy(&(psObject->dfZMax), pabyRec + nRecordSize, 8);
            if (bBigEndian)
                SwapWord(8, pabyRec + nRecordSize);
            nRecordSize += 8;

            for (int i = 0; i < psObject->nVertices; i++)
            {
                ByteCopy(psObject->padfZ + i, pabyRec + nRecordSize, 8);
                if (bBigEndian)
                    SwapWord(8, pabyRec + nRecordSize);
                nRecordSize += 8;
            }
        }

        /*
         * Write the M values, if any.
         */
        if (psObject->bMeasureIsUsed &&
            (psObject->nSHPType == SHPT_POLYGONM ||
             psObject->nSHPType == SHPT_ARCM
#ifndef DISABLE_MULTIPATCH_MEASURE
             || psObject->nSHPType == SHPT_MULTIPATCH
#endif
             || psObject->nSHPType == SHPT_POLYGONZ ||
             psObject->nSHPType == SHPT_ARCZ))
        {
            ByteCopy(&(psObject->dfMMin), pabyRec + nRecordSize, 8);
            if (bBigEndian)
                SwapWord(8, pabyRec + nRecordSize);
            nRecordSize += 8;

            ByteCopy(&(psObject->dfMMax), pabyRec + nRecordSize, 8);
            if (bBigEndian)
                SwapWord(8, pabyRec + nRecordSize);
            nRecordSize += 8;

            for (int i = 0; i < psObject->nVertices; i++)
            {
                ByteCopy(psObject->padfM + i, pabyRec + nRecordSize, 8);
                if (bBigEndian)
                    SwapWord(8, pabyRec + nRecordSize);
                nRecordSize += 8;
            }
        }
    }

    /* -------------------------------------------------------------------- */
    /*      Extract vertices for a MultiPoint.                              */
    /* -------------------------------------------------------------------- */
    else if (psObject->nSHPType == SHPT_MULTIPOINT ||
             psObject->nSHPType == SHPT_MULTIPOINTZ ||
             psObject->nSHPType == SHPT_MULTIPOINTM)
    {
        uint32_t nPoints = psObject->nVertices;

        _SHPSetBounds(pabyRec + 12, psObject);

        if (bBigEndian)
            SwapWord(4, &nPoints);
        ByteCopy(&nPoints, pabyRec + 44, 4);

        for (int i = 0; i < psObject->nVertices; i++)
        {
            ByteCopy(psObject->padfX + i, pabyRec + 48 + i * 16, 8);
            ByteCopy(psObject->padfY + i, pabyRec + 48 + i * 16 + 8, 8);

            if (bBigEndian)
                SwapWord(8, pabyRec + 48 + i * 16);
            if (bBigEndian)
                SwapWord(8, pabyRec + 48 + i * 16 + 8);
        }

        nRecordSize = 48 + 16 * psObject->nVertices;

        if (psObject->nSHPType == SHPT_MULTIPOINTZ)
        {
            ByteCopy(&(psObject->dfZMin), pabyRec + nRecordSize, 8);
            if (bBigEndian)
                SwapWord(8, pabyRec + nRecordSize);
            nRecordSize += 8;

            ByteCopy(&(psObject->dfZMax), pabyRec + nRecordSize, 8);
            if (bBigEndian)
                SwapWord(8, pabyRec + nRecordSize);
            nRecordSize += 8;

            for (int i = 0; i < psObject->nVertices; i++)
            {
                ByteCopy(psObject->padfZ + i, pabyRec + nRecordSize, 8);
                if (bBigEndian)
                    SwapWord(8, pabyRec + nRecordSize);
                nRecordSize += 8;
            }
        }

        if (psObject->bMeasureIsUsed &&
            (psObject->nSHPType == SHPT_MULTIPOINTZ ||
             psObject->nSHPType == SHPT_MULTIPOINTM))
        {
            ByteCopy(&(psObject->dfMMin), pabyRec + nRecordSize, 8);
            if (bBigEndian)
                SwapWord(8, pabyRec + nRecordSize);
            nRecordSize += 8;

            ByteCopy(&(psObject->dfMMax), pabyRec + nRecordSize, 8);
            if (bBigEndian)
                SwapWord(8, pabyRec + nRecordSize);
            nRecordSize += 8;

            for (int i = 0; i < psObject->nVertices; i++)
            {
                ByteCopy(psObject->padfM + i, pabyRec + nRecordSize, 8);
                if (bBigEndian)
                    SwapWord(8, pabyRec + nRecordSize);
                nRecordSize += 8;
            }
        }
    }

    /* -------------------------------------------------------------------- */
    /*      Write point.                                                    */
    /* -------------------------------------------------------------------- */
    else if (psObject->nSHPType == SHPT_POINT ||
             psObject->nSHPType == SHPT_POINTZ ||
             psObject->nSHPType == SHPT_POINTM)
    {
        ByteCopy(psObject->padfX, pabyRec + 12, 8);
        ByteCopy(psObject->padfY, pabyRec + 20, 8);

        if (bBigEndian)
            SwapWord(8, pabyRec + 12);
        if (bBigEndian)
            SwapWord(8, pabyRec + 20);

        nRecordSize = 28;

        if (psObject->nSHPType == SHPT_POINTZ)
        {
            ByteCopy(psObject->padfZ, pabyRec + nRecordSize, 8);
            if (bBigEndian)
                SwapWord(8, pabyRec + nRecordSize);
            nRecordSize += 8;
        }

        if (psObject->bMeasureIsUsed && (psObject->nSHPType == SHPT_POINTZ ||
                                         psObject->nSHPType == SHPT_POINTM))
        {
            ByteCopy(psObject->padfM, pabyRec + nRecordSize, 8);
            if (bBigEndian)
                SwapWord(8, pabyRec + nRecordSize);
            nRecordSize += 8;
        }
    }

    /* -------------------------------------------------------------------- */
    /*      Not much to do for null geometries.                             */
    /* -------------------------------------------------------------------- */
    else if (psObject->nSHPType == SHPT_NULL)
    {
        nRecordSize = 12;
    }
    else
    {
        /* unknown type */
        assert(false);
    }

    /* -------------------------------------------------------------------- */
    /*      Establish where we are going to put this record. If we are      */
    /*      rewriting the last record of the file, then we can update it in */
    /*      place. Otherwise if rewriting an existing record, and it will   */
    /*      fit, then put it  back where the original came from.  Otherwise */
    /*      write at the end.                                               */
    /* -------------------------------------------------------------------- */
    SAOffset nRecordOffset;
    bool bAppendToLastRecord = false;
    bool bAppendToFile = false;
    if (nShapeId != -1 &&
        psSHP->panRecOffset[nShapeId] + psSHP->panRecSize[nShapeId] + 8 ==
            psSHP->nFileSize)
    {
        nRecordOffset = psSHP->panRecOffset[nShapeId];
        bAppendToLastRecord = true;
    }
    else if (nShapeId == -1 || psSHP->panRecSize[nShapeId] < nRecordSize - 8)
    {
        if (psSHP->nFileSize > UINT_MAX - nRecordSize)
        {
            char str[255];
            snprintf(str, sizeof(str),
                     "Failed to write shape object. "
                     "The maximum file size of %u has been reached. "
                     "The current record of size %u cannot be added.",
                     psSHP->nFileSize, nRecordSize);
            str[sizeof(str) - 1] = '\0';
            psSHP->sHooks.Error(str);
            free(pabyRec);
            return -1;
        }

        bAppendToFile = true;
        nRecordOffset = psSHP->nFileSize;
    }
    else
    {
        nRecordOffset = psSHP->panRecOffset[nShapeId];
    }

    /* -------------------------------------------------------------------- */
    /*      Set the shape type, record number, and record size.             */
    /* -------------------------------------------------------------------- */
    uint32_t i32 =
        (nShapeId < 0) ? psSHP->nRecords + 1 : nShapeId + 1; /* record # */
    if (!bBigEndian)
        SwapWord(4, &i32);
    ByteCopy(&i32, pabyRec, 4);

    i32 = (nRecordSize - 8) / 2; /* record size */
    if (!bBigEndian)
        SwapWord(4, &i32);
    ByteCopy(&i32, pabyRec + 4, 4);

    i32 = psObject->nSHPType; /* shape type */
    if (bBigEndian)
        SwapWord(4, &i32);
    ByteCopy(&i32, pabyRec + 8, 4);

    /* -------------------------------------------------------------------- */
    /*      Write out record.                                               */
    /* -------------------------------------------------------------------- */

    /* -------------------------------------------------------------------- */
    /*      Guard FSeek with check for whether we're already at position;   */
    /*      no-op FSeeks defeat network filesystems' write buffering.       */
    /* -------------------------------------------------------------------- */
    if (psSHP->sHooks.FTell(psSHP->fpSHP) != nRecordOffset)
    {
        if (psSHP->sHooks.FSeek(psSHP->fpSHP, nRecordOffset, 0) != 0)
        {
            char szErrorMsg[200];

            snprintf(szErrorMsg, sizeof(szErrorMsg),
                     "Error in psSHP->sHooks.FSeek() while writing object to "
                     ".shp file: %s",
                     strerror(errno));
            szErrorMsg[sizeof(szErrorMsg) - 1] = '\0';
            psSHP->sHooks.Error(szErrorMsg);

            free(pabyRec);
            return -1;
        }
    }
    if (psSHP->sHooks.FWrite(pabyRec, nRecordSize, 1, psSHP->fpSHP) < 1)
    {
        char szErrorMsg[200];

        snprintf(szErrorMsg, sizeof(szErrorMsg),
                 "Error in psSHP->sHooks.FWrite() while writing object of %u "
                 "bytes to .shp file: %s",
                 nRecordSize, strerror(errno));
        szErrorMsg[sizeof(szErrorMsg) - 1] = '\0';
        psSHP->sHooks.Error(szErrorMsg);

        free(pabyRec);
        return -1;
    }

    free(pabyRec);

    if (bAppendToLastRecord)
    {
        psSHP->nFileSize = psSHP->panRecOffset[nShapeId] + nRecordSize;
    }
    else if (bAppendToFile)
    {
        if (nShapeId == -1)
            nShapeId = psSHP->nRecords++;

        psSHP->panRecOffset[nShapeId] = psSHP->nFileSize;
        psSHP->nFileSize += nRecordSize;
    }
    psSHP->panRecSize[nShapeId] = nRecordSize - 8;

    /* -------------------------------------------------------------------- */
    /*      Expand file wide bounds based on this shape.                    */
    /* -------------------------------------------------------------------- */
    if (bFirstFeature)
    {
        if (psObject->nSHPType == SHPT_NULL || psObject->nVertices == 0)
        {
            psSHP->adBoundsMin[0] = psSHP->adBoundsMax[0] = 0.0;
            psSHP->adBoundsMin[1] = psSHP->adBoundsMax[1] = 0.0;
            psSHP->adBoundsMin[2] = psSHP->adBoundsMax[2] = 0.0;
            psSHP->adBoundsMin[3] = psSHP->adBoundsMax[3] = 0.0;
        }
        else
        {
            psSHP->adBoundsMin[0] = psSHP->adBoundsMax[0] = psObject->padfX[0];
            psSHP->adBoundsMin[1] = psSHP->adBoundsMax[1] = psObject->padfY[0];
            psSHP->adBoundsMin[2] = psSHP->adBoundsMax[2] =
                psObject->padfZ ? psObject->padfZ[0] : 0.0;
            psSHP->adBoundsMin[3] = psSHP->adBoundsMax[3] =
                psObject->padfM ? psObject->padfM[0] : 0.0;
        }
    }

    for (int i = 0; i < psObject->nVertices; i++)
    {
        psSHP->adBoundsMin[0] = MIN(psSHP->adBoundsMin[0], psObject->padfX[i]);
        psSHP->adBoundsMin[1] = MIN(psSHP->adBoundsMin[1], psObject->padfY[i]);
        psSHP->adBoundsMax[0] = MAX(psSHP->adBoundsMax[0], psObject->padfX[i]);
        psSHP->adBoundsMax[1] = MAX(psSHP->adBoundsMax[1], psObject->padfY[i]);
        if (psObject->padfZ)
        {
            psSHP->adBoundsMin[2] =
                MIN(psSHP->adBoundsMin[2], psObject->padfZ[i]);
            psSHP->adBoundsMax[2] =
                MAX(psSHP->adBoundsMax[2], psObject->padfZ[i]);
        }
        if (psObject->padfM)
        {
            psSHP->adBoundsMin[3] =
                MIN(psSHP->adBoundsMin[3], psObject->padfM[i]);
            psSHP->adBoundsMax[3] =
                MAX(psSHP->adBoundsMax[3], psObject->padfM[i]);
        }
    }

    return (nShapeId);
}

/************************************************************************/
/*                         SHPAllocBuffer()                             */
/************************************************************************/

 void *SHPAllocBuffer(unsigned char **pBuffer, int nSize)
{
    if (pBuffer == SHPLIB_NULLPTR)
        return calloc(1, nSize);

    unsigned char *pRet = *pBuffer;
    if (pRet == SHPLIB_NULLPTR)
        return SHPLIB_NULLPTR;

    (*pBuffer) += nSize;
    return pRet;
}

/************************************************************************/
/*                    SHPReallocObjectBufIfNecessary()                  */
/************************************************************************/

 unsigned char *SHPReallocObjectBufIfNecessary(SHPHandle psSHP,
                                                     int nObjectBufSize)
{
    if (nObjectBufSize == 0)
    {
        nObjectBufSize = 4 * sizeof(double);
    }

    unsigned char *pBuffer;
    if (nObjectBufSize > psSHP->nObjectBufSize)
    {
        pBuffer = STATIC_CAST(unsigned char *,
                              realloc(psSHP->pabyObjectBuf, nObjectBufSize));
        if (pBuffer != SHPLIB_NULLPTR)
        {
            psSHP->pabyObjectBuf = pBuffer;
            psSHP->nObjectBufSize = nObjectBufSize;
        }
    }
    else
    {
        pBuffer = psSHP->pabyObjectBuf;
    }

    return pBuffer;
}

/************************************************************************/
/*                          SHPReadObject()                             */
/*                                                                      */
/*      Read the vertices, parts, and other non-attribute information   */
/*      for one shape.                                                  */
/************************************************************************/

SHPObject * SHPReadObject(SHPHandle psSHP, int hEntity)
{
    /* -------------------------------------------------------------------- */
    /*      Validate the record/entity number.                              */
    /* -------------------------------------------------------------------- */
    if (hEntity < 0 || hEntity >= psSHP->nRecords)
        return SHPLIB_NULLPTR;

    /* -------------------------------------------------------------------- */
    /*      Read offset/length from SHX loading if necessary.               */
    /* -------------------------------------------------------------------- */
    if (psSHP->panRecOffset[hEntity] == 0 && psSHP->fpSHX != SHPLIB_NULLPTR)
    {
        unsigned int nOffset;
        unsigned int nLength;

        if (psSHP->sHooks.FSeek(psSHP->fpSHX, 100 + 8 * hEntity, 0) != 0 ||
            psSHP->sHooks.FRead(&nOffset, 1, 4, psSHP->fpSHX) != 4 ||
            psSHP->sHooks.FRead(&nLength, 1, 4, psSHP->fpSHX) != 4)
        {
            char str[128];
            snprintf(str, sizeof(str),
                     "Error in fseek()/fread() reading object from .shx file "
                     "at offset %d",
                     100 + 8 * hEntity);
            str[sizeof(str) - 1] = '\0';

            psSHP->sHooks.Error(str);
            return SHPLIB_NULLPTR;
        }
        if (!bBigEndian)
            SwapWord(4, &nOffset);
        if (!bBigEndian)
            SwapWord(4, &nLength);

        if (nOffset > STATIC_CAST(unsigned int, INT_MAX))
        {
            char str[128];
            snprintf(str, sizeof(str), "Invalid offset for entity %d", hEntity);
            str[sizeof(str) - 1] = '\0';

            psSHP->sHooks.Error(str);
            return SHPLIB_NULLPTR;
        }
        if (nLength > STATIC_CAST(unsigned int, INT_MAX / 2 - 4))
        {
            char str[128];
            snprintf(str, sizeof(str), "Invalid length for entity %d", hEntity);
            str[sizeof(str) - 1] = '\0';

            psSHP->sHooks.Error(str);
            return SHPLIB_NULLPTR;
        }

        psSHP->panRecOffset[hEntity] = nOffset * 2;
        psSHP->panRecSize[hEntity] = nLength * 2;
    }

    /* -------------------------------------------------------------------- */
    /*      Ensure our record buffer is large enough.                       */
    /* -------------------------------------------------------------------- */
    const int nEntitySize = psSHP->panRecSize[hEntity] + 8;
    if (nEntitySize > psSHP->nBufSize)
    {
        int nNewBufSize = nEntitySize;
        if (nNewBufSize < INT_MAX - nNewBufSize / 3)
            nNewBufSize += nNewBufSize / 3;
        else
            nNewBufSize = INT_MAX;

        /* Before allocating too much memory, check that the file is big enough */
        /* and do not trust the file size in the header the first time we */
        /* need to allocate more than 10 MB */
        if (nNewBufSize >= 10 * 1024 * 1024)
        {
            if (psSHP->nBufSize < 10 * 1024 * 1024)
            {
                SAOffset nFileSize;
                psSHP->sHooks.FSeek(psSHP->fpSHP, 0, 2);
                nFileSize = psSHP->sHooks.FTell(psSHP->fpSHP);
                if (nFileSize >= UINT_MAX)
                    psSHP->nFileSize = UINT_MAX;
                else
                    psSHP->nFileSize = STATIC_CAST(unsigned int, nFileSize);
            }

            if (psSHP->panRecOffset[hEntity] >= psSHP->nFileSize ||
                /* We should normally use nEntitySize instead of*/
                /* psSHP->panRecSize[hEntity] in the below test, but because of */
                /* the case of non conformant .shx files detailed a bit below, */
                /* let be more tolerant */
                psSHP->panRecSize[hEntity] >
                    psSHP->nFileSize - psSHP->panRecOffset[hEntity])
            {
                char str[128];
                snprintf(str, sizeof(str),
                         "Error in fread() reading object of size %d at offset "
                         "%u from .shp file",
                         nEntitySize, psSHP->panRecOffset[hEntity]);
                str[sizeof(str) - 1] = '\0';

                psSHP->sHooks.Error(str);
                return SHPLIB_NULLPTR;
            }
        }

        unsigned char *pabyRecNew =
            STATIC_CAST(unsigned char *, realloc(psSHP->pabyRec, nNewBufSize));
        if (pabyRecNew == SHPLIB_NULLPTR)
        {
            char szErrorMsg[160];
            snprintf(szErrorMsg, sizeof(szErrorMsg),
                     "Not enough memory to allocate requested memory "
                     "(nNewBufSize=%d). "
                     "Probably broken SHP file",
                     nNewBufSize);
            szErrorMsg[sizeof(szErrorMsg) - 1] = '\0';
            psSHP->sHooks.Error(szErrorMsg);
            return SHPLIB_NULLPTR;
        }

        /* Only set new buffer size after successful alloc */
        psSHP->pabyRec = pabyRecNew;
        psSHP->nBufSize = nNewBufSize;
    }

    /* In case we were not able to reallocate the buffer on a previous step */
    if (psSHP->pabyRec == SHPLIB_NULLPTR)
    {
        return SHPLIB_NULLPTR;
    }

    /* -------------------------------------------------------------------- */
    /*      Read the record.                                                */
    /* -------------------------------------------------------------------- */
    if (psSHP->sHooks.FSeek(psSHP->fpSHP, psSHP->panRecOffset[hEntity], 0) != 0)
    {
        /*
         * TODO - mloskot: Consider detailed diagnostics of shape file,
         * for example to detect if file is truncated.
         */
        char str[128];
        snprintf(str, sizeof(str),
                 "Error in fseek() reading object from .shp file at offset %u",
                 psSHP->panRecOffset[hEntity]);
        str[sizeof(str) - 1] = '\0';

        psSHP->sHooks.Error(str);
        return SHPLIB_NULLPTR;
    }

    const int nBytesRead = STATIC_CAST(
        int, psSHP->sHooks.FRead(psSHP->pabyRec, 1, nEntitySize, psSHP->fpSHP));

    /* Special case for a shapefile whose .shx content length field is not equal */
    /* to the content length field of the .shp, which is a violation of "The */
    /* content length stored in the index record is the same as the value stored in the main */
    /* file record header." (http://www.esri.com/library/whitepapers/pdfs/shapefile.pdf, page 24) */
    /* Actually in that case the .shx content length is equal to the .shp content length + */
    /* 4 (16 bit words), representing the 8 bytes of the record header... */
    if (nBytesRead >= 8 && nBytesRead == nEntitySize - 8)
    {
        /* Do a sanity check */
        int nSHPContentLength;
        memcpy(&nSHPContentLength, psSHP->pabyRec + 4, 4);
        if (!bBigEndian)
            SwapWord(4, &(nSHPContentLength));
        if (nSHPContentLength < 0 || nSHPContentLength > INT_MAX / 2 - 4 ||
            2 * nSHPContentLength + 8 != nBytesRead)
        {
            char str[128];
            snprintf(str, sizeof(str),
                     "Sanity check failed when trying to recover from "
                     "inconsistent .shx/.shp with shape %d",
                     hEntity);
            str[sizeof(str) - 1] = '\0';

            psSHP->sHooks.Error(str);
            return SHPLIB_NULLPTR;
        }
    }
    else if (nBytesRead != nEntitySize)
    {
        /*
         * TODO - mloskot: Consider detailed diagnostics of shape file,
         * for example to detect if file is truncated.
         */
        char str[128];
        snprintf(str, sizeof(str),
                 "Error in fread() reading object of size %d at offset %u from "
                 ".shp file",
                 nEntitySize, psSHP->panRecOffset[hEntity]);
        str[sizeof(str) - 1] = '\0';

        psSHP->sHooks.Error(str);
        return SHPLIB_NULLPTR;
    }

    if (8 + 4 > nEntitySize)
    {
        char szErrorMsg[160];
        snprintf(szErrorMsg, sizeof(szErrorMsg),
                 "Corrupted .shp file : shape %d : nEntitySize = %d", hEntity,
                 nEntitySize);
        szErrorMsg[sizeof(szErrorMsg) - 1] = '\0';
        psSHP->sHooks.Error(szErrorMsg);
        return SHPLIB_NULLPTR;
    }
    int nSHPType;
    memcpy(&nSHPType, psSHP->pabyRec + 8, 4);

    if (bBigEndian)
        SwapWord(4, &(nSHPType));

    /* -------------------------------------------------------------------- */
    /*      Allocate and minimally initialize the object.                   */
    /* -------------------------------------------------------------------- */
    SHPObject *psShape;
    if (psSHP->bFastModeReadObject)
    {
        if (psSHP->psCachedObject->bFastModeReadObject)
        {
            psSHP->sHooks.Error("Invalid read pattern in fast read mode. "
                                "SHPDestroyObject() should be called.");
            return SHPLIB_NULLPTR;
        }

        psShape = psSHP->psCachedObject;
        memset(psShape, 0, sizeof(SHPObject));
    }
    else
    {
        psShape = STATIC_CAST(SHPObject *, calloc(1, sizeof(SHPObject)));
    }
    psShape->nShapeId = hEntity;
    psShape->nSHPType = nSHPType;
    psShape->bMeasureIsUsed = FALSE;
    psShape->bFastModeReadObject = psSHP->bFastModeReadObject;

    /* ==================================================================== */
    /*  Extract vertices for a Polygon or Arc.                              */
    /* ==================================================================== */
    if (psShape->nSHPType == SHPT_POLYGON || psShape->nSHPType == SHPT_ARC ||
        psShape->nSHPType == SHPT_POLYGONZ ||
        psShape->nSHPType == SHPT_POLYGONM || psShape->nSHPType == SHPT_ARCZ ||
        psShape->nSHPType == SHPT_ARCM || psShape->nSHPType == SHPT_MULTIPATCH)
    {
        if (40 + 8 + 4 > nEntitySize)
        {
            char szErrorMsg[160];
            snprintf(szErrorMsg, sizeof(szErrorMsg),
                     "Corrupted .shp file : shape %d : nEntitySize = %d",
                     hEntity, nEntitySize);
            szErrorMsg[sizeof(szErrorMsg) - 1] = '\0';
            psSHP->sHooks.Error(szErrorMsg);
            SHPDestroyObject(psShape);
            return SHPLIB_NULLPTR;
        }
        /* -------------------------------------------------------------------- */
        /*      Get the X/Y bounds.                                             */
        /* -------------------------------------------------------------------- */
        memcpy(&(psShape->dfXMin), psSHP->pabyRec + 8 + 4, 8);
        memcpy(&(psShape->dfYMin), psSHP->pabyRec + 8 + 12, 8);
        memcpy(&(psShape->dfXMax), psSHP->pabyRec + 8 + 20, 8);
        memcpy(&(psShape->dfYMax), psSHP->pabyRec + 8 + 28, 8);

        if (bBigEndian)
            SwapWord(8, &(psShape->dfXMin));
        if (bBigEndian)
            SwapWord(8, &(psShape->dfYMin));
        if (bBigEndian)
            SwapWord(8, &(psShape->dfXMax));
        if (bBigEndian)
            SwapWord(8, &(psShape->dfYMax));

        /* -------------------------------------------------------------------- */
        /*      Extract part/point count, and build vertex and part arrays      */
        /*      to proper size.                                                 */
        /* -------------------------------------------------------------------- */
        uint32_t nPoints;
        memcpy(&nPoints, psSHP->pabyRec + 40 + 8, 4);
        uint32_t nParts;
        memcpy(&nParts, psSHP->pabyRec + 36 + 8, 4);

        if (bBigEndian)
            SwapWord(4, &nPoints);
        if (bBigEndian)
            SwapWord(4, &nParts);

        /* nPoints and nParts are unsigned */
        if (/* nPoints < 0 || nParts < 0 || */
            nPoints > 50 * 1000 * 1000 || nParts > 10 * 1000 * 1000)
        {
            char szErrorMsg[160];
            snprintf(szErrorMsg, sizeof(szErrorMsg),
                     "Corrupted .shp file : shape %d, nPoints=%u, nParts=%u.",
                     hEntity, nPoints, nParts);
            szErrorMsg[sizeof(szErrorMsg) - 1] = '\0';
            psSHP->sHooks.Error(szErrorMsg);
            SHPDestroyObject(psShape);
            return SHPLIB_NULLPTR;
        }

        /* With the previous checks on nPoints and nParts, */
        /* we should not overflow here and after */
        /* since 50 M * (16 + 8 + 8) = 1 600 MB */
        int nRequiredSize = 44 + 8 + 4 * nParts + 16 * nPoints;
        if (psShape->nSHPType == SHPT_POLYGONZ ||
            psShape->nSHPType == SHPT_ARCZ ||
            psShape->nSHPType == SHPT_MULTIPATCH)
        {
            nRequiredSize += 16 + 8 * nPoints;
        }
        if (psShape->nSHPType == SHPT_MULTIPATCH)
        {
            nRequiredSize += 4 * nParts;
        }
        if (nRequiredSize > nEntitySize)
        {
            char szErrorMsg[160];
            snprintf(szErrorMsg, sizeof(szErrorMsg),
                     "Corrupted .shp file : shape %d, nPoints=%u, nParts=%u, "
                     "nEntitySize=%d.",
                     hEntity, nPoints, nParts, nEntitySize);
            szErrorMsg[sizeof(szErrorMsg) - 1] = '\0';
            psSHP->sHooks.Error(szErrorMsg);
            SHPDestroyObject(psShape);
            return SHPLIB_NULLPTR;
        }

        unsigned char *pBuffer = SHPLIB_NULLPTR;
        unsigned char **ppBuffer = SHPLIB_NULLPTR;

        if (psShape->bFastModeReadObject)
        {
            const int nObjectBufSize =
                4 * sizeof(double) * nPoints + 2 * sizeof(int) * nParts;
            pBuffer = SHPReallocObjectBufIfNecessary(psSHP, nObjectBufSize);
            ppBuffer = &pBuffer;
        }

        psShape->nVertices = nPoints;
        psShape->padfX = STATIC_CAST(
            double *, SHPAllocBuffer(ppBuffer, sizeof(double) * nPoints));
        psShape->padfY = STATIC_CAST(
            double *, SHPAllocBuffer(ppBuffer, sizeof(double) * nPoints));
        psShape->padfZ = STATIC_CAST(
            double *, SHPAllocBuffer(ppBuffer, sizeof(double) * nPoints));
        psShape->padfM = STATIC_CAST(
            double *, SHPAllocBuffer(ppBuffer, sizeof(double) * nPoints));

        psShape->nParts = nParts;
        psShape->panPartStart =
            STATIC_CAST(int *, SHPAllocBuffer(ppBuffer, nParts * sizeof(int)));
        psShape->panPartType =
            STATIC_CAST(int *, SHPAllocBuffer(ppBuffer, nParts * sizeof(int)));

        if (psShape->padfX == SHPLIB_NULLPTR ||
            psShape->padfY == SHPLIB_NULLPTR ||
            psShape->padfZ == SHPLIB_NULLPTR ||
            psShape->padfM == SHPLIB_NULLPTR ||
            psShape->panPartStart == SHPLIB_NULLPTR ||
            psShape->panPartType == SHPLIB_NULLPTR)
        {
            char szErrorMsg[160];
            snprintf(szErrorMsg, sizeof(szErrorMsg),
                     "Not enough memory to allocate requested memory "
                     "(nPoints=%u, nParts=%u) for shape %d. "
                     "Probably broken SHP file",
                     nPoints, nParts, hEntity);
            szErrorMsg[sizeof(szErrorMsg) - 1] = '\0';
            psSHP->sHooks.Error(szErrorMsg);
            SHPDestroyObject(psShape);
            return SHPLIB_NULLPTR;
        }

        for (int i = 0; STATIC_CAST(uint32_t, i) < nParts; i++)
            psShape->panPartType[i] = SHPP_RING;

        /* -------------------------------------------------------------------- */
        /*      Copy out the part array from the record.                        */
        /* -------------------------------------------------------------------- */
        memcpy(psShape->panPartStart, psSHP->pabyRec + 44 + 8, 4 * nParts);
        for (int i = 0; STATIC_CAST(uint32_t, i) < nParts; i++)
        {
            if (bBigEndian)
                SwapWord(4, psShape->panPartStart + i);

            /* We check that the offset is inside the vertex array */
            if (psShape->panPartStart[i] < 0 ||
                (psShape->panPartStart[i] >= psShape->nVertices &&
                 psShape->nVertices > 0) ||
                (psShape->panPartStart[i] > 0 && psShape->nVertices == 0))
            {
                char szErrorMsg[160];
                snprintf(szErrorMsg, sizeof(szErrorMsg),
                         "Corrupted .shp file : shape %d : panPartStart[%d] = "
                         "%d, nVertices = %d",
                         hEntity, i, psShape->panPartStart[i],
                         psShape->nVertices);
                szErrorMsg[sizeof(szErrorMsg) - 1] = '\0';
                psSHP->sHooks.Error(szErrorMsg);
                SHPDestroyObject(psShape);
                return SHPLIB_NULLPTR;
            }
            if (i > 0 &&
                psShape->panPartStart[i] <= psShape->panPartStart[i - 1])
            {
                char szErrorMsg[160];
                snprintf(szErrorMsg, sizeof(szErrorMsg),
                         "Corrupted .shp file : shape %d : panPartStart[%d] = "
                         "%d, panPartStart[%d] = %d",
                         hEntity, i, psShape->panPartStart[i], i - 1,
                         psShape->panPartStart[i - 1]);
                szErrorMsg[sizeof(szErrorMsg) - 1] = '\0';
                psSHP->sHooks.Error(szErrorMsg);
                SHPDestroyObject(psShape);
                return SHPLIB_NULLPTR;
            }
        }

        int nOffset = 44 + 8 + 4 * nParts;

        /* -------------------------------------------------------------------- */
        /*      If this is a multipatch, we will also have parts types.         */
        /* -------------------------------------------------------------------- */
        if (psShape->nSHPType == SHPT_MULTIPATCH)
        {
            memcpy(psShape->panPartType, psSHP->pabyRec + nOffset, 4 * nParts);
            for (int i = 0; STATIC_CAST(uint32_t, i) < nParts; i++)
            {
                if (bBigEndian)
                    SwapWord(4, psShape->panPartType + i);
            }

            nOffset += 4 * nParts;
        }

        /* -------------------------------------------------------------------- */
        /*      Copy out the vertices from the record.                          */
        /* -------------------------------------------------------------------- */
        for (int i = 0; STATIC_CAST(uint32_t, i) < nPoints; i++)
        {
            memcpy(psShape->padfX + i, psSHP->pabyRec + nOffset + i * 16, 8);

            memcpy(psShape->padfY + i, psSHP->pabyRec + nOffset + i * 16 + 8,
                   8);

            if (bBigEndian)
                SwapWord(8, psShape->padfX + i);
            if (bBigEndian)
                SwapWord(8, psShape->padfY + i);
        }

        nOffset += 16 * nPoints;

        /* -------------------------------------------------------------------- */
        /*      If we have a Z coordinate, collect that now.                    */
        /* -------------------------------------------------------------------- */
        if (psShape->nSHPType == SHPT_POLYGONZ ||
            psShape->nSHPType == SHPT_ARCZ ||
            psShape->nSHPType == SHPT_MULTIPATCH)
        {
            memcpy(&(psShape->dfZMin), psSHP->pabyRec + nOffset, 8);
            memcpy(&(psShape->dfZMax), psSHP->pabyRec + nOffset + 8, 8);

            if (bBigEndian)
                SwapWord(8, &(psShape->dfZMin));
            if (bBigEndian)
                SwapWord(8, &(psShape->dfZMax));

            for (int i = 0; STATIC_CAST(uint32_t, i) < nPoints; i++)
            {
                memcpy(psShape->padfZ + i,
                       psSHP->pabyRec + nOffset + 16 + i * 8, 8);
                if (bBigEndian)
                    SwapWord(8, psShape->padfZ + i);
            }

            nOffset += 16 + 8 * nPoints;
        }
        else if (psShape->bFastModeReadObject)
        {
            psShape->padfZ = SHPLIB_NULLPTR;
        }

        /* -------------------------------------------------------------------- */
        /*      If we have a M measure value, then read it now.  We assume      */
        /*      that the measure can be present for any shape if the size is    */
        /*      big enough, but really it will only occur for the Z shapes      */
        /*      (options), and the M shapes.                                    */
        /* -------------------------------------------------------------------- */
        if (nEntitySize >= STATIC_CAST(int, nOffset + 16 + 8 * nPoints))
        {
            memcpy(&(psShape->dfMMin), psSHP->pabyRec + nOffset, 8);
            memcpy(&(psShape->dfMMax), psSHP->pabyRec + nOffset + 8, 8);

            if (bBigEndian)
                SwapWord(8, &(psShape->dfMMin));
            if (bBigEndian)
                SwapWord(8, &(psShape->dfMMax));

            for (int i = 0; STATIC_CAST(uint32_t, i) < nPoints; i++)
            {
                memcpy(psShape->padfM + i,
                       psSHP->pabyRec + nOffset + 16 + i * 8, 8);
                if (bBigEndian)
                    SwapWord(8, psShape->padfM + i);
            }
            psShape->bMeasureIsUsed = TRUE;
        }
        else if (psShape->bFastModeReadObject)
        {
            psShape->padfM = SHPLIB_NULLPTR;
        }
    }

    /* ==================================================================== */
    /*  Extract vertices for a MultiPoint.                                  */
    /* ==================================================================== */
    else if (psShape->nSHPType == SHPT_MULTIPOINT ||
             psShape->nSHPType == SHPT_MULTIPOINTM ||
             psShape->nSHPType == SHPT_MULTIPOINTZ)
    {
        if (44 + 4 > nEntitySize)
        {
            char szErrorMsg[160];
            snprintf(szErrorMsg, sizeof(szErrorMsg),
                     "Corrupted .shp file : shape %d : nEntitySize = %d",
                     hEntity, nEntitySize);
            szErrorMsg[sizeof(szErrorMsg) - 1] = '\0';
            psSHP->sHooks.Error(szErrorMsg);
            SHPDestroyObject(psShape);
            return SHPLIB_NULLPTR;
        }
        uint32_t nPoints;
        memcpy(&nPoints, psSHP->pabyRec + 44, 4);

        if (bBigEndian)
            SwapWord(4, &nPoints);

        /* nPoints is unsigned */
        if (/* nPoints < 0 || */ nPoints > 50 * 1000 * 1000)
        {
            char szErrorMsg[160];
            snprintf(szErrorMsg, sizeof(szErrorMsg),
                     "Corrupted .shp file : shape %d : nPoints = %u", hEntity,
                     nPoints);
            szErrorMsg[sizeof(szErrorMsg) - 1] = '\0';
            psSHP->sHooks.Error(szErrorMsg);
            SHPDestroyObject(psShape);
            return SHPLIB_NULLPTR;
        }

        int nRequiredSize = 48 + nPoints * 16;
        if (psShape->nSHPType == SHPT_MULTIPOINTZ)
        {
            nRequiredSize += 16 + nPoints * 8;
        }
        if (nRequiredSize > nEntitySize)
        {
            char szErrorMsg[160];
            snprintf(szErrorMsg, sizeof(szErrorMsg),
                     "Corrupted .shp file : shape %d : nPoints = %u, "
                     "nEntitySize = %d",
                     hEntity, nPoints, nEntitySize);
            szErrorMsg[sizeof(szErrorMsg) - 1] = '\0';
            psSHP->sHooks.Error(szErrorMsg);
            SHPDestroyObject(psShape);
            return SHPLIB_NULLPTR;
        }

        unsigned char *pBuffer = SHPLIB_NULLPTR;
        unsigned char **ppBuffer = SHPLIB_NULLPTR;

        if (psShape->bFastModeReadObject)
        {
            const int nObjectBufSize = 4 * sizeof(double) * nPoints;
            pBuffer = SHPReallocObjectBufIfNecessary(psSHP, nObjectBufSize);
            ppBuffer = &pBuffer;
        }

        psShape->nVertices = nPoints;

        psShape->padfX = STATIC_CAST(
            double *, SHPAllocBuffer(ppBuffer, sizeof(double) * nPoints));
        psShape->padfY = STATIC_CAST(
            double *, SHPAllocBuffer(ppBuffer, sizeof(double) * nPoints));
        psShape->padfZ = STATIC_CAST(
            double *, SHPAllocBuffer(ppBuffer, sizeof(double) * nPoints));
        psShape->padfM = STATIC_CAST(
            double *, SHPAllocBuffer(ppBuffer, sizeof(double) * nPoints));

        if (psShape->padfX == SHPLIB_NULLPTR ||
            psShape->padfY == SHPLIB_NULLPTR ||
            psShape->padfZ == SHPLIB_NULLPTR ||
            psShape->padfM == SHPLIB_NULLPTR)
        {
            char szErrorMsg[160];
            snprintf(szErrorMsg, sizeof(szErrorMsg),
                     "Not enough memory to allocate requested memory "
                     "(nPoints=%u) for shape %d. "
                     "Probably broken SHP file",
                     nPoints, hEntity);
            szErrorMsg[sizeof(szErrorMsg) - 1] = '\0';
            psSHP->sHooks.Error(szErrorMsg);
            SHPDestroyObject(psShape);
            return SHPLIB_NULLPTR;
        }

        for (int i = 0; STATIC_CAST(uint32_t, i) < nPoints; i++)
        {
            memcpy(psShape->padfX + i, psSHP->pabyRec + 48 + 16 * i, 8);
            memcpy(psShape->padfY + i, psSHP->pabyRec + 48 + 16 * i + 8, 8);

            if (bBigEndian)
                SwapWord(8, psShape->padfX + i);
            if (bBigEndian)
                SwapWord(8, psShape->padfY + i);
        }

        int nOffset = 48 + 16 * nPoints;

        /* -------------------------------------------------------------------- */
        /*      Get the X/Y bounds.                                             */
        /* -------------------------------------------------------------------- */
        memcpy(&(psShape->dfXMin), psSHP->pabyRec + 8 + 4, 8);
        memcpy(&(psShape->dfYMin), psSHP->pabyRec + 8 + 12, 8);
        memcpy(&(psShape->dfXMax), psSHP->pabyRec + 8 + 20, 8);
        memcpy(&(psShape->dfYMax), psSHP->pabyRec + 8 + 28, 8);

        if (bBigEndian)
            SwapWord(8, &(psShape->dfXMin));
        if (bBigEndian)
            SwapWord(8, &(psShape->dfYMin));
        if (bBigEndian)
            SwapWord(8, &(psShape->dfXMax));
        if (bBigEndian)
            SwapWord(8, &(psShape->dfYMax));

        /* -------------------------------------------------------------------- */
        /*      If we have a Z coordinate, collect that now.                    */
        /* -------------------------------------------------------------------- */
        if (psShape->nSHPType == SHPT_MULTIPOINTZ)
        {
            memcpy(&(psShape->dfZMin), psSHP->pabyRec + nOffset, 8);
            memcpy(&(psShape->dfZMax), psSHP->pabyRec + nOffset + 8, 8);

            if (bBigEndian)
                SwapWord(8, &(psShape->dfZMin));
            if (bBigEndian)
                SwapWord(8, &(psShape->dfZMax));

            for (int i = 0; STATIC_CAST(uint32_t, i) < nPoints; i++)
            {
                memcpy(psShape->padfZ + i,
                       psSHP->pabyRec + nOffset + 16 + i * 8, 8);
                if (bBigEndian)
                    SwapWord(8, psShape->padfZ + i);
            }

            nOffset += 16 + 8 * nPoints;
        }
        else if (psShape->bFastModeReadObject)
            psShape->padfZ = SHPLIB_NULLPTR;

        /* -------------------------------------------------------------------- */
        /*      If we have a M measure value, then read it now.  We assume      */
        /*      that the measure can be present for any shape if the size is    */
        /*      big enough, but really it will only occur for the Z shapes      */
        /*      (options), and the M shapes.                                    */
        /* -------------------------------------------------------------------- */
        if (nEntitySize >= STATIC_CAST(int, nOffset + 16 + 8 * nPoints))
        {
            memcpy(&(psShape->dfMMin), psSHP->pabyRec + nOffset, 8);
            memcpy(&(psShape->dfMMax), psSHP->pabyRec + nOffset + 8, 8);

            if (bBigEndian)
                SwapWord(8, &(psShape->dfMMin));
            if (bBigEndian)
                SwapWord(8, &(psShape->dfMMax));

            for (int i = 0; STATIC_CAST(uint32_t, i) < nPoints; i++)
            {
                memcpy(psShape->padfM + i,
                       psSHP->pabyRec + nOffset + 16 + i * 8, 8);
                if (bBigEndian)
                    SwapWord(8, psShape->padfM + i);
            }
            psShape->bMeasureIsUsed = TRUE;
        }
        else if (psShape->bFastModeReadObject)
            psShape->padfM = SHPLIB_NULLPTR;
    }

    /* ==================================================================== */
    /*      Extract vertices for a point.                                   */
    /* ==================================================================== */
    else if (psShape->nSHPType == SHPT_POINT ||
             psShape->nSHPType == SHPT_POINTM ||
             psShape->nSHPType == SHPT_POINTZ)
    {
        psShape->nVertices = 1;
        if (psShape->bFastModeReadObject)
        {
            psShape->padfX = &(psShape->dfXMin);
            psShape->padfY = &(psShape->dfYMin);
            psShape->padfZ = &(psShape->dfZMin);
            psShape->padfM = &(psShape->dfMMin);
            psShape->padfZ[0] = 0.0;
            psShape->padfM[0] = 0.0;
        }
        else
        {
            psShape->padfX = STATIC_CAST(double *, calloc(1, sizeof(double)));
            psShape->padfY = STATIC_CAST(double *, calloc(1, sizeof(double)));
            psShape->padfZ = STATIC_CAST(double *, calloc(1, sizeof(double)));
            psShape->padfM = STATIC_CAST(double *, calloc(1, sizeof(double)));
        }

        if (20 + 8 + ((psShape->nSHPType == SHPT_POINTZ) ? 8 : 0) > nEntitySize)
        {
            char szErrorMsg[160];
            snprintf(szErrorMsg, sizeof(szErrorMsg),
                     "Corrupted .shp file : shape %d : nEntitySize = %d",
                     hEntity, nEntitySize);
            szErrorMsg[sizeof(szErrorMsg) - 1] = '\0';
            psSHP->sHooks.Error(szErrorMsg);
            SHPDestroyObject(psShape);
            return SHPLIB_NULLPTR;
        }
        memcpy(psShape->padfX, psSHP->pabyRec + 12, 8);
        memcpy(psShape->padfY, psSHP->pabyRec + 20, 8);

        if (bBigEndian)
            SwapWord(8, psShape->padfX);
        if (bBigEndian)
            SwapWord(8, psShape->padfY);

        int nOffset = 20 + 8;

        /* -------------------------------------------------------------------- */
        /*      If we have a Z coordinate, collect that now.                    */
        /* -------------------------------------------------------------------- */
        if (psShape->nSHPType == SHPT_POINTZ)
        {
            memcpy(psShape->padfZ, psSHP->pabyRec + nOffset, 8);

            if (bBigEndian)
                SwapWord(8, psShape->padfZ);

            nOffset += 8;
        }

        /* -------------------------------------------------------------------- */
        /*      If we have a M measure value, then read it now.  We assume      */
        /*      that the measure can be present for any shape if the size is    */
        /*      big enough, but really it will only occur for the Z shapes      */
        /*      (options), and the M shapes.                                    */
        /* -------------------------------------------------------------------- */
        if (nEntitySize >= nOffset + 8)
        {
            memcpy(psShape->padfM, psSHP->pabyRec + nOffset, 8);

            if (bBigEndian)
                SwapWord(8, psShape->padfM);
            psShape->bMeasureIsUsed = TRUE;
        }

        /* -------------------------------------------------------------------- */
        /*      Since no extents are supplied in the record, we will apply      */
        /*      them from the single vertex.                                    */
        /* -------------------------------------------------------------------- */
        psShape->dfXMin = psShape->dfXMax = psShape->padfX[0];
        psShape->dfYMin = psShape->dfYMax = psShape->padfY[0];
        psShape->dfZMin = psShape->dfZMax = psShape->padfZ[0];
        psShape->dfMMin = psShape->dfMMax = psShape->padfM[0];
    }

    return (psShape);
}

/************************************************************************/
/*                            SHPTypeName()                             */
/************************************************************************/

const char * SHPTypeName(int nSHPType)
{
    switch (nSHPType)
    {
        case SHPT_NULL:
            return "NullShape";

        case SHPT_POINT:
            return "Point";

        case SHPT_ARC:
            return "Arc";

        case SHPT_POLYGON:
            return "Polygon";

        case SHPT_MULTIPOINT:
            return "MultiPoint";

        case SHPT_POINTZ:
            return "PointZ";

        case SHPT_ARCZ:
            return "ArcZ";

        case SHPT_POLYGONZ:
            return "PolygonZ";

        case SHPT_MULTIPOINTZ:
            return "MultiPointZ";

        case SHPT_POINTM:
            return "PointM";

        case SHPT_ARCM:
            return "ArcM";

        case SHPT_POLYGONM:
            return "PolygonM";

        case SHPT_MULTIPOINTM:
            return "MultiPointM";

        case SHPT_MULTIPATCH:
            return "MultiPatch";

        default:
            return "UnknownShapeType";
    }
}

/************************************************************************/
/*                          SHPPartTypeName()                           */
/************************************************************************/

const char * SHPPartTypeName(int nPartType)
{
    switch (nPartType)
    {
        case SHPP_TRISTRIP:
            return "TriangleStrip";

        case SHPP_TRIFAN:
            return "TriangleFan";

        case SHPP_OUTERRING:
            return "OuterRing";

        case SHPP_INNERRING:
            return "InnerRing";

        case SHPP_FIRSTRING:
            return "FirstRing";

        case SHPP_RING:
            return "Ring";

        default:
            return "UnknownPartType";
    }
}

/************************************************************************/
/*                          SHPDestroyObject()                          */
/************************************************************************/

void  SHPDestroyObject(SHPObject *psShape)
{
    if (psShape == SHPLIB_NULLPTR)
        return;

    if (psShape->bFastModeReadObject)
    {
        psShape->bFastModeReadObject = FALSE;
        return;
    }

    if (psShape->padfX != SHPLIB_NULLPTR)
        free(psShape->padfX);
    if (psShape->padfY != SHPLIB_NULLPTR)
        free(psShape->padfY);
    if (psShape->padfZ != SHPLIB_NULLPTR)
        free(psShape->padfZ);
    if (psShape->padfM != SHPLIB_NULLPTR)
        free(psShape->padfM);

    if (psShape->panPartStart != SHPLIB_NULLPTR)
        free(psShape->panPartStart);
    if (psShape->panPartType != SHPLIB_NULLPTR)
        free(psShape->panPartType);

    free(psShape);
}

/************************************************************************/
/*                       SHPGetPartVertexCount()                        */
/************************************************************************/

 int SHPGetPartVertexCount(const SHPObject *psObject, int iPart)
{
    if (iPart == psObject->nParts - 1)
        return psObject->nVertices - psObject->panPartStart[iPart];
    else
        return psObject->panPartStart[iPart + 1] -
               psObject->panPartStart[iPart];
}

/************************************************************************/
/*                      SHPRewindIsInnerRing()                          */
/************************************************************************/

/* Return -1 in case of ambiguity */
 int SHPRewindIsInnerRing(const SHPObject *psObject, int iOpRing,
                                double dfTestX, double dfTestY,
                                double dfRelativeTolerance, int bSameZ,
                                double dfTestZ)
{
    /* -------------------------------------------------------------------- */
    /*      Determine if this ring is an inner ring or an outer ring        */
    /*      relative to all the other rings.  For now we assume the         */
    /*      first ring is outer and all others are inner, but eventually    */
    /*      we need to fix this to handle multiple island polygons and      */
    /*      unordered sets of rings.                                        */
    /*                                                                      */
    /* -------------------------------------------------------------------- */

    bool bInner = false;
    for (int iCheckRing = 0; iCheckRing < psObject->nParts; iCheckRing++)
    {
        if (iCheckRing == iOpRing)
            continue;

        const int nVertStartCheck = psObject->panPartStart[iCheckRing];
        const int nVertCountCheck = SHPGetPartVertexCount(psObject, iCheckRing);

        /* Ignore rings that don't have the same (constant) Z value as the
         * point. */
        /* As noted in SHPRewindObject(), this is a simplification */
        /* of what we should ideally do. */
        if (!bSameZ)
        {
            int bZTestOK = TRUE;
            for (int iVert = nVertStartCheck + 1;
                 iVert < nVertStartCheck + nVertCountCheck; ++iVert)
            {
                if (psObject->padfZ[iVert] != dfTestZ)
                {
                    bZTestOK = FALSE;
                    break;
                }
            }
            if (!bZTestOK)
                continue;
        }

        for (int iEdge = 0; iEdge < nVertCountCheck; iEdge++)
        {
            int iNext;
            if (iEdge < nVertCountCheck - 1)
                iNext = iEdge + 1;
            else
                iNext = 0;

            const double y0 = psObject->padfY[iEdge + nVertStartCheck];
            const double y1 = psObject->padfY[iNext + nVertStartCheck];
            /* Rule #1:
             * Test whether the edge 'straddles' the horizontal ray from
             * the test point (dfTestY,dfTestY)
             * The rule #1 also excludes edges colinear with the ray.
             */
            if ((y0 < dfTestY && dfTestY <= y1) ||
                (y1 < dfTestY && dfTestY <= y0))
            {
                /* Rule #2:
                 * Test if edge-ray intersection is on the right from the
                 * test point (dfTestY,dfTestY)
                 */
                const double x0 = psObject->padfX[iEdge + nVertStartCheck];
                const double x1 = psObject->padfX[iNext + nVertStartCheck];
                const double intersect_minus_testX =
                    (x0 - dfTestX) + (dfTestY - y0) / (y1 - y0) * (x1 - x0);

                if (fabs(intersect_minus_testX) <=
                    dfRelativeTolerance * fabs(dfTestX))
                {
                    /* Potential shared edge, or slightly overlapping polygons
                     */
                    return -1;
                }
                else if (intersect_minus_testX < 0)
                {
                    bInner = !bInner;
                }
            }
        }
    } /* for iCheckRing */
    return bInner;
}

/************************************************************************/
/*                          SHPRewindObject()                           */
/*                                                                      */
/*      Reset the winding of polygon objects to adhere to the           */
/*      specification.                                                  */
/************************************************************************/

// int  SHPRewindObject(CPL_UNUSED SHPHandle hSHP, SHPObject *psObject)
// {
//     /* -------------------------------------------------------------------- */
//     /*      Do nothing if this is not a polygon object.                     */
//     /* -------------------------------------------------------------------- */
//     if (psObject->nSHPType != SHPT_POLYGON &&
//         psObject->nSHPType != SHPT_POLYGONZ &&
//         psObject->nSHPType != SHPT_POLYGONM)
//         return 0;

//     if (psObject->nVertices == 0 || psObject->nParts == 0)
//         return 0;

//     /* -------------------------------------------------------------------- */
//     /*      Test if all points have the same Z value.                       */
//     /* -------------------------------------------------------------------- */
//     int bSameZ = TRUE;
//     if (psObject->nSHPType == SHPT_POLYGONZ ||
//         psObject->nSHPType == SHPT_POLYGONM)
//     {
//         for (int iVert = 1; iVert < psObject->nVertices; ++iVert)
//         {
//             if (psObject->padfZ[iVert] != psObject->padfZ[0])
//             {
//                 bSameZ = FALSE;
//                 break;
//             }
//         }
//     }

//     /* -------------------------------------------------------------------- */
//     /*      Process each of the rings.                                      */
//     /* -------------------------------------------------------------------- */
//     int bAltered = 0;
//     for (int iOpRing = 0; iOpRing < psObject->nParts; iOpRing++)
//     {
//         const int nVertStart = psObject->panPartStart[iOpRing];
//         const int nVertCount = SHPGetPartVertexCount(psObject, iOpRing);

//         if (nVertCount < 2)
//             continue;

//         /* If a ring has a non-constant Z value, then consider it as an outer */
//         /* ring. */
//         /* NOTE: this is a rough approximation. If we were smarter, */
//         /* we would check that all points of the ring are coplanar, and compare
//          */
//         /* that to other rings in the same (oblique) plane. */
//         int bDoIsInnerRingTest = TRUE;
//         if (!bSameZ)
//         {
//             int bPartSameZ = TRUE;
//             for (int iVert = nVertStart + 1; iVert < nVertStart + nVertCount;
//                  ++iVert)
//             {
//                 if (psObject->padfZ[iVert] != psObject->padfZ[nVertStart])
//                 {
//                     bPartSameZ = FALSE;
//                     break;
//                 }
//             }
//             if (!bPartSameZ)
//                 bDoIsInnerRingTest = FALSE;
//         }

//         int bInner = FALSE;
//         if (bDoIsInnerRingTest)
//         {
//             for (int iTolerance = 0; iTolerance < 2; iTolerance++)
//             {
//                 /* In a first attempt, use a relaxed criterion to decide if a
//                  * point */
//                 /* is inside another ring. If all points of the current ring are
//                  * in the */
//                 /* "grey" zone w.r.t that criterion, which seems really
//                  * unlikely, */
//                 /* then use the strict criterion for another pass. */
//                 const double dfRelativeTolerance = (iTolerance == 0) ? 1e-9 : 0;
//                 for (int iVert = nVertStart;
//                      iVert + 1 < nVertStart + nVertCount; ++iVert)
//                 {
//                     /* Use point in the middle of segment to avoid testing
//                      * common points of rings.
//                      */
//                     const double dfTestX =
//                         (psObject->padfX[iVert] + psObject->padfX[iVert + 1]) /
//                         2;
//                     const double dfTestY =
//                         (psObject->padfY[iVert] + psObject->padfY[iVert + 1]) /
//                         2;
//                     const double dfTestZ =
//                         !bSameZ ? psObject->padfZ[nVertStart] : 0;

//                     bInner = SHPRewindIsInnerRing(psObject, iOpRing, dfTestX,
//                                                   dfTestY, dfRelativeTolerance,
//                                                   bSameZ, dfTestZ);
//                     if (bInner >= 0)
//                         break;
//                 }
//                 if (bInner >= 0)
//                     break;
//             }
//             if (bInner < 0)
//             {
//                 /* Completely degenerate case. Do not bother touching order. */
//                 continue;
//             }
//         }

//         /* -------------------------------------------------------------------- */
//         /*      Determine the current order of this ring so we will know if     */
//         /*      it has to be reversed.                                          */
//         /* -------------------------------------------------------------------- */

//         double dfSum = psObject->padfX[nVertStart] *
//                        (psObject->padfY[nVertStart + 1] -
//                         psObject->padfY[nVertStart + nVertCount - 1]);
//         int iVert = nVertStart + 1;
//         for (; iVert < nVertStart + nVertCount - 1; iVert++)
//         {
//             dfSum += psObject->padfX[iVert] *
//                      (psObject->padfY[iVert + 1] - psObject->padfY[iVert - 1]);
//         }

//         dfSum += psObject->padfX[iVert] *
//                  (psObject->padfY[nVertStart] - psObject->padfY[iVert - 1]);

//         /* -------------------------------------------------------------------- */
//         /*      Reverse if necessary.                                           */
//         /* -------------------------------------------------------------------- */
//         if ((dfSum < 0.0 && bInner) || (dfSum > 0.0 && !bInner))
//         {
//             bAltered++;
//             for (int i = 0; i < nVertCount / 2; i++)
//             {
//                 /* Swap X */
//                 double dfSaved = psObject->padfX[nVertStart + i];
//                 psObject->padfX[nVertStart + i] =
//                     psObject->padfX[nVertStart + nVertCount - i - 1];
//                 psObject->padfX[nVertStart + nVertCount - i - 1] = dfSaved;

//                 /* Swap Y */
//                 dfSaved = psObject->padfY[nVertStart + i];
//                 psObject->padfY[nVertStart + i] =
//                     psObject->padfY[nVertStart + nVertCount - i - 1];
//                 psObject->padfY[nVertStart + nVertCount - i - 1] = dfSaved;

//                 /* Swap Z */
//                 if (psObject->padfZ)
//                 {
//                     dfSaved = psObject->padfZ[nVertStart + i];
//                     psObject->padfZ[nVertStart + i] =
//                         psObject->padfZ[nVertStart + nVertCount - i - 1];
//                     psObject->padfZ[nVertStart + nVertCount - i - 1] = dfSaved;
//                 }

//                 /* Swap M */
//                 if (psObject->padfM)
//                 {
//                     dfSaved = psObject->padfM[nVertStart + i];
//                     psObject->padfM[nVertStart + i] =
//                         psObject->padfM[nVertStart + nVertCount - i - 1];
//                     psObject->padfM[nVertStart + nVertCount - i - 1] = dfSaved;
//                 }
//             }
//         }
//     }

//     return bAltered;
// }

 void DBFWriteHeader(DBFHandle psDBF)
{
    unsigned char abyHeader[XBASE_FILEHDR_SZ] = {0};

    if (!psDBF->bNoHeader)
        return;

    psDBF->bNoHeader = FALSE;

    /* -------------------------------------------------------------------- */
    /*      Initialize the file header information.                         */
    /* -------------------------------------------------------------------- */
    abyHeader[0] = 0x03; /* memo field? - just copying */

    /* write out update date */
    abyHeader[1] = STATIC_CAST(unsigned char, psDBF->nUpdateYearSince1900);
    abyHeader[2] = STATIC_CAST(unsigned char, psDBF->nUpdateMonth);
    abyHeader[3] = STATIC_CAST(unsigned char, psDBF->nUpdateDay);

    /* record count preset at zero */

    abyHeader[8] = STATIC_CAST(unsigned char, psDBF->nHeaderLength % 256);
    abyHeader[9] = STATIC_CAST(unsigned char, psDBF->nHeaderLength / 256);

    abyHeader[10] = STATIC_CAST(unsigned char, psDBF->nRecordLength % 256);
    abyHeader[11] = STATIC_CAST(unsigned char, psDBF->nRecordLength / 256);

    abyHeader[29] = STATIC_CAST(unsigned char, psDBF->iLanguageDriver);

    /* -------------------------------------------------------------------- */
    /*      Write the initial 32 byte file header, and all the field        */
    /*      descriptions.                                                   */
    /* -------------------------------------------------------------------- */
    psDBF->sHooks.FSeek(psDBF->fp, 0, 0);
    psDBF->sHooks.FWrite(abyHeader, XBASE_FILEHDR_SZ, 1, psDBF->fp);
    psDBF->sHooks.FWrite(psDBF->pszHeader, XBASE_FLDHDR_SZ, psDBF->nFields,
                         psDBF->fp);

    /* -------------------------------------------------------------------- */
    /*      Write out the newline character if there is room for it.        */
    /* -------------------------------------------------------------------- */
    if (psDBF->nHeaderLength >
        XBASE_FLDHDR_SZ * psDBF->nFields + XBASE_FLDHDR_SZ)
    {
        char cNewline = HEADER_RECORD_TERMINATOR;
        psDBF->sHooks.FWrite(&cNewline, 1, 1, psDBF->fp);
    }

    /* -------------------------------------------------------------------- */
    /*      If the file is new, add a EOF character.                        */
    /* -------------------------------------------------------------------- */
    if (psDBF->nRecords == 0 && psDBF->bWriteEndOfFileChar)
    {
        char ch = END_OF_FILE_CHARACTER;

        psDBF->sHooks.FWrite(&ch, 1, 1, psDBF->fp);
    }
}

static bool DBFFlushRecord(DBFHandle psDBF)
{
    if (psDBF->bCurrentRecordModified && psDBF->nCurrentRecord > -1)
    {
        psDBF->bCurrentRecordModified = FALSE;

        const SAOffset nRecordOffset =
            psDBF->nRecordLength *
                STATIC_CAST(SAOffset, psDBF->nCurrentRecord) +
            psDBF->nHeaderLength;

        /* -------------------------------------------------------------------- */
        /*      Guard FSeek with check for whether we're already at position;   */
        /*      no-op FSeeks defeat network filesystems' write buffering.       */
        /* -------------------------------------------------------------------- */
        if (psDBF->bRequireNextWriteSeek ||
            psDBF->sHooks.FTell(psDBF->fp) != nRecordOffset)
        {
            if (psDBF->sHooks.FSeek(psDBF->fp, nRecordOffset, 0) != 0)
            {
                char szMessage[128];
                snprintf(
                    szMessage, sizeof(szMessage),
                    "Failure seeking to position before writing DBF record %d.",
                    psDBF->nCurrentRecord);
                psDBF->sHooks.Error(szMessage);
                return false;
            }
        }

        if (psDBF->sHooks.FWrite(psDBF->pszCurrentRecord, psDBF->nRecordLength,
                                 1, psDBF->fp) != 1)
        {
            char szMessage[128];
            snprintf(szMessage, sizeof(szMessage),
                     "Failure writing DBF record %d.", psDBF->nCurrentRecord);
            psDBF->sHooks.Error(szMessage);
            return false;
        }

        /* -------------------------------------------------------------------- */
        /*      If next op is also a write, allow possible skipping of FSeek.   */
        /* -------------------------------------------------------------------- */
        psDBF->bRequireNextWriteSeek = FALSE;

        if (psDBF->nCurrentRecord == psDBF->nRecords - 1)
        {
            if (psDBF->bWriteEndOfFileChar)
            {
                char ch = END_OF_FILE_CHARACTER;
                psDBF->sHooks.FWrite(&ch, 1, 1, psDBF->fp);
            }
        }
    }

    return true;
}

 bool DBFLoadRecord(DBFHandle psDBF, int iRecord)
{
    if (psDBF->nCurrentRecord != iRecord)
    {
        if (!DBFFlushRecord(psDBF))
            return false;

        const SAOffset nRecordOffset =
            psDBF->nRecordLength * STATIC_CAST(SAOffset, iRecord) +
            psDBF->nHeaderLength;

        if (psDBF->sHooks.FSeek(psDBF->fp, nRecordOffset, SEEK_SET) != 0)
        {
            char szMessage[128];
            snprintf(szMessage, sizeof(szMessage),
                     "fseek(%ld) failed on DBF file.",
                     STATIC_CAST(long, nRecordOffset));
            psDBF->sHooks.Error(szMessage);
            return false;
        }

        if (psDBF->sHooks.FRead(psDBF->pszCurrentRecord, psDBF->nRecordLength,
                                1, psDBF->fp) != 1)
        {
            char szMessage[128];
            snprintf(szMessage, sizeof(szMessage),
                     "fread(%d) failed on DBF file.", psDBF->nRecordLength);
            psDBF->sHooks.Error(szMessage);
            return false;
        }

        psDBF->nCurrentRecord = iRecord;
        /* -------------------------------------------------------------------- */
        /*      Require a seek for next write in case of mixed R/W operations.  */
        /* -------------------------------------------------------------------- */
        psDBF->bRequireNextWriteSeek = TRUE;
    }

    return true;
}

void  DBFUpdateHeader(DBFHandle psDBF)
{
    if (psDBF->bNoHeader)
        DBFWriteHeader(psDBF);

    if (!DBFFlushRecord(psDBF))
        return;

    psDBF->sHooks.FSeek(psDBF->fp, 0, 0);

    unsigned char abyFileHeader[XBASE_FILEHDR_SZ] = {0};
    psDBF->sHooks.FRead(abyFileHeader, 1, sizeof(abyFileHeader), psDBF->fp);

    abyFileHeader[1] = STATIC_CAST(unsigned char, psDBF->nUpdateYearSince1900);
    abyFileHeader[2] = STATIC_CAST(unsigned char, psDBF->nUpdateMonth);
    abyFileHeader[3] = STATIC_CAST(unsigned char, psDBF->nUpdateDay);
    abyFileHeader[4] = STATIC_CAST(unsigned char, psDBF->nRecords & 0xFF);
    abyFileHeader[5] =
        STATIC_CAST(unsigned char, (psDBF->nRecords >> 8) & 0xFF);
    abyFileHeader[6] =
        STATIC_CAST(unsigned char, (psDBF->nRecords >> 16) & 0xFF);
    abyFileHeader[7] =
        STATIC_CAST(unsigned char, (psDBF->nRecords >> 24) & 0xFF);

    psDBF->sHooks.FSeek(psDBF->fp, 0, 0);
    psDBF->sHooks.FWrite(abyFileHeader, sizeof(abyFileHeader), 1, psDBF->fp);

    psDBF->sHooks.FFlush(psDBF->fp);
}

void  DBFSetLastModifiedDate(DBFHandle psDBF, int nYYSince1900,
                                        int nMM, int nDD)
{
    psDBF->nUpdateYearSince1900 = nYYSince1900;
    psDBF->nUpdateMonth = nMM;
    psDBF->nUpdateDay = nDD;
}

DBFHandle  DBFOpen(const char *pszFilename, const char *pszAccess)

{
    SAHooks sHooks;

    SASetupDefaultHooks(&sHooks);

    return DBFOpenLL(pszFilename, pszAccess, &sHooks);
}

 int DBFGetLenWithoutExtension(const char *pszBasename)
{
    const int nLen = STATIC_CAST(int, strlen(pszBasename));
    for (int i = nLen - 1;
         i > 0 && pszBasename[i] != '/' && pszBasename[i] != '\\'; i--)
    {
        if (pszBasename[i] == '.')
        {
            return i;
        }
    }
    return nLen;
}

DBFHandle  DBFOpenLL(const char *pszFilename, const char *pszAccess,
                                const SAHooks *psHooks)
{
    /* -------------------------------------------------------------------- */
    /*      We only allow the access strings "rb" and "r+".                  */
    /* -------------------------------------------------------------------- */
    if (strcmp(pszAccess, "r") != 0 && strcmp(pszAccess, "r+") != 0 &&
        strcmp(pszAccess, "rb") != 0 && strcmp(pszAccess, "rb+") != 0 &&
        strcmp(pszAccess, "r+b") != 0)
        return SHPLIB_NULLPTR;

    if (strcmp(pszAccess, "r") == 0)
        pszAccess = "rb";

    if (strcmp(pszAccess, "r+") == 0)
        pszAccess = "rb+";

    /* -------------------------------------------------------------------- */
    /*      Compute the base (layer) name.  If there is any extension       */
    /*      on the passed in filename we will strip it off.                 */
    /* -------------------------------------------------------------------- */
    const int nLenWithoutExtension = DBFGetLenWithoutExtension(pszFilename);
    char *pszFullname = STATIC_CAST(char *, malloc(nLenWithoutExtension + 5));
    memcpy(pszFullname, pszFilename, nLenWithoutExtension);
    memcpy(pszFullname + nLenWithoutExtension, ".dbf", 5);

    DBFHandle psDBF = STATIC_CAST(DBFHandle, calloc(1, sizeof(DBFInfo)));
    psDBF->fp = psHooks->FOpen(pszFullname, pszAccess, psHooks->pvUserData);
    memcpy(&(psDBF->sHooks), psHooks, sizeof(SAHooks));

    if (psDBF->fp == SHPLIB_NULLPTR)
    {
        memcpy(pszFullname + nLenWithoutExtension, ".DBF", 5);
        psDBF->fp =
            psDBF->sHooks.FOpen(pszFullname, pszAccess, psHooks->pvUserData);
    }

    memcpy(pszFullname + nLenWithoutExtension, ".cpg", 5);
    SAFile pfCPG = psHooks->FOpen(pszFullname, "r", psHooks->pvUserData);
    if (pfCPG == SHPLIB_NULLPTR)
    {
        memcpy(pszFullname + nLenWithoutExtension, ".CPG", 5);
        pfCPG = psHooks->FOpen(pszFullname, "r", psHooks->pvUserData);
    }

    free(pszFullname);

    if (psDBF->fp == SHPLIB_NULLPTR)
    {
        free(psDBF);
        if (pfCPG)
            psHooks->FClose(pfCPG);
        return SHPLIB_NULLPTR;
    }

    psDBF->bNoHeader = FALSE;
    psDBF->nCurrentRecord = -1;
    psDBF->bCurrentRecordModified = FALSE;

    /* -------------------------------------------------------------------- */
    /*  Read Table Header info                                              */
    /* -------------------------------------------------------------------- */
    const int nBufSize = 500;
    unsigned char *pabyBuf = STATIC_CAST(unsigned char *, malloc(nBufSize));
    if (psDBF->sHooks.FRead(pabyBuf, XBASE_FILEHDR_SZ, 1, psDBF->fp) != 1)
    {
        psDBF->sHooks.FClose(psDBF->fp);
        if (pfCPG)
            psDBF->sHooks.FClose(pfCPG);
        free(pabyBuf);
        free(psDBF);
        return SHPLIB_NULLPTR;
    }

    DBFSetLastModifiedDate(psDBF, pabyBuf[1], pabyBuf[2], pabyBuf[3]);

    psDBF->nRecords = pabyBuf[4] | (pabyBuf[5] << 8) | (pabyBuf[6] << 16) |
                      ((pabyBuf[7] & 0x7f) << 24);

    const int nHeadLen = pabyBuf[8] | (pabyBuf[9] << 8);
    psDBF->nHeaderLength = nHeadLen;
    psDBF->nRecordLength = pabyBuf[10] | (pabyBuf[11] << 8);
    psDBF->iLanguageDriver = pabyBuf[29];

    if (psDBF->nRecordLength == 0 || nHeadLen < XBASE_FILEHDR_SZ)
    {
        psDBF->sHooks.FClose(psDBF->fp);
        if (pfCPG)
            psDBF->sHooks.FClose(pfCPG);
        free(pabyBuf);
        free(psDBF);
        return SHPLIB_NULLPTR;
    }

    const int nFields = (nHeadLen - XBASE_FILEHDR_SZ) / XBASE_FLDHDR_SZ;
    psDBF->nFields = nFields;

    /* coverity[tainted_data] */
    psDBF->pszCurrentRecord = STATIC_CAST(char *, malloc(psDBF->nRecordLength));

    /* -------------------------------------------------------------------- */
    /*  Figure out the code page from the LDID and CPG                      */
    /* -------------------------------------------------------------------- */
    psDBF->pszCodePage = SHPLIB_NULLPTR;
    if (pfCPG)
    {
        memset(pabyBuf, 0, nBufSize);
        psDBF->sHooks.FRead(pabyBuf, 1, nBufSize - 1, pfCPG);
        const size_t n = strcspn(REINTERPRET_CAST(char *, pabyBuf), "\n\r");
        if (n > 0)
        {
            pabyBuf[n] = '\0';
            psDBF->pszCodePage = STATIC_CAST(char *, malloc(n + 1));
            memcpy(psDBF->pszCodePage, pabyBuf, n + 1);
        }
        psDBF->sHooks.FClose(pfCPG);
    }
    if (psDBF->pszCodePage == SHPLIB_NULLPTR && pabyBuf[29] != 0)
    {
        snprintf(REINTERPRET_CAST(char *, pabyBuf), nBufSize, "LDID/%d",
                 psDBF->iLanguageDriver);
        psDBF->pszCodePage = STATIC_CAST(
            char *, malloc(strlen(REINTERPRET_CAST(char *, pabyBuf)) + 1));
        strcpy(psDBF->pszCodePage, REINTERPRET_CAST(char *, pabyBuf));
    }

    /* -------------------------------------------------------------------- */
    /*  Read in Field Definitions                                           */
    /* -------------------------------------------------------------------- */
    pabyBuf = STATIC_CAST(unsigned char *, realloc(pabyBuf, nHeadLen));
    psDBF->pszHeader = REINTERPRET_CAST(char *, pabyBuf);

    psDBF->sHooks.FSeek(psDBF->fp, XBASE_FILEHDR_SZ, 0);
    if (psDBF->sHooks.FRead(pabyBuf, nHeadLen - XBASE_FILEHDR_SZ, 1,
                            psDBF->fp) != 1)
    {
        psDBF->sHooks.FClose(psDBF->fp);
        free(pabyBuf);
        free(psDBF->pszCurrentRecord);
        free(psDBF->pszCodePage);
        free(psDBF);
        return SHPLIB_NULLPTR;
    }

    psDBF->panFieldOffset = STATIC_CAST(int *, malloc(sizeof(int) * nFields));
    psDBF->panFieldSize = STATIC_CAST(int *, malloc(sizeof(int) * nFields));
    psDBF->panFieldDecimals = STATIC_CAST(int *, malloc(sizeof(int) * nFields));
    psDBF->pachFieldType = STATIC_CAST(char *, malloc(sizeof(char) * nFields));

    for (int iField = 0; iField < nFields; iField++)
    {
        unsigned char *pabyFInfo = pabyBuf + iField * XBASE_FLDHDR_SZ;
        if (pabyFInfo[0] == HEADER_RECORD_TERMINATOR)
        {
            psDBF->nFields = iField;
            break;
        }

        if (pabyFInfo[11] == 'N' || pabyFInfo[11] == 'F')
        {
            psDBF->panFieldSize[iField] = pabyFInfo[16];
            psDBF->panFieldDecimals[iField] = pabyFInfo[17];
        }
        else
        {
            psDBF->panFieldSize[iField] = pabyFInfo[16];
            psDBF->panFieldDecimals[iField] = 0;

            /*
            ** The following seemed to be used sometimes to handle files with
            long
            ** string fields, but in other cases (such as bug 1202) the decimals
            field
            ** just seems to indicate some sort of preferred formatting, not
            very
            ** wide fields.  So I have disabled this code.  FrankW.
                    psDBF->panFieldSize[iField] = pabyFInfo[16] +
            pabyFInfo[17]*256; psDBF->panFieldDecimals[iField] = 0;
            */
        }

        psDBF->pachFieldType[iField] = STATIC_CAST(char, pabyFInfo[11]);
        if (iField == 0)
            psDBF->panFieldOffset[iField] = 1;
        else
            psDBF->panFieldOffset[iField] = psDBF->panFieldOffset[iField - 1] +
                                            psDBF->panFieldSize[iField - 1];
    }

    /* Check that the total width of fields does not exceed the record width */
    if (psDBF->nFields > 0 && psDBF->panFieldOffset[psDBF->nFields - 1] +
                                      psDBF->panFieldSize[psDBF->nFields - 1] >
                                  psDBF->nRecordLength)
    {
        DBFClose(psDBF);
        return SHPLIB_NULLPTR;
    }

    DBFSetWriteEndOfFileChar(psDBF, TRUE);

    psDBF->bRequireNextWriteSeek = TRUE;

    return (psDBF);
}

void  DBFClose(DBFHandle psDBF)
{
    if (psDBF == SHPLIB_NULLPTR)
        return;

    /* -------------------------------------------------------------------- */
    /*      Write out header if not already written.                        */
    /* -------------------------------------------------------------------- */
    if (psDBF->bNoHeader)
        DBFWriteHeader(psDBF);

    (DBFFlushRecord(psDBF));

    /* -------------------------------------------------------------------- */
    /*      Update last access date, and number of records if we have       */
    /*      write access.                                                   */
    /* -------------------------------------------------------------------- */
    if (psDBF->bUpdated)
        DBFUpdateHeader(psDBF);

    /* -------------------------------------------------------------------- */
    /*      Close, and free resources.                                      */
    /* -------------------------------------------------------------------- */
    psDBF->sHooks.FClose(psDBF->fp);

    if (psDBF->panFieldOffset != SHPLIB_NULLPTR)
    {
        free(psDBF->panFieldOffset);
        free(psDBF->panFieldSize);
        free(psDBF->panFieldDecimals);
        free(psDBF->pachFieldType);
    }

    if (psDBF->pszWorkField != SHPLIB_NULLPTR)
        free(psDBF->pszWorkField);

    free(psDBF->pszHeader);
    free(psDBF->pszCurrentRecord);
    free(psDBF->pszCodePage);

    free(psDBF);
}

DBFHandle  DBFCreate(const char *pszFilename)
{
    return DBFCreateEx(pszFilename, "LDID/87");  // 0x57
}

DBFHandle  DBFCreateEx(const char *pszFilename,
                                  const char *pszCodePage)
{
    SAHooks sHooks;

    SASetupDefaultHooks(&sHooks);

    return DBFCreateLL(pszFilename, pszCodePage, &sHooks);
}

DBFHandle  DBFCreateLL(const char *pszFilename,
                                  const char *pszCodePage,
                                  const SAHooks *psHooks)
{
    /* -------------------------------------------------------------------- */
    /*      Compute the base (layer) name.  If there is any extension       */
    /*      on the passed in filename we will strip it off.                 */
    /* -------------------------------------------------------------------- */
    const int nLenWithoutExtension = DBFGetLenWithoutExtension(pszFilename);
    char *pszFullname = STATIC_CAST(char *, malloc(nLenWithoutExtension + 5));
    memcpy(pszFullname, pszFilename, nLenWithoutExtension);
    memcpy(pszFullname + nLenWithoutExtension, ".dbf", 5);

    /* -------------------------------------------------------------------- */
    /*      Create the file.                                                */
    /* -------------------------------------------------------------------- */
    SAFile fp = psHooks->FOpen(pszFullname, "wb+", psHooks->pvUserData);
    if (fp == SHPLIB_NULLPTR)
    {
        free(pszFullname);
        return SHPLIB_NULLPTR;
    }

    memcpy(pszFullname + nLenWithoutExtension, ".cpg", 5);
    int ldid = -1;
    if (pszCodePage != SHPLIB_NULLPTR)
    {
        if (strncmp(pszCodePage, "LDID/", 5) == 0)
        {
            ldid = atoi(pszCodePage + 5);
            if (ldid > 255)
                ldid = -1;  // don't use 0 to indicate out of range as LDID/0 is
                            // a valid one
        }
        if (ldid < 0)
        {
            SAFile fpCPG =
                psHooks->FOpen(pszFullname, "w", psHooks->pvUserData);
            psHooks->FWrite(
                CONST_CAST(void *, STATIC_CAST(const void *, pszCodePage)),
                strlen(pszCodePage), 1, fpCPG);
            psHooks->FClose(fpCPG);
        }
    }
    if (pszCodePage == SHPLIB_NULLPTR || ldid >= 0)
    {
        psHooks->Remove(pszFullname, psHooks->pvUserData);
    }

    free(pszFullname);

    /* -------------------------------------------------------------------- */
    /*      Create the info structure.                                      */
    /* -------------------------------------------------------------------- */
    DBFHandle psDBF = STATIC_CAST(DBFHandle, calloc(1, sizeof(DBFInfo)));

    memcpy(&(psDBF->sHooks), psHooks, sizeof(SAHooks));
    psDBF->fp = fp;
    psDBF->nRecords = 0;
    psDBF->nFields = 0;
    psDBF->nRecordLength = 1;
    psDBF->nHeaderLength =
        XBASE_FILEHDR_SZ + 1; /* + 1 for HEADER_RECORD_TERMINATOR */

    psDBF->panFieldOffset = SHPLIB_NULLPTR;
    psDBF->panFieldSize = SHPLIB_NULLPTR;
    psDBF->panFieldDecimals = SHPLIB_NULLPTR;
    psDBF->pachFieldType = SHPLIB_NULLPTR;
    psDBF->pszHeader = SHPLIB_NULLPTR;

    psDBF->nCurrentRecord = -1;
    psDBF->bCurrentRecordModified = FALSE;
    psDBF->pszCurrentRecord = SHPLIB_NULLPTR;

    psDBF->bNoHeader = TRUE;

    psDBF->iLanguageDriver = ldid > 0 ? ldid : 0;
    psDBF->pszCodePage = SHPLIB_NULLPTR;
    if (pszCodePage)
    {
        psDBF->pszCodePage =
            STATIC_CAST(char *, malloc(strlen(pszCodePage) + 1));
        strcpy(psDBF->pszCodePage, pszCodePage);
    }
    DBFSetLastModifiedDate(psDBF, 95, 7, 26); /* dummy date */

    DBFSetWriteEndOfFileChar(psDBF, TRUE);

    psDBF->bRequireNextWriteSeek = TRUE;

    return (psDBF);
}

int  DBFAddField(DBFHandle psDBF, const char *pszFieldName,
                            DBFFieldType eType, int nWidth, int nDecimals)
{
    char chNativeType;

    if (eType == FTLogical)
        chNativeType = 'L';
    else if (eType == FTDate)
        chNativeType = 'D';
    else if (eType == FTString)
        chNativeType = 'C';
    else
        chNativeType = 'N';

    return DBFAddNativeFieldType(psDBF, pszFieldName, chNativeType, nWidth,
                                 nDecimals);
}

 char DBFGetNullCharacter(char chType)
{
    switch (chType)
    {
        case 'N':
        case 'F':
            return '*';
        case 'D':
            return '0';
        case 'L':
            return '?';
        default:
            return ' ';
    }
}

int  DBFAddNativeFieldType(DBFHandle psDBF, const char *pszFieldName,
                                      char chType, int nWidth, int nDecimals)
{
    /* make sure that everything is written in .dbf */
    if (!DBFFlushRecord(psDBF))
        return -1;

    if (psDBF->nHeaderLength + XBASE_FLDHDR_SZ > 65535)
    {
        char szMessage[128];
        snprintf(szMessage, sizeof(szMessage),
                 "Cannot add field %s. Header length limit reached "
                 "(max 65535 bytes, 2046 fields).",
                 pszFieldName);
        psDBF->sHooks.Error(szMessage);
        return -1;
    }

    /* -------------------------------------------------------------------- */
    /*      Do some checking to ensure we can add records to this file.     */
    /* -------------------------------------------------------------------- */
    if (nWidth < 1)
        return -1;

    if (nWidth > XBASE_FLD_MAX_WIDTH)
        nWidth = XBASE_FLD_MAX_WIDTH;

    if (psDBF->nRecordLength + nWidth > 65535)
    {
        char szMessage[128];
        snprintf(szMessage, sizeof(szMessage),
                 "Cannot add field %s. Record length limit reached "
                 "(max 65535 bytes).",
                 pszFieldName);
        psDBF->sHooks.Error(szMessage);
        return -1;
    }

    const int nOldRecordLength = psDBF->nRecordLength;
    const int nOldHeaderLength = psDBF->nHeaderLength;

    /* -------------------------------------------------------------------- */
    /*      realloc all the arrays larger to hold the additional field      */
    /*      information.                                                    */
    /* -------------------------------------------------------------------- */
    psDBF->nFields++;

    psDBF->panFieldOffset = STATIC_CAST(
        int *, realloc(psDBF->panFieldOffset, sizeof(int) * psDBF->nFields));

    psDBF->panFieldSize = STATIC_CAST(
        int *, realloc(psDBF->panFieldSize, sizeof(int) * psDBF->nFields));

    psDBF->panFieldDecimals = STATIC_CAST(
        int *, realloc(psDBF->panFieldDecimals, sizeof(int) * psDBF->nFields));

    psDBF->pachFieldType = STATIC_CAST(
        char *, realloc(psDBF->pachFieldType, sizeof(char) * psDBF->nFields));

    /* -------------------------------------------------------------------- */
    /*      Assign the new field information fields.                        */
    /* -------------------------------------------------------------------- */
    psDBF->panFieldOffset[psDBF->nFields - 1] = psDBF->nRecordLength;
    psDBF->nRecordLength += nWidth;
    psDBF->panFieldSize[psDBF->nFields - 1] = nWidth;
    psDBF->panFieldDecimals[psDBF->nFields - 1] = nDecimals;
    psDBF->pachFieldType[psDBF->nFields - 1] = chType;

    /* -------------------------------------------------------------------- */
    /*      Extend the required header information.                         */
    /* -------------------------------------------------------------------- */
    psDBF->nHeaderLength += XBASE_FLDHDR_SZ;
    psDBF->bUpdated = FALSE;

    psDBF->pszHeader = STATIC_CAST(
        char *, realloc(psDBF->pszHeader, psDBF->nFields * XBASE_FLDHDR_SZ));

    char *pszFInfo = psDBF->pszHeader + XBASE_FLDHDR_SZ * (psDBF->nFields - 1);

    for (int i = 0; i < XBASE_FLDHDR_SZ; i++)
        pszFInfo[i] = '\0';

    strncpy(pszFInfo, pszFieldName, XBASE_FLDNAME_LEN_WRITE);

    pszFInfo[11] = psDBF->pachFieldType[psDBF->nFields - 1];

    if (chType == 'C')
    {
        pszFInfo[16] = STATIC_CAST(unsigned char, nWidth % 256);
        pszFInfo[17] = STATIC_CAST(unsigned char, nWidth / 256);
    }
    else
    {
        pszFInfo[16] = STATIC_CAST(unsigned char, nWidth);
        pszFInfo[17] = STATIC_CAST(unsigned char, nDecimals);
    }

    /* -------------------------------------------------------------------- */
    /*      Make the current record buffer appropriately larger.            */
    /* -------------------------------------------------------------------- */
    psDBF->pszCurrentRecord = STATIC_CAST(
        char *, realloc(psDBF->pszCurrentRecord, psDBF->nRecordLength));

    /* we're done if dealing with new .dbf */
    if (psDBF->bNoHeader)
        return (psDBF->nFields - 1);

    /* -------------------------------------------------------------------- */
    /*      For existing .dbf file, shift records                           */
    /* -------------------------------------------------------------------- */

    /* alloc record */
    char *pszRecord =
        STATIC_CAST(char *, malloc(sizeof(char) * psDBF->nRecordLength));

    const char chFieldFill = DBFGetNullCharacter(chType);

    SAOffset nRecordOffset;
    for (int i = psDBF->nRecords - 1; i >= 0; --i)
    {
        nRecordOffset =
            nOldRecordLength * STATIC_CAST(SAOffset, i) + nOldHeaderLength;

        /* load record */
        psDBF->sHooks.FSeek(psDBF->fp, nRecordOffset, 0);
        if (psDBF->sHooks.FRead(pszRecord, nOldRecordLength, 1, psDBF->fp) != 1)
        {
            free(pszRecord);
            return -1;
        }

        /* set new field's value to NULL */
        memset(pszRecord + nOldRecordLength, chFieldFill, nWidth);

        nRecordOffset = psDBF->nRecordLength * STATIC_CAST(SAOffset, i) +
                        psDBF->nHeaderLength;

        /* move record to the new place*/
        psDBF->sHooks.FSeek(psDBF->fp, nRecordOffset, 0);
        psDBF->sHooks.FWrite(pszRecord, psDBF->nRecordLength, 1, psDBF->fp);
    }

    if (psDBF->bWriteEndOfFileChar)
    {
        char ch = END_OF_FILE_CHARACTER;

        nRecordOffset =
            psDBF->nRecordLength * STATIC_CAST(SAOffset, psDBF->nRecords) +
            psDBF->nHeaderLength;

        psDBF->sHooks.FSeek(psDBF->fp, nRecordOffset, 0);
        psDBF->sHooks.FWrite(&ch, 1, 1, psDBF->fp);
    }

    /* free record */
    free(pszRecord);

    /* force update of header with new header, record length and new field */
    psDBF->bNoHeader = TRUE;
    DBFUpdateHeader(psDBF);

    psDBF->nCurrentRecord = -1;
    psDBF->bCurrentRecordModified = FALSE;
    psDBF->bUpdated = TRUE;

    return (psDBF->nFields - 1);
}

 void *DBFReadAttribute(DBFHandle psDBF, int hEntity, int iField,
                              char chReqType)
{
    /* -------------------------------------------------------------------- */
    /*      Verify selection.                                               */
    /* -------------------------------------------------------------------- */
    if (hEntity < 0 || hEntity >= psDBF->nRecords)
        return SHPLIB_NULLPTR;

    if (iField < 0 || iField >= psDBF->nFields)
        return SHPLIB_NULLPTR;

    /* -------------------------------------------------------------------- */
    /*     Have we read the record?                                         */
    /* -------------------------------------------------------------------- */
    if (!DBFLoadRecord(psDBF, hEntity))
        return SHPLIB_NULLPTR;

    const unsigned char *pabyRec =
        REINTERPRET_CAST(const unsigned char *, psDBF->pszCurrentRecord);

    /* -------------------------------------------------------------------- */
    /*      Ensure we have room to extract the target field.                */
    /* -------------------------------------------------------------------- */
    if (psDBF->panFieldSize[iField] >= psDBF->nWorkFieldLength)
    {
        psDBF->nWorkFieldLength = psDBF->panFieldSize[iField] + 100;
        if (psDBF->pszWorkField == SHPLIB_NULLPTR)
            psDBF->pszWorkField =
                STATIC_CAST(char *, malloc(psDBF->nWorkFieldLength));
        else
            psDBF->pszWorkField = STATIC_CAST(
                char *, realloc(psDBF->pszWorkField, psDBF->nWorkFieldLength));
    }

    /* -------------------------------------------------------------------- */
    /*      Extract the requested field.                                    */
    /* -------------------------------------------------------------------- */
    memcpy(psDBF->pszWorkField,
           REINTERPRET_CAST(const char *, pabyRec) +
               psDBF->panFieldOffset[iField],
           psDBF->panFieldSize[iField]);
    psDBF->pszWorkField[psDBF->panFieldSize[iField]] = '\0';

    void *pReturnField = psDBF->pszWorkField;

    /* -------------------------------------------------------------------- */
    /*      Decode the field.                                               */
    /* -------------------------------------------------------------------- */
    if (chReqType == 'I')
    {
        psDBF->fieldValue.nIntField = atoi(psDBF->pszWorkField);

        pReturnField = &(psDBF->fieldValue.nIntField);
    }
    else if (chReqType == 'N')
    {
        psDBF->fieldValue.dfDoubleField =
            psDBF->sHooks.Atof(psDBF->pszWorkField);

        pReturnField = &(psDBF->fieldValue.dfDoubleField);
    }

/* -------------------------------------------------------------------- */
/*      Should we trim white space off the string attribute value?      */
/* -------------------------------------------------------------------- */
#ifdef TRIM_DBF_WHITESPACE
    else
    {
        char *pchSrc = psDBF->pszWorkField;
        char *pchDst = pchSrc;

        while (*pchSrc == ' ')
            pchSrc++;

        while (*pchSrc != '\0')
            *(pchDst++) = *(pchSrc++);
        *pchDst = '\0';

        while (pchDst != psDBF->pszWorkField && *(--pchDst) == ' ')
            *pchDst = '\0';
    }
#endif

    return pReturnField;
}

int  DBFReadIntegerAttribute(DBFHandle psDBF, int iRecord,
                                        int iField)
{
    int *pnValue =
        STATIC_CAST(int *, DBFReadAttribute(psDBF, iRecord, iField, 'I'));

    if (pnValue == SHPLIB_NULLPTR)
        return 0;
    else
        return *pnValue;
}

double  DBFReadDoubleAttribute(DBFHandle psDBF, int iRecord,
                                          int iField)
{
    double *pdValue =
        STATIC_CAST(double *, DBFReadAttribute(psDBF, iRecord, iField, 'N'));

    if (pdValue == SHPLIB_NULLPTR)
        return 0.0;
    else
        return *pdValue;
}

const char*
    DBFReadStringAttribute(DBFHandle psDBF, int iRecord, int iField)

{
    return STATIC_CAST(const char *,
                       DBFReadAttribute(psDBF, iRecord, iField, 'C'));
}

const char *
    DBFReadLogicalAttribute(DBFHandle psDBF, int iRecord, int iField)

{
    return STATIC_CAST(const char *,
                       DBFReadAttribute(psDBF, iRecord, iField, 'L'));
}

 bool DBFIsValueNULL(char chType, const char *pszValue)
{
    if (pszValue == SHPLIB_NULLPTR)
        return true;

    switch (chType)
    {
        case 'N':
        case 'F':
            /*
            ** We accept all asterisks or all blanks as NULL
            ** though according to the spec I think it should be all
            ** asterisks.
            */
            if (pszValue[0] == '*')
                return true;

            for (int i = 0; pszValue[i] != '\0'; i++)
            {
                if (pszValue[i] != ' ')
                    return false;
            }
            return true;

        case 'D':
            /* NULL date fields have value "00000000" */
            /* Some DBF files have fields filled with spaces */
            /* (trimmed by DBFReadStringAttribute) to indicate null */
            /* values for dates (#4265). */
            /* And others have '       0': https://lists.osgeo.org/pipermail/gdal-dev/2023-November/058010.html */
            return strncmp(pszValue, "00000000", 8) == 0 ||
                   strcmp(pszValue, " ") == 0 || strcmp(pszValue, "0") == 0;

        case 'L':
            /* NULL boolean fields have value "?" */
            return pszValue[0] == '?';

        default:
            /* empty string fields are considered NULL */
            return strlen(pszValue) == 0;
    }
}

int  DBFIsAttributeNULL(DBFHandle psDBF, int iRecord, int iField)
{
    const char *pszValue = DBFReadStringAttribute(psDBF, iRecord, iField);

    if (pszValue == SHPLIB_NULLPTR)
        return TRUE;

    return DBFIsValueNULL(psDBF->pachFieldType[iField], pszValue);
}

int  DBFGetFieldCount(DBFHandle psDBF)

{
    return (psDBF->nFields);
}

int  DBFGetRecordCount(DBFHandle psDBF)

{
    return (psDBF->nRecords);
}

DBFFieldType  DBFGetFieldInfo(const DBFHandle psDBF, int iField,
                                         char *pszFieldName, int *pnWidth,
                                         int *pnDecimals)
{
    if (iField < 0 || iField >= psDBF->nFields)
        return (FTInvalid);

    if (pnWidth != SHPLIB_NULLPTR)
        *pnWidth = psDBF->panFieldSize[iField];

    if (pnDecimals != SHPLIB_NULLPTR)
        *pnDecimals = psDBF->panFieldDecimals[iField];

    if (pszFieldName != SHPLIB_NULLPTR)
    {
        strncpy(pszFieldName,
                STATIC_CAST(char *, psDBF->pszHeader) +
                    iField * XBASE_FLDHDR_SZ,
                XBASE_FLDNAME_LEN_READ);
        pszFieldName[XBASE_FLDNAME_LEN_READ] = '\0';
        for (int i = XBASE_FLDNAME_LEN_READ - 1;
             i > 0 && pszFieldName[i] == ' '; i--)
            pszFieldName[i] = '\0';
    }

    if (psDBF->pachFieldType[iField] == 'L')
        return (FTLogical);

    else if (psDBF->pachFieldType[iField] == 'D')
        return (FTDate);

    else if (psDBF->pachFieldType[iField] == 'N' ||
             psDBF->pachFieldType[iField] == 'F')
    {
        if (psDBF->panFieldDecimals[iField] > 0 ||
            psDBF->panFieldSize[iField] >= 10)
            return (FTDouble);
        else
            return (FTInteger);
    }
    else
    {
        return (FTString);
    }
}

/************************************************************************/
/*                         DBFWriteAttribute()                          */
/*                                                                      */
/*      Write an attribute record to the file.                          */
/************************************************************************/

 bool DBFWriteAttribute(DBFHandle psDBF, int hEntity, int iField,
                              void *pValue)
{
    /* -------------------------------------------------------------------- */
    /*      Is this a valid record?                                         */
    /* -------------------------------------------------------------------- */
    if (hEntity < 0 || hEntity > psDBF->nRecords)
        return false;

    if (psDBF->bNoHeader)
        DBFWriteHeader(psDBF);

    /* -------------------------------------------------------------------- */
    /*      Is this a brand new record?                                     */
    /* -------------------------------------------------------------------- */
    if (hEntity == psDBF->nRecords)
    {
        if (!DBFFlushRecord(psDBF))
            return false;

        psDBF->nRecords++;
        for (int i = 0; i < psDBF->nRecordLength; i++)
            psDBF->pszCurrentRecord[i] = ' ';

        psDBF->nCurrentRecord = hEntity;
    }

    /* -------------------------------------------------------------------- */
    /*      Is this an existing record, but different than the last one     */
    /*      we accessed?                                                    */
    /* -------------------------------------------------------------------- */
    if (!DBFLoadRecord(psDBF, hEntity))
        return false;

    unsigned char *pabyRec =
        REINTERPRET_CAST(unsigned char *, psDBF->pszCurrentRecord);

    psDBF->bCurrentRecordModified = TRUE;
    psDBF->bUpdated = TRUE;

    /* -------------------------------------------------------------------- */
    /*      Translate NULL value to valid DBF file representation.          */
    /*                                                                      */
    /*      Contributed by Jim Matthews.                                    */
    /* -------------------------------------------------------------------- */
    if (pValue == SHPLIB_NULLPTR)
    {
        memset(pabyRec + psDBF->panFieldOffset[iField],
               DBFGetNullCharacter(psDBF->pachFieldType[iField]),
               psDBF->panFieldSize[iField]);
        return true;
    }

    /* -------------------------------------------------------------------- */
    /*      Assign all the record fields.                                   */
    /* -------------------------------------------------------------------- */
    bool nRetResult = true;

    switch (psDBF->pachFieldType[iField])
    {
        case 'D':
        case 'N':
        case 'F':
        {
            int nWidth = psDBF->panFieldSize[iField];

            char szSField[XBASE_FLD_MAX_WIDTH + 1];
            if (STATIC_CAST(int, sizeof(szSField)) - 2 < nWidth)
                nWidth = sizeof(szSField) - 2;

            char szFormat[20];
            snprintf(szFormat, sizeof(szFormat), "%%%d.%df", nWidth,
                     psDBF->panFieldDecimals[iField]);
            CPLsnprintf(szSField, sizeof(szSField), szFormat,
                        *STATIC_CAST(double *, pValue));
            szSField[sizeof(szSField) - 1] = '\0';
            if (STATIC_CAST(int, strlen(szSField)) >
                psDBF->panFieldSize[iField])
            {
                szSField[psDBF->panFieldSize[iField]] = '\0';
                nRetResult = false;
            }
            memcpy(REINTERPRET_CAST(char *,
                                    pabyRec + psDBF->panFieldOffset[iField]),
                   szSField, strlen(szSField));
            break;
        }

        case 'L':
            if (psDBF->panFieldSize[iField] >= 1 &&
                (*STATIC_CAST(char *, pValue) == 'F' ||
                 *STATIC_CAST(char *, pValue) == 'T'))
            {
                *(pabyRec + psDBF->panFieldOffset[iField]) =
                    *STATIC_CAST(char *, pValue);
            }
            else
            {
                nRetResult = false;
            }
            break;

        default:
        {
            int j;
            if (STATIC_CAST(int, strlen(STATIC_CAST(char *, pValue))) >
                psDBF->panFieldSize[iField])
            {
                j = psDBF->panFieldSize[iField];
                nRetResult = false;
            }
            else
            {
                memset(pabyRec + psDBF->panFieldOffset[iField], ' ',
                       psDBF->panFieldSize[iField]);
                j = STATIC_CAST(int, strlen(STATIC_CAST(char *, pValue)));
            }

            strncpy(REINTERPRET_CAST(char *,
                                     pabyRec + psDBF->panFieldOffset[iField]),
                    STATIC_CAST(const char *, pValue), j);
            break;
        }
    }

    return nRetResult;
}

/************************************************************************/
/*                     DBFWriteAttributeDirectly()                      */
/*                                                                      */
/*      Write an attribute record to the file, but without any          */
/*      reformatting based on type.  The provided buffer is written     */
/*      as is to the field position in the record.                      */
/************************************************************************/

int  DBFWriteAttributeDirectly(DBFHandle psDBF, int hEntity,
                                          int iField, const void *pValue)
{
    /* -------------------------------------------------------------------- */
    /*      Is this a valid record?                                         */
    /* -------------------------------------------------------------------- */
    if (hEntity < 0 || hEntity > psDBF->nRecords)
        return (FALSE);

    if (psDBF->bNoHeader)
        DBFWriteHeader(psDBF);

    /* -------------------------------------------------------------------- */
    /*      Is this a brand new record?                                     */
    /* -------------------------------------------------------------------- */
    if (hEntity == psDBF->nRecords)
    {
        if (!DBFFlushRecord(psDBF))
            return FALSE;

        psDBF->nRecords++;
        for (int i = 0; i < psDBF->nRecordLength; i++)
            psDBF->pszCurrentRecord[i] = ' ';

        psDBF->nCurrentRecord = hEntity;
    }

    /* -------------------------------------------------------------------- */
    /*      Is this an existing record, but different than the last one     */
    /*      we accessed?                                                    */
    /* -------------------------------------------------------------------- */
    if (!DBFLoadRecord(psDBF, hEntity))
        return FALSE;

    if (iField >= 0)
    {
        unsigned char *pabyRec =
            REINTERPRET_CAST(unsigned char *, psDBF->pszCurrentRecord);

        /* -------------------------------------------------------------------- */
        /*      Assign all the record fields.                                   */
        /* -------------------------------------------------------------------- */
        int j;
        if (STATIC_CAST(int, strlen(STATIC_CAST(const char *, pValue))) >
            psDBF->panFieldSize[iField])
            j = psDBF->panFieldSize[iField];
        else
        {
            memset(pabyRec + psDBF->panFieldOffset[iField], ' ',
                   psDBF->panFieldSize[iField]);
            j = STATIC_CAST(int, strlen(STATIC_CAST(const char *, pValue)));
        }

        memcpy(
            REINTERPRET_CAST(char *, pabyRec + psDBF->panFieldOffset[iField]),
            STATIC_CAST(const char *, pValue), j);
    }

    psDBF->bCurrentRecordModified = TRUE;
    psDBF->bUpdated = TRUE;

    return (TRUE);
}

/************************************************************************/
/*                      DBFWriteDoubleAttribute()                       */
/*                                                                      */
/*      Write a double attribute.                                       */
/************************************************************************/

int  DBFWriteDoubleAttribute(DBFHandle psDBF, int iRecord,
                                        int iField, double dValue)
{
    return (DBFWriteAttribute(psDBF, iRecord, iField,
                              STATIC_CAST(void *, &dValue)));
}

/************************************************************************/
/*                      DBFWriteIntegerAttribute()                      */
/*                                                                      */
/*      Write an integer attribute.                                     */
/************************************************************************/

int  DBFWriteIntegerAttribute(DBFHandle psDBF, int iRecord,
                                         int iField, int nValue)
{
    double dValue = nValue;

    return (DBFWriteAttribute(psDBF, iRecord, iField,
                              STATIC_CAST(void *, &dValue)));
}

/************************************************************************/
/*                      DBFWriteStringAttribute()                       */
/*                                                                      */
/*      Write a string attribute.                                       */
/************************************************************************/

int  DBFWriteStringAttribute(DBFHandle psDBF, int iRecord,
                                        int iField, const char *pszValue)
{
    return (
        DBFWriteAttribute(psDBF, iRecord, iField,
                          STATIC_CAST(void *, CONST_CAST(char *, pszValue))));
}

/************************************************************************/
/*                      DBFWriteNULLAttribute()                         */
/*                                                                      */
/*      Write a NULL attribute.                                         */
/************************************************************************/

int  DBFWriteNULLAttribute(DBFHandle psDBF, int iRecord, int iField)
{
    return (DBFWriteAttribute(psDBF, iRecord, iField, SHPLIB_NULLPTR));
}

/************************************************************************/
/*                      DBFWriteLogicalAttribute()                      */
/*                                                                      */
/*      Write a logical attribute.                                      */
/************************************************************************/

int  DBFWriteLogicalAttribute(DBFHandle psDBF, int iRecord,
                                         int iField, const char lValue)
{
    return (
        DBFWriteAttribute(psDBF, iRecord, iField,
                          STATIC_CAST(void *, CONST_CAST(char *, &lValue))));
}

/************************************************************************/
/*                      DBFWriteDateAttribute()                         */
/*                                                                      */
/*      Write a date attribute.                                         */
/************************************************************************/

int  DBFWriteDateAttribute(DBFHandle psDBF, int iRecord, int iField,
                                      const SHPDate *lValue)
{
    if (SHPLIB_NULLPTR == lValue)
        return false;
    /* check for supported digit range, but do not check for valid date */
    if (lValue->year < 0 || lValue->year > 9999)
        return false;
    if (lValue->month < 0 || lValue->month > 99)
        return false;
    if (lValue->day < 0 || lValue->day > 99)
        return false;
    char dateValue[9]; /* "yyyyMMdd\0" */
    snprintf(dateValue, sizeof(dateValue), "%04d%02d%02d", lValue->year,
             lValue->month, lValue->day);
    return (DBFWriteAttributeDirectly(psDBF, iRecord, iField, dateValue));
}

/************************************************************************/
/*                         DBFWriteTuple()                              */
/*                                                                      */
/*      Write an attribute record to the file.                          */
/************************************************************************/

int  DBFWriteTuple(DBFHandle psDBF, int hEntity,
                              const void *pRawTuple)
{
    /* -------------------------------------------------------------------- */
    /*      Is this a valid record?                                         */
    /* -------------------------------------------------------------------- */
    if (hEntity < 0 || hEntity > psDBF->nRecords)
        return (FALSE);

    if (psDBF->bNoHeader)
        DBFWriteHeader(psDBF);

    /* -------------------------------------------------------------------- */
    /*      Is this a brand new record?                                     */
    /* -------------------------------------------------------------------- */
    if (hEntity == psDBF->nRecords)
    {
        if (!DBFFlushRecord(psDBF))
            return FALSE;

        psDBF->nRecords++;
        for (int i = 0; i < psDBF->nRecordLength; i++)
            psDBF->pszCurrentRecord[i] = ' ';

        psDBF->nCurrentRecord = hEntity;
    }

    /* -------------------------------------------------------------------- */
    /*      Is this an existing record, but different than the last one     */
    /*      we accessed?                                                    */
    /* -------------------------------------------------------------------- */
    if (!DBFLoadRecord(psDBF, hEntity))
        return FALSE;

    unsigned char *pabyRec =
        REINTERPRET_CAST(unsigned char *, psDBF->pszCurrentRecord);

    memcpy(pabyRec, pRawTuple, psDBF->nRecordLength);

    psDBF->bCurrentRecordModified = TRUE;
    psDBF->bUpdated = TRUE;

    return (TRUE);
}

/************************************************************************/
/*                            DBFReadTuple()                            */
/*                                                                      */
/*      Read a complete record.  Note that the result is only valid     */
/*      till the next record read for any reason.                       */
/************************************************************************/

const char *DBFReadTuple(DBFHandle psDBF, int hEntity)
{
    if (hEntity < 0 || hEntity >= psDBF->nRecords)
        return SHPLIB_NULLPTR;

    if (!DBFLoadRecord(psDBF, hEntity))
        return SHPLIB_NULLPTR;

    return STATIC_CAST(const char *, psDBF->pszCurrentRecord);
}

/************************************************************************/
/*                          DBFCloneEmpty()                             */
/*                                                                      */
/*      Create a new .dbf file with same code page and field            */
/*      definitions as the given handle.                                */
/************************************************************************/

DBFHandle  DBFCloneEmpty(const DBFHandle psDBF,
                                    const char *pszFilename)
{
    DBFHandle newDBF =
        DBFCreateLL(pszFilename, psDBF->pszCodePage, &psDBF->sHooks);
    if (newDBF == SHPLIB_NULLPTR)
        return SHPLIB_NULLPTR;

    newDBF->nFields = psDBF->nFields;
    newDBF->nRecordLength = psDBF->nRecordLength;
    newDBF->nHeaderLength = psDBF->nHeaderLength;

    if (psDBF->pszHeader)
    {
        newDBF->pszHeader =
            STATIC_CAST(char *, malloc(XBASE_FLDHDR_SZ * psDBF->nFields));
        memcpy(newDBF->pszHeader, psDBF->pszHeader,
               XBASE_FLDHDR_SZ * psDBF->nFields);
    }

    newDBF->panFieldOffset =
        STATIC_CAST(int *, malloc(sizeof(int) * psDBF->nFields));
    memcpy(newDBF->panFieldOffset, psDBF->panFieldOffset,
           sizeof(int) * psDBF->nFields);
    newDBF->panFieldSize =
        STATIC_CAST(int *, malloc(sizeof(int) * psDBF->nFields));
    memcpy(newDBF->panFieldSize, psDBF->panFieldSize,
           sizeof(int) * psDBF->nFields);
    newDBF->panFieldDecimals =
        STATIC_CAST(int *, malloc(sizeof(int) * psDBF->nFields));
    memcpy(newDBF->panFieldDecimals, psDBF->panFieldDecimals,
           sizeof(int) * psDBF->nFields);
    newDBF->pachFieldType =
        STATIC_CAST(char *, malloc(sizeof(char) * psDBF->nFields));
    memcpy(newDBF->pachFieldType, psDBF->pachFieldType,
           sizeof(char) * psDBF->nFields);

    newDBF->bNoHeader = TRUE;
    newDBF->bUpdated = TRUE;
    newDBF->bWriteEndOfFileChar = psDBF->bWriteEndOfFileChar;

    DBFWriteHeader(newDBF);
    DBFClose(newDBF);

    newDBF = DBFOpen(pszFilename, "rb+");
    newDBF->bWriteEndOfFileChar = psDBF->bWriteEndOfFileChar;

    return (newDBF);
}

/************************************************************************/
/*                       DBFGetNativeFieldType()                        */
/*                                                                      */
/*      Return the DBase field type for the specified field.            */
/*                                                                      */
/*      Value can be one of: 'C' (String), 'D' (Date), 'F' (Float),     */
/*                           'N' (Numeric, with or without decimal),    */
/*                           'L' (Logical),                             */
/*                           'M' (Memo: 10 digits .DBT block ptr)       */
/************************************************************************/

char  DBFGetNativeFieldType(const DBFHandle psDBF, int iField)
{
    if (iField >= 0 && iField < psDBF->nFields)
        return psDBF->pachFieldType[iField];

    return ' ';
}

/************************************************************************/
/*                          DBFGetFieldIndex()                          */
/*                                                                      */
/*      Get the index number for a field in a .dbf file.                */
/*                                                                      */
/*      Contributed by Jim Matthews.                                    */
/************************************************************************/

int  DBFGetFieldIndex(const DBFHandle psDBF,
                                 const char *pszFieldName)
{
    char name[XBASE_FLDNAME_LEN_READ + 1];

    for (int i = 0; i < DBFGetFieldCount(psDBF); i++)
    {
        DBFGetFieldInfo(psDBF, i, name, SHPLIB_NULLPTR, SHPLIB_NULLPTR);
        if (!STRCASECMP(pszFieldName, name))
            return (i);
    }
    return (-1);
}

/************************************************************************/
/*                         DBFIsRecordDeleted()                         */
/*                                                                      */
/*      Returns TRUE if the indicated record is deleted, otherwise      */
/*      it returns FALSE.                                               */
/************************************************************************/

int  DBFIsRecordDeleted(const DBFHandle psDBF, int iShape)
{
    /* -------------------------------------------------------------------- */
    /*      Verify selection.                                               */
    /* -------------------------------------------------------------------- */
    if (iShape < 0 || iShape >= psDBF->nRecords)
        return TRUE;

    /* -------------------------------------------------------------------- */
    /*      Have we read the record?                                        */
    /* -------------------------------------------------------------------- */
    if (!DBFLoadRecord(psDBF, iShape))
        return FALSE;

    /* -------------------------------------------------------------------- */
    /*      '*' means deleted.                                              */
    /* -------------------------------------------------------------------- */
    return psDBF->pszCurrentRecord[0] == '*';
}

int  DBFMarkRecordDeleted(DBFHandle psDBF, int iShape,
                                     int bIsDeleted)
{
    /* -------------------------------------------------------------------- */
    /*      Verify selection.                                               */
    /* -------------------------------------------------------------------- */
    if (iShape < 0 || iShape >= psDBF->nRecords)
        return FALSE;

    /* -------------------------------------------------------------------- */
    /*      Is this an existing record, but different than the last one     */
    /*      we accessed?                                                    */
    /* -------------------------------------------------------------------- */
    if (!DBFLoadRecord(psDBF, iShape))
        return FALSE;

    /* -------------------------------------------------------------------- */
    /*      Assign value, marking record as dirty if it changes.            */
    /* -------------------------------------------------------------------- */
    char chNewFlag;
    if (bIsDeleted)
        chNewFlag = '*';
    else
        chNewFlag = ' ';

    if (psDBF->pszCurrentRecord[0] != chNewFlag)
    {
        psDBF->bCurrentRecordModified = TRUE;
        psDBF->bUpdated = TRUE;
        psDBF->pszCurrentRecord[0] = chNewFlag;
    }

    return TRUE;
}

/************************************************************************/
/*                            DBFGetCodePage                            */
/************************************************************************/

const char *DBFGetCodePage(const DBFHandle psDBF)
{
    if (psDBF == SHPLIB_NULLPTR)
        return SHPLIB_NULLPTR;
    return psDBF->pszCodePage;
}

/************************************************************************/
/*                          DBFDeleteField()                            */
/*                                                                      */
/*      Remove a field from a .dbf file                                 */
/************************************************************************/

int  DBFDeleteField(DBFHandle psDBF, int iField)
{
    if (iField < 0 || iField >= psDBF->nFields)
        return FALSE;

    /* make sure that everything is written in .dbf */
    if (!DBFFlushRecord(psDBF))
        return FALSE;

    /* get information about field to be deleted */
    int nOldRecordLength = psDBF->nRecordLength;
    int nOldHeaderLength = psDBF->nHeaderLength;
    int nDeletedFieldOffset = psDBF->panFieldOffset[iField];
    int nDeletedFieldSize = psDBF->panFieldSize[iField];

    /* update fields info */
    for (int i = iField + 1; i < psDBF->nFields; i++)
    {
        psDBF->panFieldOffset[i - 1] =
            psDBF->panFieldOffset[i] - nDeletedFieldSize;
        psDBF->panFieldSize[i - 1] = psDBF->panFieldSize[i];
        psDBF->panFieldDecimals[i - 1] = psDBF->panFieldDecimals[i];
        psDBF->pachFieldType[i - 1] = psDBF->pachFieldType[i];
    }

    /* resize fields arrays */
    psDBF->nFields--;

    psDBF->panFieldOffset = STATIC_CAST(
        int *, realloc(psDBF->panFieldOffset, sizeof(int) * psDBF->nFields));

    psDBF->panFieldSize = STATIC_CAST(
        int *, realloc(psDBF->panFieldSize, sizeof(int) * psDBF->nFields));

    psDBF->panFieldDecimals = STATIC_CAST(
        int *, realloc(psDBF->panFieldDecimals, sizeof(int) * psDBF->nFields));

    psDBF->pachFieldType = STATIC_CAST(
        char *, realloc(psDBF->pachFieldType, sizeof(char) * psDBF->nFields));

    /* update header information */
    psDBF->nHeaderLength -= XBASE_FLDHDR_SZ;
    psDBF->nRecordLength -= nDeletedFieldSize;

    /* overwrite field information in header */
    memmove(psDBF->pszHeader + iField * XBASE_FLDHDR_SZ,
            psDBF->pszHeader + (iField + 1) * XBASE_FLDHDR_SZ,
            sizeof(char) * (psDBF->nFields - iField) * XBASE_FLDHDR_SZ);

    psDBF->pszHeader = STATIC_CAST(
        char *, realloc(psDBF->pszHeader, psDBF->nFields * XBASE_FLDHDR_SZ));

    /* update size of current record appropriately */
    psDBF->pszCurrentRecord = STATIC_CAST(
        char *, realloc(psDBF->pszCurrentRecord, psDBF->nRecordLength));

    /* we're done if we're dealing with not yet created .dbf */
    if (psDBF->bNoHeader && psDBF->nRecords == 0)
        return TRUE;

    /* force update of header with new header and record length */
    psDBF->bNoHeader = TRUE;
    DBFUpdateHeader(psDBF);

    /* alloc record */
    char *pszRecord =
        STATIC_CAST(char *, malloc(sizeof(char) * nOldRecordLength));

    /* shift records to their new positions */
    for (int iRecord = 0; iRecord < psDBF->nRecords; iRecord++)
    {
        SAOffset nRecordOffset =
            nOldRecordLength * STATIC_CAST(SAOffset, iRecord) +
            nOldHeaderLength;

        /* load record */
        psDBF->sHooks.FSeek(psDBF->fp, nRecordOffset, 0);
        if (psDBF->sHooks.FRead(pszRecord, nOldRecordLength, 1, psDBF->fp) != 1)
        {
            free(pszRecord);
            return FALSE;
        }

        nRecordOffset = psDBF->nRecordLength * STATIC_CAST(SAOffset, iRecord) +
                        psDBF->nHeaderLength;

        /* move record in two steps */
        psDBF->sHooks.FSeek(psDBF->fp, nRecordOffset, 0);
        psDBF->sHooks.FWrite(pszRecord, nDeletedFieldOffset, 1, psDBF->fp);
        psDBF->sHooks.FWrite(
            pszRecord + nDeletedFieldOffset + nDeletedFieldSize,
            nOldRecordLength - nDeletedFieldOffset - nDeletedFieldSize, 1,
            psDBF->fp);
    }

    if (psDBF->bWriteEndOfFileChar)
    {
        char ch = END_OF_FILE_CHARACTER;
        SAOffset nEOFOffset =
            psDBF->nRecordLength * STATIC_CAST(SAOffset, psDBF->nRecords) +
            psDBF->nHeaderLength;

        psDBF->sHooks.FSeek(psDBF->fp, nEOFOffset, 0);
        psDBF->sHooks.FWrite(&ch, 1, 1, psDBF->fp);
    }

    /* TODO: truncate file */

    /* free record */
    free(pszRecord);

    psDBF->nCurrentRecord = -1;
    psDBF->bCurrentRecordModified = FALSE;
    psDBF->bUpdated = TRUE;

    return TRUE;
}

/************************************************************************/
/*                          DBFReorderFields()                          */
/*                                                                      */
/*      Reorder the fields of a .dbf file                               */
/*                                                                      */
/* panMap must be exactly psDBF->nFields long and be a permutation      */
/* of [0, psDBF->nFields-1]. This assumption will not be asserted in the*/
/* code of DBFReorderFields.                                            */
/************************************************************************/

int  DBFReorderFields(DBFHandle psDBF, const int *panMap)
{
    if (psDBF->nFields == 0)
        return TRUE;

    /* make sure that everything is written in .dbf */
    if (!DBFFlushRecord(psDBF))
        return FALSE;

    /* a simple malloc() would be enough, but calloc() helps clang static
     * analyzer */
    int *panFieldOffsetNew =
        STATIC_CAST(int *, calloc(psDBF->nFields, sizeof(int)));
    int *panFieldSizeNew =
        STATIC_CAST(int *, calloc(psDBF->nFields, sizeof(int)));
    int *panFieldDecimalsNew =
        STATIC_CAST(int *, calloc(psDBF->nFields, sizeof(int)));
    char *pachFieldTypeNew =
        STATIC_CAST(char *, calloc(psDBF->nFields, sizeof(char)));
    char *pszHeaderNew = STATIC_CAST(
        char *, malloc(sizeof(char) * XBASE_FLDHDR_SZ * psDBF->nFields));

    /* shuffle fields definitions */
    for (int i = 0; i < psDBF->nFields; i++)
    {
        panFieldSizeNew[i] = psDBF->panFieldSize[panMap[i]];
        panFieldDecimalsNew[i] = psDBF->panFieldDecimals[panMap[i]];
        pachFieldTypeNew[i] = psDBF->pachFieldType[panMap[i]];
        memcpy(pszHeaderNew + i * XBASE_FLDHDR_SZ,
               psDBF->pszHeader + panMap[i] * XBASE_FLDHDR_SZ, XBASE_FLDHDR_SZ);
    }
    panFieldOffsetNew[0] = 1;
    for (int i = 1; i < psDBF->nFields; i++)
    {
        panFieldOffsetNew[i] =
            panFieldOffsetNew[i - 1] + panFieldSizeNew[i - 1];
    }

    free(psDBF->pszHeader);
    psDBF->pszHeader = pszHeaderNew;

    bool errorAbort = false;

    /* we're done if we're dealing with not yet created .dbf */
    if (!(psDBF->bNoHeader && psDBF->nRecords == 0))
    {
        /* force update of header with new header and record length */
        psDBF->bNoHeader = TRUE;
        DBFUpdateHeader(psDBF);

        /* alloc record */
        char *pszRecord =
            STATIC_CAST(char *, malloc(sizeof(char) * psDBF->nRecordLength));
        char *pszRecordNew =
            STATIC_CAST(char *, malloc(sizeof(char) * psDBF->nRecordLength));

        /* shuffle fields in records */
        for (int iRecord = 0; iRecord < psDBF->nRecords; iRecord++)
        {
            const SAOffset nRecordOffset =
                psDBF->nRecordLength * STATIC_CAST(SAOffset, iRecord) +
                psDBF->nHeaderLength;

            /* load record */
            psDBF->sHooks.FSeek(psDBF->fp, nRecordOffset, 0);
            if (psDBF->sHooks.FRead(pszRecord, psDBF->nRecordLength, 1,
                                    psDBF->fp) != 1)
            {
                errorAbort = true;
                break;
            }

            pszRecordNew[0] = pszRecord[0];

            for (int i = 0; i < psDBF->nFields; i++)
            {
                memcpy(pszRecordNew + panFieldOffsetNew[i],
                       pszRecord + psDBF->panFieldOffset[panMap[i]],
                       psDBF->panFieldSize[panMap[i]]);
            }

            /* write record */
            psDBF->sHooks.FSeek(psDBF->fp, nRecordOffset, 0);
            psDBF->sHooks.FWrite(pszRecordNew, psDBF->nRecordLength, 1,
                                 psDBF->fp);
        }

        /* free record */
        free(pszRecord);
        free(pszRecordNew);
    }

    if (errorAbort)
    {
        free(panFieldOffsetNew);
        free(panFieldSizeNew);
        free(panFieldDecimalsNew);
        free(pachFieldTypeNew);
        psDBF->nCurrentRecord = -1;
        psDBF->bCurrentRecordModified = FALSE;
        psDBF->bUpdated = FALSE;
        return FALSE;
    }

    free(psDBF->panFieldOffset);
    free(psDBF->panFieldSize);
    free(psDBF->panFieldDecimals);
    free(psDBF->pachFieldType);

    psDBF->panFieldOffset = panFieldOffsetNew;
    psDBF->panFieldSize = panFieldSizeNew;
    psDBF->panFieldDecimals = panFieldDecimalsNew;
    psDBF->pachFieldType = pachFieldTypeNew;

    psDBF->nCurrentRecord = -1;
    psDBF->bCurrentRecordModified = FALSE;
    psDBF->bUpdated = TRUE;

    return TRUE;
}


void  DBFSetWriteEndOfFileChar(DBFHandle psDBF, int bWriteFlag)
{
    psDBF->bWriteEndOfFileChar = bWriteFlag;
}

int  DBFAlterFieldDefn(DBFHandle psDBF, int iField,
                                  const char *pszFieldName, char chType,
                                  int nWidth, int nDecimals)
{
    if (iField < 0 || iField >= psDBF->nFields)
        return FALSE;

    /* make sure that everything is written in .dbf */
    if (!DBFFlushRecord(psDBF))
        return FALSE;

    const char chFieldFill = DBFGetNullCharacter(chType);

    const char chOldType = psDBF->pachFieldType[iField];
    const int nOffset = psDBF->panFieldOffset[iField];
    const int nOldWidth = psDBF->panFieldSize[iField];
    const int nOldRecordLength = psDBF->nRecordLength;

    /* -------------------------------------------------------------------- */
    /*      Do some checking to ensure we can add records to this file.     */
    /* -------------------------------------------------------------------- */
    if (nWidth < 1)
        return -1;

    if (nWidth > XBASE_FLD_MAX_WIDTH)
        nWidth = XBASE_FLD_MAX_WIDTH;

    /* -------------------------------------------------------------------- */
    /*      Assign the new field information fields.                        */
    /* -------------------------------------------------------------------- */
    psDBF->panFieldSize[iField] = nWidth;
    psDBF->panFieldDecimals[iField] = nDecimals;
    psDBF->pachFieldType[iField] = chType;

    /* -------------------------------------------------------------------- */
    /*      Update the header information.                                  */
    /* -------------------------------------------------------------------- */
    char *pszFInfo = psDBF->pszHeader + XBASE_FLDHDR_SZ * iField;

    for (int i = 0; i < XBASE_FLDHDR_SZ; i++)
        pszFInfo[i] = '\0';

    strncpy(pszFInfo, pszFieldName, XBASE_FLDNAME_LEN_WRITE);

    pszFInfo[11] = psDBF->pachFieldType[iField];

    if (chType == 'C')
    {
        pszFInfo[16] = STATIC_CAST(unsigned char, nWidth % 256);
        pszFInfo[17] = STATIC_CAST(unsigned char, nWidth / 256);
    }
    else
    {
        pszFInfo[16] = STATIC_CAST(unsigned char, nWidth);
        pszFInfo[17] = STATIC_CAST(unsigned char, nDecimals);
    }

    /* -------------------------------------------------------------------- */
    /*      Update offsets                                                  */
    /* -------------------------------------------------------------------- */
    if (nWidth != nOldWidth)
    {
        for (int i = iField + 1; i < psDBF->nFields; i++)
            psDBF->panFieldOffset[i] += nWidth - nOldWidth;
        psDBF->nRecordLength += nWidth - nOldWidth;

        psDBF->pszCurrentRecord = STATIC_CAST(
            char *, realloc(psDBF->pszCurrentRecord, psDBF->nRecordLength));
    }

    /* we're done if we're dealing with not yet created .dbf */
    if (psDBF->bNoHeader && psDBF->nRecords == 0)
        return TRUE;

    /* force update of header with new header and record length */
    psDBF->bNoHeader = TRUE;
    DBFUpdateHeader(psDBF);

    bool errorAbort = false;

    if (nWidth < nOldWidth || (nWidth == nOldWidth && chType != chOldType))
    {
        char *pszRecord =
            STATIC_CAST(char *, malloc(sizeof(char) * nOldRecordLength));
        char *pszOldField =
            STATIC_CAST(char *, malloc(sizeof(char) * (nOldWidth + 1)));

        pszOldField[nOldWidth] = 0;

        /* move records to their new positions */
        for (int iRecord = 0; iRecord < psDBF->nRecords; iRecord++)
        {
            SAOffset nRecordOffset =
                nOldRecordLength * STATIC_CAST(SAOffset, iRecord) +
                psDBF->nHeaderLength;

            /* load record */
            psDBF->sHooks.FSeek(psDBF->fp, nRecordOffset, 0);
            if (psDBF->sHooks.FRead(pszRecord, nOldRecordLength, 1,
                                    psDBF->fp) != 1)
            {
                errorAbort = true;
                break;
            }

            memcpy(pszOldField, pszRecord + nOffset, nOldWidth);
            const bool bIsNULL =
                DBFIsValueNULL(chOldType, pszOldField);

            if (nWidth != nOldWidth)
            {
                if ((chOldType == 'N' || chOldType == 'F' ||
                     chOldType == 'D') &&
                    pszOldField[0] == ' ')
                {
                    /* Strip leading spaces when truncating a numeric field */
                    memmove(pszRecord + nOffset,
                            pszRecord + nOffset + nOldWidth - nWidth, nWidth);
                }
                if (nOffset + nOldWidth < nOldRecordLength)
                {
                    memmove(pszRecord + nOffset + nWidth,
                            pszRecord + nOffset + nOldWidth,
                            nOldRecordLength - (nOffset + nOldWidth));
                }
            }

            /* Convert null value to the appropriate value of the new type */
            if (bIsNULL)
            {
                memset(pszRecord + nOffset, chFieldFill, nWidth);
            }

            nRecordOffset =
                psDBF->nRecordLength * STATIC_CAST(SAOffset, iRecord) +
                psDBF->nHeaderLength;

            /* write record */
            psDBF->sHooks.FSeek(psDBF->fp, nRecordOffset, 0);
            psDBF->sHooks.FWrite(pszRecord, psDBF->nRecordLength, 1, psDBF->fp);
        }

        if (!errorAbort && psDBF->bWriteEndOfFileChar)
        {
            char ch = END_OF_FILE_CHARACTER;

            SAOffset nRecordOffset =
                psDBF->nRecordLength * STATIC_CAST(SAOffset, psDBF->nRecords) +
                psDBF->nHeaderLength;

            psDBF->sHooks.FSeek(psDBF->fp, nRecordOffset, 0);
            psDBF->sHooks.FWrite(&ch, 1, 1, psDBF->fp);
        }
        /* TODO: truncate file */

        free(pszRecord);
        free(pszOldField);
    }
    else if (nWidth > nOldWidth)
    {
        char *pszRecord =
            STATIC_CAST(char *, malloc(sizeof(char) * psDBF->nRecordLength));
        char *pszOldField =
            STATIC_CAST(char *, malloc(sizeof(char) * (nOldWidth + 1)));

        pszOldField[nOldWidth] = 0;

        /* move records to their new positions */
        for (int iRecord = psDBF->nRecords - 1; iRecord >= 0; iRecord--)
        {
            SAOffset nRecordOffset =
                nOldRecordLength * STATIC_CAST(SAOffset, iRecord) +
                psDBF->nHeaderLength;

            /* load record */
            psDBF->sHooks.FSeek(psDBF->fp, nRecordOffset, 0);
            if (psDBF->sHooks.FRead(pszRecord, nOldRecordLength, 1,
                                    psDBF->fp) != 1)
            {
                errorAbort = true;
                break;
            }

            memcpy(pszOldField, pszRecord + nOffset, nOldWidth);
            const bool bIsNULL =
                DBFIsValueNULL(chOldType, pszOldField);

            if (nOffset + nOldWidth < nOldRecordLength)
            {
                memmove(pszRecord + nOffset + nWidth,
                        pszRecord + nOffset + nOldWidth,
                        nOldRecordLength - (nOffset + nOldWidth));
            }

            /* Convert null value to the appropriate value of the new type */
            if (bIsNULL)
            {
                memset(pszRecord + nOffset, chFieldFill, nWidth);
            }
            else
            {
                if ((chOldType == 'N' || chOldType == 'F'))
                {
                    /* Add leading spaces when expanding a numeric field */
                    memmove(pszRecord + nOffset + nWidth - nOldWidth,
                            pszRecord + nOffset, nOldWidth);
                    memset(pszRecord + nOffset, ' ', nWidth - nOldWidth);
                }
                else
                {
                    /* Add trailing spaces */
                    memset(pszRecord + nOffset + nOldWidth, ' ',
                           nWidth - nOldWidth);
                }
            }

            nRecordOffset =
                psDBF->nRecordLength * STATIC_CAST(SAOffset, iRecord) +
                psDBF->nHeaderLength;

            /* write record */
            psDBF->sHooks.FSeek(psDBF->fp, nRecordOffset, 0);
            psDBF->sHooks.FWrite(pszRecord, psDBF->nRecordLength, 1, psDBF->fp);
        }

        if (!errorAbort && psDBF->bWriteEndOfFileChar)
        {
            char ch = END_OF_FILE_CHARACTER;

            SAOffset nRecordOffset =
                psDBF->nRecordLength * STATIC_CAST(SAOffset, psDBF->nRecords) +
                psDBF->nHeaderLength;

            psDBF->sHooks.FSeek(psDBF->fp, nRecordOffset, 0);
            psDBF->sHooks.FWrite(&ch, 1, 1, psDBF->fp);
        }

        free(pszRecord);
        free(pszOldField);
    }

    if (errorAbort)
    {
        psDBF->nCurrentRecord = -1;
        psDBF->bCurrentRecordModified = TRUE;
        psDBF->bUpdated = FALSE;

        return FALSE;
    }
    psDBF->nCurrentRecord = -1;
    psDBF->bCurrentRecordModified = FALSE;
    psDBF->bUpdated = TRUE;

    return TRUE;
}

#endif /* ndef SHAPEFILE_H_INCLUDED */
