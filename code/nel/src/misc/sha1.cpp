// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

//
// Includes
//

#include "stdmisc.h"

#include "nel/misc/sha1.h"
#include "nel/misc/file.h"
#include "nel/misc/path.h"

//
// Namespaces
//

using namespace std;
using namespace NLMISC;

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

//
// Types
//

/*
 * If you do not have the ISO standard stdint.h header file, then you
 * must typedef the following:
 *    name              meaning
 *  uint32_t         unsigned 32 bit integer
 *  uint8_t          unsigned 8 bit integer (i.e., unsigned char)
 *  int_least16_t    integer of >= 16 bits
 *
 */

typedef unsigned int uint32_t;
typedef unsigned char uint8_t;
typedef short int_least16_t;

//
// Constantes
//

static const int bufferSize = 100000;
static uint8_t buffer[bufferSize];

enum
{
    shaSuccess = 0,
    shaNull,            /* Null pointer parameter */
    shaInputTooLong,    /* input data too long */
    shaStateError       /* called Input after Result */
};

#define SHA1HashSize 20

/*
 *  This structure will hold context information for the SHA-1
 *  hashing operation
 */
typedef struct SHA1Context
{
    uint32_t Intermediate_Hash[SHA1HashSize/4]; /* Message Digest  */

    uint32_t Length_Low;            /* Message length in bits      */
    uint32_t Length_High;           /* Message length in bits      */

                               /* Index into message block array   */
    int_least16_t Message_Block_Index;
    uint8_t Message_Block[64];      /* 512-bit message blocks      */

    int Computed;               /* Is the digest computed?         */
    int Corrupted;             /* Is the message digest corrupted? */
} SHA1Context;

//
//  Function Prototypes
//

int SHA1Reset (SHA1Context *);
int SHA1Input (SHA1Context *, const uint8_t *, unsigned int);
int SHA1Result (SHA1Context *, uint8_t Message_Digest[SHA1HashSize]);

//
// Functions
//

CHashKey getSHA1(const uint8 *buffer, uint32 size)
{
    SHA1Context sha;
    int err;
    uint8_t Message_Digest[20];

	err = SHA1Reset(&sha);
	if (err)
	{
		nlwarning("SHA: SHA1Reset Error %d.\n", err );
		return CHashKey();
	}

	err = SHA1Input(&sha, (const uint8_t*)buffer, size);
	if (err)
	{
		nlwarning ("SHA: SHA1Input Error %d.\n", err);
		return CHashKey();
	}

	err = SHA1Result(&sha, Message_Digest);
	if (err)
	{
		nlwarning("SHA: SHA1Result Error %d, could not compute message digest.\n", err );
		return CHashKey();
	}

	CHashKey hk (Message_Digest);
	return hk;
}

CHashKey getSHA1(const string &filename, bool forcePath)
{
    SHA1Context sha;
    int err;
    uint8_t Message_Digest[20];

	//printf("reading '%s'\n", findData.cFileName);
	CIFile ifile;
	ifile.setCacheFileOnOpen(true);
	if (!ifile.open(forcePath ? filename:CPath::lookup(filename)))
	{
		nlwarning ("SHA: Can't open the file '%s'", filename.c_str());
		return CHashKey();
	}

	//FILE *fp = fopen (filename.c_str(), "rb");
	//if (fp == NULL) return CHashKey();

	err = SHA1Reset(&sha);
	if (err)
	{
		nlwarning("SHA: SHA1Reset Error %d.\n", err );
		ifile.close ();
		return CHashKey();
	}

	sint fs = ifile.getFileSize();
	sint n, read = 0;
	do
	{
		//bs = (int)fread (buffer, 1, bufferSize, fp);
		n = std::min (bufferSize, fs-read);
		//nlinfo ("read %d bytes", n);
		ifile.serialBuffer((uint8 *)buffer, n);

		err = SHA1Input(&sha, buffer, n);
		if (err)
		{
			nlwarning ("SHA: SHA1Input Error %d.\n", err);
			ifile.close();
			return CHashKey();
		}
		read += n;
	}
	while (!ifile.eof());

	ifile.close	();

	err = SHA1Result(&sha, Message_Digest);
	if (err)
	{
		nlwarning("SHA: SHA1Result Error %d, could not compute message digest.\n", err );
		return CHashKey();
	}

	CHashKey hk (Message_Digest);
	return hk;
}

/*
*
* HMAC = hash( (Key ^ 0x5c) .. hash( (Key ^0x36) .. Message ) )
*
*/

CHashKey getHMacSHA1(const uint8 *text, uint32 text_len, const uint8 *key, uint32 key_len)
{
	SHA1Context sha;

	uint8_t SHA1_Key[64];
	uint8_t SHA1_Key1[20];
	uint8_t SHA1_Key2[20];

	string buffer1;
	string buffer2;

	// Init some vars
	for (uint i = 0; i <  64; i++)
		SHA1_Key[i] = 0;

	// If lenght of key > 64 use sha1 hash
	if (key_len > 64) {
		uint8_t SHA1_Key0[20];
		SHA1Reset(&sha);
		SHA1Input(&sha, (const uint8_t*)key, key_len);
		SHA1Result(&sha, SHA1_Key0);
		CHashKey hk0 (SHA1_Key0);
		for (uint i = 0; i <  20; i++)
			SHA1_Key[i] = hk0.HashKeyString[i];
	} else {
		for (uint i = 0; i < key_len; i++)
			SHA1_Key[i] = key[i];
	}

	// Do 0x36 XOR Key
	for (uint i = 0; i < 64; i++)
		buffer1 += 0x36 ^ SHA1_Key[i];

	// Append text
	for (uint i = 0; i < text_len; i++)
		buffer1 += text[i];

	// Get hash
	SHA1Reset(&sha);
	SHA1Input(&sha, (const uint8_t*)buffer1.c_str(), (uint)buffer1.size());
	SHA1Result(&sha, SHA1_Key1);
	CHashKey hk1 (SHA1_Key1);

	// Do 0x5c XOR Key
	for (uint i = 0; i < 64; i++)
		buffer2 += 0x5c ^ SHA1_Key[i];

	// Append previous hash
	for (uint i = 0; i < 20; i++)
		buffer2 += hk1.HashKeyString[i];

	// Get new hash
	SHA1Reset(&sha);
	SHA1Input(&sha, (const uint8_t*)buffer2.c_str(), (uint)buffer2.size());
	SHA1Result(&sha, SHA1_Key2);
	CHashKey hk (SHA1_Key2);

	return hk;
}


#ifdef _MFC_VER
	#pragma runtime_checks( "", off )
#endif

/*
 *  Define the SHA1 circular left shift macro
 */
#define SHA1CircularShift(bits,word) \
                (((word) << (bits)) | ((word) >> (32-(bits))))

/* Local Function Prototypes */
void SHA1PadMessage(SHA1Context *);
void SHA1ProcessMessageBlock(SHA1Context *);

/*
 *  SHA1Reset
 *
 *  Description:
 *      This function will initialize the SHA1Context in preparation
 *      for computing a new SHA1 message digest.
 *
 *  Parameters:
 *      context: [in/out]
 *          The context to reset.
 *
 *  Returns:
 *      sha Error Code.
 *
 */
int SHA1Reset(SHA1Context *context)
{
    if (!context)
    {
        return shaNull;
    }

    context->Length_Low             = 0;
    context->Length_High            = 0;
    context->Message_Block_Index    = 0;

    context->Intermediate_Hash[0]   = 0x67452301;
    context->Intermediate_Hash[1]   = 0xEFCDAB89;
    context->Intermediate_Hash[2]   = 0x98BADCFE;
    context->Intermediate_Hash[3]   = 0x10325476;
    context->Intermediate_Hash[4]   = 0xC3D2E1F0;

    context->Computed   = 0;
    context->Corrupted  = 0;

    return shaSuccess;
}

/*
 *  SHA1Result
 *
 *  Description:
 *      This function will return the 160-bit message digest into the
 *      Message_Digest array  provided by the caller.
 *      NOTE: The first octet of hash is stored in the 0th element,
 *            the last octet of hash in the 19th element.
 *
 *  Parameters:
 *      context: [in/out]
 *          The context to use to calculate the SHA-1 hash.
 *      Message_Digest: [out]
 *          Where the digest is returned.
 *
 *  Returns:
 *      sha Error Code.
 *
 */
int SHA1Result( SHA1Context *context,
                uint8_t Message_Digest[SHA1HashSize])
{
    int i;

    if (!context || !Message_Digest)
    {
        return shaNull;
    }

    if (context->Corrupted)
    {
        return context->Corrupted;
    }

    if (!context->Computed)
    {
        SHA1PadMessage(context);
        for(i=0; i<64; ++i)
        {
            /* message may be sensitive, clear it out */
            context->Message_Block[i] = 0;
        }
        context->Length_Low = 0;    /* and clear length */
        context->Length_High = 0;
        context->Computed = 1;

    }

    for(i = 0; i < SHA1HashSize; ++i)
    {
        Message_Digest[i] = uint8_t((context->Intermediate_Hash[i>>2]
                            >> 8 * ( 3 - ( i & 0x03 ) ) ) & 0xff );
    }

    return shaSuccess;
}

/*
 *  SHA1Input
 *
 *  Description:
 *      This function accepts an array of octets as the next portion
 *      of the message.
 *
 *  Parameters:
 *      context: [in/out]
 *          The SHA context to update
 *      message_array: [in]
 *          An array of characters representing the next portion of
 *          the message.
 *      length: [in]
 *          The length of the message in message_array
 *
 *  Returns:
 *      sha Error Code.
 *
 */
int SHA1Input(    SHA1Context    *context,
                  const uint8_t  *message_array,
                  unsigned       length)
{
    if (!length)
    {
        return shaSuccess;
    }

    if (!context || !message_array)
    {
        return shaNull;
    }

    if (context->Computed)
    {
        context->Corrupted = shaStateError;

        return shaStateError;
    }

    if (context->Corrupted)
    {
         return context->Corrupted;
    }
    while(length-- && !context->Corrupted)
    {
    context->Message_Block[context->Message_Block_Index++] =
                    (*message_array & 0xFF);

    context->Length_Low += 8;
    if (context->Length_Low == 0)
    {
        context->Length_High++;
        if (context->Length_High == 0)
        {
            /* Message is too long */
            context->Corrupted = 1;
        }
    }

    if (context->Message_Block_Index == 64)
    {
        SHA1ProcessMessageBlock(context);
    }

    message_array++;
    }

    return shaSuccess;
}

/*
 *  SHA1ProcessMessageBlock
 *
 *  Description:
 *      This function will process the next 512 bits of the message
 *      stored in the Message_Block array.
 *
 *  Parameters:
 *      None.
 *
 *  Returns:
 *      Nothing.
 *
 *  Comments:

 *      Many of the variable names in this code, especially the
 *      single character names, were used because those were the
 *      names used in the publication.
 *
 *
 */
void SHA1ProcessMessageBlock(SHA1Context *context)
{
    const uint32_t K[] =    {       /* Constants defined in SHA-1   */
                            0x5A827999,
                            0x6ED9EBA1,
                            0x8F1BBCDC,
                            0xCA62C1D6
                            };
    int           t;                 /* Loop counter                */
    uint32_t      temp;              /* Temporary word value        */
    uint32_t      W[80];             /* Word sequence               */
    uint32_t      A, B, C, D, E;     /* Word buffers                */

    /*
     *  Initialize the first 16 words in the array W
     */
    for(t = 0; t < 16; t++)
    {
        W[t] = context->Message_Block[t * 4] << 24;
        W[t] |= context->Message_Block[t * 4 + 1] << 16;
        W[t] |= context->Message_Block[t * 4 + 2] << 8;
        W[t] |= context->Message_Block[t * 4 + 3];
    }

    for(t = 16; t < 80; t++)
    {
       W[t] = SHA1CircularShift(1,W[t-3] ^ W[t-8] ^ W[t-14] ^ W[t-16]);
    }

    A = context->Intermediate_Hash[0];
    B = context->Intermediate_Hash[1];
    C = context->Intermediate_Hash[2];
    D = context->Intermediate_Hash[3];
    E = context->Intermediate_Hash[4];

    for(t = 0; t < 20; t++)
    {
        temp =  SHA1CircularShift(5,A) +
                ((B & C) | ((~B) & D)) + E + W[t] + K[0];
        E = D;
        D = C;
        C = SHA1CircularShift(30,B);

        B = A;
        A = temp;
    }

    for(t = 20; t < 40; t++)
    {
        temp = SHA1CircularShift(5,A) + (B ^ C ^ D) + E + W[t] + K[1];
        E = D;
        D = C;
        C = SHA1CircularShift(30,B);
        B = A;
        A = temp;
    }

    for(t = 40; t < 60; t++)
    {
        temp = SHA1CircularShift(5,A) +
               ((B & C) | (B & D) | (C & D)) + E + W[t] + K[2];
        E = D;
        D = C;
        C = SHA1CircularShift(30,B);
        B = A;
        A = temp;
    }

    for(t = 60; t < 80; t++)
    {
        temp = SHA1CircularShift(5,A) + (B ^ C ^ D) + E + W[t] + K[3];
        E = D;
        D = C;
        C = SHA1CircularShift(30,B);
        B = A;
        A = temp;
    }

    context->Intermediate_Hash[0] += A;
    context->Intermediate_Hash[1] += B;
    context->Intermediate_Hash[2] += C;
    context->Intermediate_Hash[3] += D;
    context->Intermediate_Hash[4] += E;

    context->Message_Block_Index = 0;
}

/*
 *  SHA1PadMessage
 *

 *  Description:
 *      According to the standard, the message must be padded to an even
 *      512 bits.  The first padding bit must be a '1'.  The last 64
 *      bits represent the length of the original message.  All bits in
 *      between should be 0.  This function will pad the message
 *      according to those rules by filling the Message_Block array
 *      accordingly.  It will also call the ProcessMessageBlock function
 *      provided appropriately.  When it returns, it can be assumed that
 *      the message digest has been computed.
 *
 *  Parameters:
 *      context: [in/out]
 *          The context to pad
 *      ProcessMessageBlock: [in]
 *          The appropriate SHA*ProcessMessageBlock function
 *  Returns:
 *      Nothing.
 *
 */

void SHA1PadMessage(SHA1Context *context)
{
    /*
     *  Check to see if the current message block is too small to hold
     *  the initial padding bits and length.  If so, we will pad the
     *  block, process it, and then continue padding into a second
     *  block.
     */
    if (context->Message_Block_Index > 55)
    {
        context->Message_Block[context->Message_Block_Index++] = 0x80;
        while(context->Message_Block_Index < 64)
        {
            context->Message_Block[context->Message_Block_Index++] = 0;
        }

        SHA1ProcessMessageBlock(context);

        while(context->Message_Block_Index < 56)
        {
            context->Message_Block[context->Message_Block_Index++] = 0;
        }
    }
    else
    {
        context->Message_Block[context->Message_Block_Index++] = 0x80;
        while(context->Message_Block_Index < 56)
        {

            context->Message_Block[context->Message_Block_Index++] = 0;
        }
    }

	/*
	*  Store the message length as the last 8 octets
	*/
	context->Message_Block[56] = uint8_t((context->Length_High >> 24)&0xff);
	context->Message_Block[57] = uint8_t((context->Length_High >> 16)&0xff);
	context->Message_Block[58] = uint8_t((context->Length_High >> 8)&0xff);
	context->Message_Block[59] = uint8_t((context->Length_High)&0xff);
	context->Message_Block[60] = uint8_t((context->Length_Low >> 24)&0xff);
	context->Message_Block[61] = uint8_t((context->Length_Low >> 16)&0xff);
	context->Message_Block[62] = uint8_t((context->Length_Low >> 8)&0xff);
	context->Message_Block[63] = uint8_t((context->Length_Low)&0xff);

	SHA1ProcessMessageBlock(context);
}
