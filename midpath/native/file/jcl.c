/*
 * MIDPath - Copyright (C) 2006-2008 Guillaume Legris, Mathieu Legris
 * 
 * GNU Classpath - Copyright (C) 1998 Free Software Foundation, Inc.
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License version
 * 2 only, as published by the Free Software Foundation. 
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License version 2 for more details. 
 * 
 * You should have received a copy of the GNU General Public License
 * version 2 along with this work; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA  
 */

/* do not move; needed here because of some macro definitions */
/*#include "config.h"*/

#include <stdlib.h>
#include <stdio.h>
#include "jcl.h"

#ifndef __GNUC__
  #ifndef __attribute__
    #define __attribute__(x)	/* nothing */
  #endif
#endif


///*
// * JNI OnLoad constructor.
// */
//JNIEXPORT jint JNICALL
//JNI_OnLoad (JavaVM *vm, void *reserved __attribute__((unused)))
//{
//  JNIEnv *env;
//  void *envp;
//
//  if ((*vm)->GetEnv (vm, &envp, JNI_VERSION_1_4) != JNI_OK)
//    {
//      return JNI_VERSION_1_4;
//    }
//  env = (JNIEnv *) envp;
//#if SIZEOF_VOID_P == 8
//  rawDataClass = (*env)->FindClass (env, "gnu/classpath/Pointer64");
//  if (rawDataClass != NULL)
//    rawDataClass = (*env)->NewGlobalRef (env, rawDataClass);
//
//  if (rawDataClass != NULL)
//    {
//      rawData_fid = (*env)->GetFieldID (env, rawDataClass, "data", "J");
//      rawData_mid = (*env)->GetMethodID (env, rawDataClass, "<init>", "(J)V");
//    }
//#else
//#if SIZEOF_VOID_P == 4
//  rawDataClass = (*env)->FindClass (env, "gnu/classpath/Pointer32");
//  if (rawDataClass != NULL)
//    rawDataClass = (*env)->NewGlobalRef (env, rawDataClass);
//
//  if (rawDataClass != NULL)
//    {
//      rawData_fid = (*env)->GetFieldID (env, rawDataClass, "data", "I");
//      rawData_mid = (*env)->GetMethodID (env, rawDataClass, "<init>", "(I)V");
//    }
//#endif /* SIZEOF_VOID_P == 4 */
//#endif /* SIZEOF_VOID_P == 8 */
//
//  return JNI_VERSION_1_4;
//}


JNIEXPORT void JNICALL
JCL_ThrowException (JNIEnv * env, const char *className, const char *errMsg)
{
  jclass excClass;
  if ((*env)->ExceptionOccurred (env))
    {
      (*env)->ExceptionClear (env);
    }
  excClass = (*env)->FindClass (env, className);
  if (excClass == NULL)
    {
      jclass errExcClass;
      errExcClass =
	(*env)->FindClass (env, "java/lang/ClassNotFoundException");
      if (errExcClass == NULL)
	{
	  errExcClass = (*env)->FindClass (env, "java/lang/InternalError");
	  if (errExcClass == NULL)
	    {
	      fprintf (stderr, "JCL: Utterly failed to throw exeption ");
	      fprintf (stderr, "%s", className);
	      fprintf (stderr, " with message ");
	      fprintf (stderr, "%s", errMsg);
	      return;
	    }
	}
      /* Removed this (more comprehensive) error string to avoid the need for
       * a static variable or allocation of a buffer for this message in this
       * (unlikely) error case. --Fridi. 
       *
       * sprintf(errstr,"JCL: Failed to throw exception %s with message %s: could not find exception class.", className, errMsg); 
       */
      (*env)->ThrowNew (env, errExcClass, className);
    }
  (*env)->ThrowNew (env, excClass, errMsg);
}

JNIEXPORT void *JNICALL
JCL_malloc (JNIEnv * env, size_t size)
{
  void *mem = malloc (size);
  if (mem == NULL)
    {
      JCL_ThrowException (env, "java/lang/OutOfMemoryError",
			  "malloc() failed.");
      return NULL;
    }
  return mem;
}

JNIEXPORT void *JNICALL
JCL_realloc (JNIEnv * env, void *ptr, size_t size)
{
  ptr = realloc (ptr, size);
  if (ptr == 0)
    {
      JCL_ThrowException (env, "java/lang/OutOfMemoryError",
			  "malloc() failed.");
      return NULL;
    }
  return (ptr);
}

JNIEXPORT void JNICALL
JCL_free (JNIEnv * env __attribute__ ((unused)), void *p)
{
  if (p != NULL)
    {
      free (p);
    }
}

JNIEXPORT const char *JNICALL
JCL_jstring_to_cstring (JNIEnv * env, jstring s)
{
  const char *cstr;
  if (s == NULL)
    {
      JCL_ThrowException (env, "java/lang/NullPointerException",
			  "Null string");
      return NULL;
    }
  cstr = (const char *) (*env)->GetStringUTFChars (env, s, NULL);
  if (cstr == NULL)
    {
      JCL_ThrowException (env, "java/lang/InternalError",
			  "GetStringUTFChars() failed.");
      return NULL;
    }
  return cstr;
}

JNIEXPORT void JNICALL
JCL_free_cstring (JNIEnv * env, jstring s, const char *cstr)
{
  (*env)->ReleaseStringUTFChars (env, s, cstr);
}

//JNIEXPORT jint JNICALL
//JCL_MonitorEnter (JNIEnv * env, jobject o)
//{
//  jint retval = (*env)->MonitorEnter (env, o);
//  if (retval != 0)
//    {
//      JCL_ThrowException (env, "java/lang/InternalError",
//			  "MonitorEnter() failed.");
//    }
//  return retval;
//}

//JNIEXPORT jint JNICALL
//JCL_MonitorExit (JNIEnv * env, jobject o)
//{
//  jint retval = (*env)->MonitorExit (env, o);
//  if (retval != 0)
//    {
//      JCL_ThrowException (env, "java/lang/InternalError",
//			  "MonitorExit() failed.");
//    }
//  return retval;
//}

//JNIEXPORT jclass JNICALL
//JCL_FindClass (JNIEnv * env, const char *className)
//{
//  jclass retval = (*env)->FindClass (env, className);
//  if (retval == NULL)
//    {
//      JCL_ThrowException (env, "java/lang/ClassNotFoundException", className);
//    }
//  return retval;
//}


/*
 * Build a Pointer object.
 */

/*JNIEXPORT jobject JNICALL
JCL_NewRawDataObject (JNIEnv * env, void *data)
{
  if (rawDataClass == NULL || rawData_mid == NULL)
    {
      JCL_ThrowException (env, "java/lang/InternalError",
                          "Pointer class was not properly initialized");
      return NULL;
    }

#if SIZEOF_VOID_P == 8
  return (*env)->NewObject (env, rawDataClass, rawData_mid, (jlong) data);
#else
  return (*env)->NewObject (env, rawDataClass, rawData_mid, (jint) data);
#endif
}*/

/*JNIEXPORT void * JNICALL
JCL_GetRawData (JNIEnv * env, jobject rawdata)
{
  if (rawData_fid == NULL)
    {
      JCL_ThrowException (env, "java/lang/InternalError",
                          "Pointer class was not properly initialized");
      return NULL;
    }

#if SIZEOF_VOID_P == 8
  return (void *) (*env)->GetLongField (env, rawdata, rawData_fid);
#else
  return (void *) (*env)->GetIntField (env, rawdata, rawData_fid);
#endif  
}*/
