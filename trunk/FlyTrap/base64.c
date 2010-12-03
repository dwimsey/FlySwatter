/*! @internal
The following code for Base64 encoding and decoding operations is licensed with the following license.
This license applies ONLY to the Base64 code taken from b64.c originally, it does not apply to any other parts of this
file.

Copyright (c) 2001 Bob Trower, Trantor Standard Systems Inc.

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated
documentation files (the "Software"), to deal in the
Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute,
sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall
be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS
OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/*
** Translation Table as described in RFC1113
*/
static const char cb64[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/*
** Translation Table to decode (created by author)
*/
static const char cd64[]="|$$$}rstuvwxyz{$$$$$$$>?@ABCDEFGHIJKLMNOPQRSTUVW$$$$$$XYZ[\\]^_`abcdefghijklmnopq";

/*
** base64encodeblock
**
** encode 3 8-bit binary bytes as 4 '6-bit' characters
*/
void base64encodeblock( unsigned char in[3], unsigned char out[4], int len )
{
    out[0] = cb64[ in[0] >> 2 ];
    out[1] = cb64[ ((in[0] & 0x03) << 4) | ((in[1] & 0xf0) >> 4) ];
    out[2] = (unsigned char) (len > 1 ? cb64[ ((in[1] & 0x0f) << 2) | ((in[2] & 0xc0) >> 6) ] : '=');
    out[3] = (unsigned char) (len > 2 ? cb64[ in[2] & 0x3f ] : '=');
}

/*
** base64decodeblock
**
** decode 4 '6-bit' characters into 3 8-bit binary bytes
*/
void base64decodeblock( unsigned char in[4], unsigned char out[3] )
{   
    out[ 0 ] = (unsigned char ) (in[0] << 2 | in[1] >> 4);
    out[ 1 ] = (unsigned char ) (in[1] << 4 | in[2] >> 2);
    out[ 2 ] = (unsigned char ) (((in[2] << 6) & 0xc0) | in[3]);
}

#ifdef BASE64_FILEIO_SUPPORTED
/*
** base64encode_fstream
**
** base64 encode a stream adding padding and line breaks as per spec.
*/
void base64encode_fstream(FILE *infile, FILE *outfile, int linesize )
{
    unsigned char in[3], out[4];
    int i, len, blocksout = 0;

    while( !feof( infile ) ) {
        len = 0;
        for( i = 0; i < 3; i++ ) {
            in[i] = (unsigned char) getc( infile );
            if( !feof( infile ) ) {
                len++;
            }
            else {
                in[i] = 0;
            }
        }
        if( len ) {
            base64encodeblock( in, out, len );
            for( i = 0; i < 4; i++ ) {
                putc( out[i], outfile );
            }
            blocksout++;
        }
        if( blocksout >= (linesize/4) || feof( infile ) ) {
            if( blocksout ) {
                fprintf( outfile, "\r\n" );
            }
            blocksout = 0;
        }
    }
}

/*
** base64decode_fstream
**
** decode a base64 encoded stream discarding padding, line breaks and noise
*/
void base64decode_fstream( FILE *infile, FILE *outfile )
{
    unsigned char in[4], out[3], v;
    int i, len;

    while( !feof( infile ) ) {
        for( len = 0, i = 0; i < 4 && !feof( infile ); i++ ) {
            v = 0;
            while( !feof( infile ) && v == 0 ) {
                v = (unsigned char) getc( infile );
                v = (unsigned char) ((v < 43 || v > 122) ? 0 : cd64[ v - 43 ]);
                if( v ) {
                    v = (unsigned char) ((v == '$') ? 0 : v - 61);
                }
            }
            if( !feof( infile ) ) {
                len++;
                if( v ) {
                    in[ i ] = (unsigned char) (v - 1);
                }
            }
            else {
                in[i] = 0;
            }
        }
        if( len ) {
            base64decodeblock( in, out );
            for( i = 0; i < len - 1; i++ ) {
                putc( out[i], outfile );
            }
        }
    }
}

#endif // !BASE64_FILEIO_SUPPORTED

/*
	This is the end of the Base64 encoding/decoding functions written by Bob Trower and the license for
	that code applies to no other parts of this file.
*/

#ifndef _CRT_SECURE_NO_WARNINGS
// we don't want this warnings, they really provide little usefulness and we're aware of how to code properly for them
#define _CRT_SECURE_NO_WARNINGS 1
#endif

#ifndef _CRT_NONSTDC_NO_DEPRECATE
// we don't care about these warnings either, we prefer compatibility with other compilers and OSes
#define _CRT_NONSTDC_NO_DEPRECATE 1
#endif


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

char *b64append(const char *inbuf, const char* format)
{
	char *retVal = NULL;

	if(inbuf == NULL) {
		return(strdup(format));
	}
	retVal = (char*)calloc((strlen(inbuf) + strlen(format) + 2), sizeof(char));
	if(retVal == NULL) {
		// couldn't allocate a new string, abort early
		return(NULL);
	}
	strcpy(retVal, inbuf);
	strcat(retVal, format);
	free((void*)inbuf);
	return(retVal);
}

/*
** base64encode
**
** base64 encode a block of memory adding padding and line breaks as per spec.
*/
unsigned char *base64encode(unsigned char *inBuf, size_t inSize, int lineSize)
{
	unsigned char *nextIn, *nextOut;
	int blockLen;
	int encodedSize = (inSize * 4)/3+4;
	unsigned char *outBuf;
	if(lineSize < 1) {
		outBuf = calloc(encodedSize + 2, sizeof(char));
	} else {
		outBuf = calloc((encodedSize + ((encodedSize / lineSize) * 3)) + 2, sizeof(char));
	}


	nextIn = inBuf;
	nextOut = outBuf;
	while((unsigned int)(nextIn-inBuf) < (inSize-2)) {
		base64encodeblock(nextIn, nextOut, 3);
		nextIn += 3;
		nextOut += 4;
		if(lineSize > 0) {
//			if((((nextOut-outBuf) % lineSize)+1) == lineSize) {
//				nextOut[0] = '\r';
//				nextOut[1] = '\n';
//				nextOut += 2;
//			}
		}
	}
		
	blockLen = (nextIn - inBuf) - (inSize - 2);
	if(blockLen > 0) {
		base64encodeblock(nextIn, nextOut, blockLen);
		nextIn += blockLen;
		nextOut += 4;
	}
	return(outBuf);
	}

/*
** base64decode
**
** decode a base64 encoded memory buffer discarding padding, line breaks and noise
*/
/*
void base64decode( FILE *infile, FILE *outfile )
{
    unsigned char in[4], out[3], v;
    int i, len;

    while( !feof( infile ) ) {
        for( len = 0, i = 0; i < 4 && !feof( infile ); i++ ) {
            v = 0;
            while( !feof( infile ) && v == 0 ) {
                v = (unsigned char) getc( infile );
                v = (unsigned char) ((v < 43 || v > 122) ? 0 : cd64[ v - 43 ]);
                if( v ) {
                    v = (unsigned char) ((v == '$') ? 0 : v - 61);
                }
            }
            if( !feof( infile ) ) {
                len++;
                if( v ) {
                    in[ i ] = (unsigned char) (v - 1);
                }
            }
            else {
                in[i] = 0;
            }
        }
        if( len ) {
            base64decodeblock( in, out );
            for( i = 0; i < len - 1; i++ ) {
                putc( out[i], outfile );
            }
        }
    }
}
*/