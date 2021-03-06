/* src/native/vm/cldc1.1/com_sun_cldc_io_ResourceInputStream.c

   Copyright (C) 2007 R. Grafl, A. Krall, C. Kruegel, C. Oates,
   R. Obermaisser, M. Platter, M. Probst, S. Ring, E. Steiner,
   C. Thalinger, D. Thuernbeck, P. Tomsich, C. Ullrich, J. Wenninger,
   Institut f. Computersprachen - TU Wien

   This file is part of CACAO.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2, or (at
   your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.

   $Id: java_lang_VMRuntime.c 5900 2006-11-04 17:30:44Z michi $

*/

#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <zlib.h>

#include "config.h"

#include "mm/memory.h"

#include "native/jni.h"
#include "native/native.h"

#include "native/include/java_lang_Object.h"
#include "native/include/java_lang_String.h"
#include "native/include/java_lang_Long.h"
#include "native/include/com_sun_cldc_io_ResourceInputStream.h"

#include "vm/types.h"
#include "vm/builtin.h"
#include "vm/vm.h" /* REMOVE ME: temporarily */
#include "vm/exceptions.h"
#include "vm/stringlocal.h"

#include "vmcore/zip.h"

#include "threads/lock-common.h"

/* native methods implemented by this file ************************************/
 
static JNINativeMethod methods[] = {
	{ "open", "(Ljava/lang/String;)Ljava/lang/Object;", (void *) (ptrint) &Java_com_sun_cldc_io_ResourceInputStream_open },
	{ "bytesRemain", "(Ljava/lang/Object;)I", (void *) (ptrint) &Java_com_sun_cldc_io_ResourceInputStream_bytesRemain },
	{ "readByte", "(Ljava/lang/Object;)I", (void *) (ptrint) &Java_com_sun_cldc_io_ResourceInputStream_readByte },
	{ "readBytes", "(Ljava/lang/Object;[BII)I", (void *) (ptrint) &Java_com_sun_cldc_io_ResourceInputStream_readBytes },
	{ "clone", "(Ljava/lang/Object;)Ljava/lang/Object;", (void *) (ptrint) &Java_com_sun_cldc_io_ResourceInputStream_clone },
};
 
/* _Jv_com_sun_cldc_io_ResourceInputStream_init ********************************
 
   Register native functions.
 
*******************************************************************************/
 
void _Jv_com_sun_cldc_io_ResourceInputStream_init(void)
{
	utf *u;
 
	u = utf_new_char("com/sun/cldc/io/ResourceInputStream");
 
	native_method_register(u, methods, NATIVE_METHODS_COUNT);
}

#define LFH_HEADER_SIZE              30
#define LFH_SIGNATURE                0x04034b50
#define LFH_FILE_NAME_LENGTH         26
#define LFH_EXTRA_FIELD_LENGTH       28

typedef struct lfh lfh;

struct lfh {
	u2 compressionmethod;
	u4 compressedsize;
	u4 uncompressedsize;
	u2 filenamelength;
	u2 extrafieldlength;
};


#define FILE_INFOS 3
#define FILE_POINTER 0
#define FILE_LENGTH 1
#define FILE_POS 2

static struct java_longarray* zip_read_resource(list_classpath_entry *lce, utf *name)
{
	hashtable_zipfile_entry *htzfe;
	lfh                      lfh;
	u1                      *indata;
	u1                      *outdata;
	z_stream                 zs;
	int                      err;
	java_longarray  *fhandler;
	
	/* try to find the class in the current archive */

	htzfe = zip_find(lce, name);

	if (htzfe == NULL)
		return NULL;

	/* read stuff from local file header */

	lfh.filenamelength   = SUCK_LE_U2(htzfe->data + LFH_FILE_NAME_LENGTH);
	lfh.extrafieldlength = SUCK_LE_U2(htzfe->data + LFH_EXTRA_FIELD_LENGTH);

	indata = htzfe->data +
		LFH_HEADER_SIZE +
		lfh.filenamelength +
		lfh.extrafieldlength;

	/* allocate buffer for uncompressed data */

	outdata = MNEW(u1, htzfe->uncompressedsize);

	/* how is the file stored? */

	switch (htzfe->compressionmethod) {
	case Z_DEFLATED:
		/* fill z_stream structure */

		zs.next_in   = indata;
		zs.avail_in  = htzfe->compressedsize;
		zs.next_out  = outdata;
		zs.avail_out = htzfe->uncompressedsize;

		zs.zalloc = Z_NULL;
		zs.zfree  = Z_NULL;
		zs.opaque = Z_NULL;

		/* initialize this inflate run */

		if (inflateInit2(&zs, -MAX_WBITS) != Z_OK)
			vm_abort("zip_get: inflateInit2 failed: %s", strerror(errno));

		/* decompress the file into buffer */

		err = inflate(&zs, Z_SYNC_FLUSH);

		if ((err != Z_STREAM_END) && (err != Z_OK))
			vm_abort("zip_get: inflate failed: %s", strerror(errno));

		/* finish this inflate run */

		if (inflateEnd(&zs) != Z_OK)
			vm_abort("zip_get: inflateEnd failed: %s", strerror(errno));
		break;

	case 0:
		/* uncompressed file, just copy the data */
		MCOPY(outdata, indata, u1, htzfe->compressedsize);
		break;

	default:
		vm_abort("zip_get: unknown compression method %d",
				 htzfe->compressionmethod);
	}
	
	/* Create a file handler object (currently an int array) */
	if (!(fhandler = (java_longarray *) builtin_newarray_long(FILE_INFOS)))
		return NULL;
	fhandler->data[FILE_POINTER] = (int)outdata; /* data pointer */
	fhandler->data[FILE_LENGTH] = htzfe->uncompressedsize; /* File size */
	fhandler->data[FILE_POS] = (int) 0; /* File Access position */
	return fhandler;
	
}

static struct java_longarray* file_read_resource(char *path) 
{
	int len;
	struct stat statBuffer;
	u1 *filep;
	java_longarray  *fhandler;
	int fd;
	
	fd = open(path, O_RDONLY);
	
	if (fd > 0) {
		
		if (fstat(fd, &statBuffer) != -1) {
			len = statBuffer.st_size;
		} else {
			return NULL;
		}
		
		/* Map file into the memory */
		filep = mmap(0, len, PROT_READ, MAP_PRIVATE, fd, 0);
		
		/* Create a file handler object (currently an int array) */
		if (!(fhandler = (java_longarray *) builtin_newarray_long(FILE_INFOS)))
			return NULL;
		fhandler->data[FILE_POINTER] = (int)filep; /* File pointer */
		fhandler->data[FILE_LENGTH] = (int) len; /* File size */
		fhandler->data[FILE_POS] = (int) 0; /* File Access position */
		return fhandler;
		
		
	} else {
		return NULL;
	}
	
}

/*
 * Class:     com/sun/cldc/io/ResourceInputStream
 * Method:    open
 * Signature: (Ljava/lang/String;)Ljava/lang/Object;
 */
JNIEXPORT struct java_lang_Object* JNICALL Java_com_sun_cldc_io_ResourceInputStream_open(JNIEnv *env, jclass clazz, java_lang_String *name)
{
	
	list_classpath_entry *lce;
	char *filename;
	s4 filenamelen;
	char *path;
	utf *uname;
	java_longarray  *fhandler;
	
	/* get the classname as char string (do it here for the warning at
       the end of the function) */

	uname = javastring_toutf((java_objectheader *)name, false);
	filenamelen = utf_bytes(uname) + strlen("0");
	filename = MNEW(char, filenamelen);
	utf_copy(filename, uname);
	
	/* walk through all classpath entries */

	for (lce = list_first(list_classpath_entries); lce != NULL;
		 lce = list_next(list_classpath_entries, lce)) {
		 	
#if defined(ENABLE_ZLIB)
		if (lce->type == CLASSPATH_ARCHIVE) {

			/* enter a monitor on zip/jar archives */
			LOCK_MONITOR_ENTER(lce);

			/* try to get the file in current archive */
			fhandler = zip_read_resource(lce, uname);

			/* leave the monitor */
			LOCK_MONITOR_EXIT(lce);
			
			if (fhandler != NULL) { /* file exists */
				break;
			}

		} else {
#endif
			
			path = MNEW(char, lce->pathlen + filenamelen);
			strcpy(path, lce->path);
			strcat(path, filename);

			fhandler = file_read_resource(path);
			
			MFREE(path, char, lce->pathlen + filenamelen);

			if (fhandler != NULL) { /* file exists */
				break;
			}
			
#if defined(ENABLE_ZLIB)
		}
#endif	
			
	}

	MFREE(filename, char, filenamelen);

	return (java_lang_Object*) fhandler;
	
}


/*
 * Class:     com_sun_cldc_io_ResourceInputStream
 * Method:    bytesRemain
 * Signature: (Ljava/lang/Object;)I
 */
JNIEXPORT s4 JNICALL Java_com_sun_cldc_io_ResourceInputStream_bytesRemain(JNIEnv *env, jclass clazz, struct java_lang_Object* jobj) {
	
	java_longarray  *fhandler;
	fhandler = (java_longarray *) jobj;
	
	return fhandler->data[FILE_LENGTH] - fhandler->data[FILE_POS];

}

/*
 * Class:     com_sun_cldc_io_ResourceInputStream
 * Method:    readByte
 * Signature: (Ljava/lang/Object;)I
 */
JNIEXPORT s4 JNICALL Java_com_sun_cldc_io_ResourceInputStream_readByte(JNIEnv *env, jclass clazz, struct java_lang_Object* jobj) {
	
	java_longarray  *fhandler;
	u1 byte;
	s8 len;
	s8 position;
	u1 *filep;
	
	fhandler = (java_longarray *) jobj;
	position = fhandler->data[FILE_POS];
	len = fhandler->data[FILE_LENGTH];
	filep = (u1*)(int)fhandler->data[FILE_POINTER];
	
	if (position < len) {
		byte = (u1)filep[position];
		position++;
	} else {
		return -1; /* EOF */
	}
	
	/* Update access position */
	fhandler->data[FILE_POS] = position;
	
	return (byte & 0xFF);

}

/*
 * Class:     com_sun_cldc_io_ResourceInputStream
 * Method:    readBytes
 * Signature: (Ljava/lang/Object;[BII)I
 */
JNIEXPORT s4 JNICALL Java_com_sun_cldc_io_ResourceInputStream_readBytes(JNIEnv *env, jclass clazz, struct java_lang_Object* jobj, java_bytearray* byteArray, s4 off, s4 len) {
	
	java_longarray  *fhandler;
	int readBytes = -1;
	s8 fileLength;
	s8 position;
	s8 available;
	u1 *filep;
	void *buf;

	/* get pointer to the buffer */
	buf = &(byteArray->data[off]);
	
	fhandler = (java_longarray *) jobj;
	position = fhandler->data[FILE_POS];
	fileLength = fhandler->data[FILE_LENGTH];
	filep = (u1*)(int)fhandler->data[FILE_POINTER];
	
	if (position < fileLength) {
		available = fileLength - position;
		if (available < len) {
			readBytes = available;
		} else {
			readBytes = len;
		}
		memcpy(buf, (filep + position), readBytes * sizeof(u1));
		position += readBytes;
	} else {
		return -1; /* EOF */
	}

	/* Update access position */
	fhandler->data[FILE_POS] = position;
	
	return readBytes;
}

/*
 * Class:     com_sun_cldc_io_ResourceInputStream
 * Method:    clone
 * Signature: (Ljava/lang/Object;)Ljava/lang/Object;
 */
JNIEXPORT struct java_lang_Object* JNICALL Java_com_sun_cldc_io_ResourceInputStream_clone(JNIEnv *env, jclass clazz, struct java_lang_Object* jobj) {
	
	java_longarray *srcArray;
	java_longarray *dstArray;
	int i;
	
	srcArray = (java_longarray *) jobj;
	
	if (!(dstArray = (java_longarray *) builtin_newarray_int(FILE_INFOS)))
			return NULL;

	for (i = 0; i < FILE_INFOS; i++) {
		dstArray->data[i] = srcArray->data[i];
	}
	
	return (java_lang_Object*) dstArray;

}


/*
 * These are local overrides for various environment variables in Emacs.
 * Please do not remove this and leave it at the end of the file, where
 * Emacs will automagically detect them.
 * ---------------------------------------------------------------------
 * Local variables:
 * mode: c
 * indent-tabs-mode: t
 * c-basic-offset: 4
 * tab-width: 4
 * End:
 * vim:noexpandtab:sw=4:ts=4:
 */
