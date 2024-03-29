/*
---- A DYNAMIC Huffman (FGK) Coding Implementation ----

Filename:     FGKD.C	(Decoder)
Written by:   Gerald R. Tamayo
Date:         2005/2023/2024

This is the decompressor for
	FGKC.C, an *Adaptive* Huffman (FGK) Algorithm implementation.

To compile:   tcc -w fgkd.c gtbitio.c huf.c
              bcc32 -w fgkd.c gtbitio.c huf.c
              g++ -O2 fgkd.c gtbitio.c huf.c -s -o fgkd
*/
#include <stdio.h>
#include <stdlib.h>
#include "gtbitio.h"
#include "fgk.c"

typedef struct {
	char algorithm[4];
	unsigned long file_size;
} file_stamp;

void copyright( void );

int main( int argc, char *argv[] )
{
	unsigned long in_file_len = 0, out_file_len = 0;
	file_stamp fstamp;
	
	if ( argc != 3 ) {
		fprintf(stderr, "\n Usage: fgkd infile outfile");
		copyright();
		return 0;
	}

	if ( (gIN = fopen( argv[1], "rb" )) == NULL ) {
		fprintf(stderr, "\nError opening input file.");
		copyright();
		return 0;
	}
	if ( (pOUT = fopen( argv[2], "wb" )) == NULL ) {
		fprintf(stderr, "\nError opening output file.");
		copyright();
		return 0;
	}
	init_put_buffer();
	
	fprintf(stderr, "\n---- A DYNAMIC Huffman (FGK) Implementation ----\n");
	fprintf(stderr, "\nName of input file : %s", argv[1] );
	fprintf(stderr, "\nName of output file: %s", argv[2] );
	
	/* get file length. */
	fseek( gIN, 0, SEEK_END );
	in_file_len = ftell( gIN );
	
	/* ===== The Main Huffman Implementation ======= */
	
	fprintf(stderr, "\n\nHuffman decompressing...");
	
	/* This is a DYNAMIC algorithm, so no need to read stats. */
	
	/* start the decoding process. */
	rewind( gIN );
	
	/* read first the file stamp/header. */
	fread( &fstamp, sizeof(file_stamp), 1, gIN );
	if ( fstamp.file_size == 0 ) goto done_decompressing;
	
	/* make sure all symbol node
			addresses are NULL. */
	init_hufflist();
	
	/* get FIRST symbol. */
	hc = fgetc( gIN );
	out_file_len = fstamp.file_size - 1;
	
	/*
	create first 0-node which quickly becomes the root
	of the Huffman tree.
	*/
	top = zero_node = create_node();
	
	/* output first symbol. */
	pfputc( (unsigned char) hc );
	
	/* Update the Huffman tree. */
	update_treeFGK( hc );	/* pass the symbol. */
	
	/* now get the bit stream. */
	init_get_buffer();

	while ( out_file_len-- ) {
		hc = hdecompress( top );

		if ( hc == ZERO_NODE_SYMBOL ) {	/* unseen byte. */
			/* get raw byte. */
			hc = get_nbits( 8 );
		}
		
		/* output the decoded byte. */
		pfputc( (unsigned char) hc );
		
		/* update the Huffman tree. */
		update_treeFGK( hc );	/* pass the symbol. */
	}
	flush_put_buffer();
	
	done_decompressing:
	
	fprintf(stderr, "done.");
	
	/* get outfile's size. */
	out_file_len = ftell( pOUT );
	
	fprintf(stderr, "\n\nLength of input file     = %15lu bytes", in_file_len );
	fprintf(stderr, "\nLength of output file    = %15lu bytes\n", out_file_len );
	
	free_get_buffer();
	free_put_buffer();
	if ( gIN ) fclose( gIN );
	if ( pOUT ) fclose( pOUT );
	return 0;
}

void copyright( void )
{
	fprintf(stderr, "\n\n Written by: Gerald R. Tamayo, 2005-2024\n");
}
