/*
 *  Copyright (c) 2015 David Yuste Romero
 *
 *  THIS MATERIAL IS PROVIDED AS IS, WITH ABSOLUTELY NO WARRANTY EXPRESSED
 *  OR IMPLIED.  ANY USE IS AT YOUR OWN RISK.
 *
 *  Permission is hereby granted to use or copy this program
 *  for any purpose,  provided the above notices are retained on all copies.
 *  Permission to modify the code and to distribute modified code is granted,
 *  provided the above notices are retained, and a notice that the code was
 *  modified is included with the above copyright notice.
 */
#ifndef YUCODE_CURL_CRYPT_H
#define YUCODE_CURL_CRYPT_H

#include <pthread.h>
#include <openssl/crypto.h>

namespace curl {
namespace crypt {
	
	static pthread_mutex_t *LockArray;

	static void LockCallback(int mode, int type, const char *file, int line)
	{
		(void)file;
		(void)line;
		if (mode & CRYPTO_LOCK) {
			pthread_mutex_lock(&(LockArray[type]));
		}
		else {
			pthread_mutex_unlock(&(LockArray[type]));
		}
	}
	
	static unsigned long ThreadId(void)
	{
		unsigned long ret;
		
		ret=(unsigned long)pthread_self();
		return(ret);
	}
	
	static void InitLocks(void)
	{
		int i;
		
		LockArray=(pthread_mutex_t *)OPENSSL_malloc(CRYPTO_num_locks() *
							sizeof(pthread_mutex_t));
		for (i=0; i<CRYPTO_num_locks(); i++) {
		pthread_mutex_init(&(LockArray[i]),NULL);
		}
		
		CRYPTO_set_id_callback(ThreadId);
		CRYPTO_set_locking_callback(LockCallback);
	}
	
	static void KillLocks(void)
	{
		int i;
		
		CRYPTO_set_locking_callback(NULL);
		for (i=0; i<CRYPTO_num_locks(); i++)
		pthread_mutex_destroy(&(LockArray[i]));
		
		OPENSSL_free(LockArray);
	}
}
}

#endif
