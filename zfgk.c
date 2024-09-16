/*
	---- A DYNAMIC Huffman Coding Implementation ----

	Filename:     ZFGK.C	(Encoder/Decoder)
	Written by:   Gerald R. Tamayo
	Date:         2005/2023, September 16, 2024
	
	To compile:   g++ -O3 zfgk.c gtbitio2.c huf2.c -s -o zfgk
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "gtbitio2.h"
#include "fgk2.c"

enum {
	/* modes */
	COMPRESS,
	DECOMPRESS,
};

typedef struct {
	char algorithm[4];
	int64_t file_size;
} file_stamp;

int64_t out_file_len = 0;
file_stamp fstamp;

void copyright( void );

void usage( void )
{
	fprintf(stderr, "\n Usage: zfgk c|d infile outfile\n"
		"\n Commands:\n  c = compression \n  d = decompression\n"
	);
	copyright();
	exit(0);
}

void compress_fgk ( void )
{
	/* start huffman encoding the symbols/file bytes. */
	
	/* get first symbol. */
	hc = gfgetc();
	if ( hc == EOF ) return;
	
	/* make sure all symbol node
		addresses are NULL. */
	init_hufflist();
	
	/*
	create first 0-node which quickly becomes
	the root of the tree.
	*/
	top = zero_node = create_node();
	
	/* output first symbol as a raw byte. */
	fputc( (unsigned char) hc, pOUT );
	++nbytes_out;
	
	/* recompute the tree if necessary. */
	update_treeFGK( hc );  /* pass the symbol. */
	
	while ( (hc=gfgetc()) != EOF ) {
		/* encode the byte hc. */
		if ( hufflist[ hc ] ){   /* VALID address; previously seen. */
			/* output the symbol's code. */
			hcompress( hufflist[ hc ] );
		}
		else {	/* NULL address, a new byte! */
			/* output the zero node's code. */
			hcompress( zero_node );
			
			/* send the actual byte. */
			put_nbits( hc, 8 );
		}
		
		/* recompute the tree if necessary. */
		update_treeFGK( hc );  /* pass the symbol. */
	}
}

void decompress_fgk ( void )
{
	/* make sure all symbol node
		addresses are NULL. */
	init_hufflist();
	
	/* get FIRST symbol. */
	hc = fgetc( gIN );
	if ( hc == EOF ) return;
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
	nbytes_read += sizeof(file_stamp)+1;
	
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
}

int main( int argc, char *argv[] )
{
	float ratio = 0.0;
	int mode = -1;
	
	clock_t start_time = clock();
	
	if ( argc != 4 ) usage();
	init_buffer_sizes( (1<<15) );
	
	if ( tolower( argv[1][0]) == 'c' ) mode = COMPRESS;
	else if ( tolower( argv[1][0]) == 'd' ) mode = DECOMPRESS;
	else usage();
	
	if ( (gIN=fopen( argv[2], "rb" )) == NULL ) {
		fprintf(stderr, "\nError opening input file.");
		return 0;
	}
	if ( (pOUT=fopen( argv[3], "wb" )) == NULL ) {
		fprintf(stderr, "\nError opening output file.");
		return 0;
	}
	init_put_buffer();
	
	if ( mode == COMPRESS ){
		fprintf(stderr, "\n ---- A DYNAMIC Huffman (FGK) Implementation ----\n");
		fprintf(stderr, "\n Encoding [ %s to %s ] ...", argv[2], argv[3] );
		
		/* encode FILE STAMP. */
		rewind( pOUT );
		strcpy( fstamp.algorithm, "FGK" );
		fstamp.file_size = 0;  /* initial write. */
		fwrite( &fstamp, sizeof(file_stamp), 1, pOUT );
		nbytes_out = sizeof(file_stamp);
		
		init_get_buffer();
		
		/* compress */
		compress_fgk();
	}
	else if ( mode == DECOMPRESS ){
		fprintf(stderr, "\n Decoding...");
		
		/* read first the file stamp/header. */
		fread( &fstamp, sizeof(file_stamp), 1, gIN );
		
		/* decompress */
		decompress_fgk();
	}
	flush_put_buffer();
	
	nbytes_read = get_nbytes_read();
	
	if ( mode == COMPRESS ){
		/* re-Write the FILE STAMP. */
		rewind( pOUT );
		fstamp.file_size = nbytes_read;  /* write actual input file size. */
		fwrite( &fstamp, sizeof(file_stamp), 1, pOUT );
	}
	
	fprintf(stderr, "done.\n  %s (%lld) -> %s (%lld)",
		argv[2], nbytes_read, argv[3], nbytes_out);
	if ( mode == COMPRESS ) {
		ratio = (((float) nbytes_read - (float) nbytes_out) /
			(float) nbytes_read ) * (float) 100;
		fprintf(stderr, "\n Compression ratio: %3.2f %%", ratio );
	}
	fprintf(stderr, " in %3.2f secs.\n", (double) (clock()-start_time) / CLOCKS_PER_SEC );
	
	free_get_buffer();
	free_put_buffer();
	if ( gIN ) fclose( gIN );
	if ( pOUT ) fclose( pOUT );
	return 0;
}

void copyright( void )
{
	fprintf(stderr, "\n Gerald R. Tamayo (c) 2024\n");
}
