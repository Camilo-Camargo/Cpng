#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <assert.h>
#include <math.h>

#define BUF_INT_SIZE 5
#define BUF_LONG_SIZE 9

/* Signatures */
const char Signature_key[]  = {137, 80, 78, 71, 13, 10, 26, 10}; 
#define SIGNATURE_SIZE  8
const char Signature_IHDR[] = {73,72,68,82};
const char Signature_PLTE[] = {80,76,84,69};
const char Signature_IDAT[] = {73,68,65,84};
const char Signature_IEND[] = {73,69,78,68};
#define IHDR_SIZE 4
 
int little_to_big_int(int little){
	int lsb = (little & 0xff000000) >> 24u;
	int b1  = (little & 0x00ff0000) >> 8u;
	int b2  = (little & 0x0000ff00) << 8u;
	int msb = (little & 0x000000ff) << 24u;  

	return msb | b2 | b1 | lsb;
}

void debug(char *msg);

void print_byte(char *str, size_t n){   
	int i = n;
	while(i--){ 
		unsigned char number = (unsigned char)(*str++);
		printf("%u ", number);   
	} 
	printf("\n");
}  

void read_bytes(FILE *file, unsigned int n, char *buffer){ 
	fread(buffer, sizeof(char), n, file);
	char buffer_format[1024];
	sprintf(buffer_format,"read %u bytes.", n);
	debug(buffer_format);
}     

unsigned char read_ubyte(FILE *file){
		unsigned char value;
		fread(&value, sizeof(unsigned char), 1, file);
		debug("read 1 byte (unsigned).");
		return value;
}

void signature_check(char *signature){
	if(memcmp(signature, Signature_key, sizeof(Signature_key))){
		fprintf(stderr, "Incorrect signature.\n");
		exit(1);	
	}	
	debug("signature correct"); 
} 

int read_int(FILE *file){
	int value;
	fread(&value, sizeof(int), 1, file);
	debug("read 4 byte.");
	return little_to_big_int(value);
}
 
int read_uint(FILE *file){
	unsigned int value;
	fread(&value, sizeof(unsigned int), 1, file);
	return little_to_big_int(value);
}  

int read_chunk_type(FILE *img_file, const char* signature, char *buf4){
	read_bytes(img_file, 4, buf4); 
	if(memcmp(buf4, signature, 4)){
		debug("Signature failed");
		return -1;
	} 
	return 0;
}

void read_IHDR(FILE *img_file){
	char buf4[5];
	/* read chunk layout */  
	// length
	int length = read_uint(img_file);
	// chunk type   
	read_chunk_type(img_file, Signature_IHDR, buf4);
	debug("IHDR passed");
	// chunk data 
		//width  4 bytes  
		int width = read_int(img_file);
		//height 4 bytes 
		int height = read_uint(img_file); 
		printf("size: %dx%d\n", width, height);
		//bit depth
		unsigned char bit_depth = read_ubyte(img_file);
		//compression_method
		unsigned char compression_method = read_ubyte(img_file);
		//filter method
		unsigned char filter_method = read_ubyte(img_file); 
		//interlace method
		unsigned char interlace_method = read_ubyte(img_file);
	// crc 
	int crc = read_int(img_file);
	printf("Length: %d\n", length);
	printf("Chunk type: %s\n", buf4);
	printf("\t width: %d\n", width);
	printf("\t height: %d\n", height);
	printf("\t bit_depth: %d\n", bit_depth);
	printf("\t compression_method: %d\n", compression_method);
	printf("\t interlace_method: %d\n", interlace_method);
	printf("\t crc: %d\n", crc);
	printf("--------------------------------------\n"); 
}   

void read_PLTE(FILE *img_file){
	char buf4[5]; 

	/* read chunk layout */  
	// length
	int length = read_uint(img_file);
	// chunk type    
	// restore the pointer of the file
	if(read_chunk_type(img_file, Signature_PLTE, buf4)){ 
		fseek(img_file, -7, SEEK_CUR);
		return;
	}

	debug("PLTE passed"); 
	printf("Length: %d\n", length);
	printf("Chunk type: %s\n", buf4); 
	printf("--------------------------------------\n"); 
}

int read_IDAT(FILE *img_file){
	char buf4[5]; 

	/* read chunk layout */  
	// length 
	int length = read_uint(img_file);
	// chunk type    
	// restore the pointer of the file 
	if(read_chunk_type(img_file, Signature_IDAT, buf4) == -1){ 
		fseek(img_file, -7, SEEK_CUR);
		return -1;
	}

	debug("IDAT passed"); 
	printf("Length: %d\n", length);
	printf("Chunk type: %s\n", buf4); 
	int crc = read_int(img_file);
	printf("\t crc: %d\n", crc);
	printf("--------------------------------------\n"); 
	return length;
}

int read_IEND(FILE *img_file){
	char buf4[5]; 

	/* read chunk layout */  
	// length
	int length = read_uint(img_file);
	// chunk type    
	// restore the pointer of the file
	if(read_chunk_type(img_file, Signature_IEND, buf4)){ 
		return -1;
	}
	debug("IEND passed"); 
	printf("Length: %d\n", length);
	printf("Chunk type: %s\n", buf4); 
	printf("--------------------------------------\n"); 
	return length;
}
 
int main(int argc, char **argv){
	printf("---------------- SIMPLE PNG SCANNER -------------------\n"); 
	FILE * img_file = fopen("img.png", "rwb");
	char signature[BUF_LONG_SIZE]; 
	read_bytes(img_file, SIGNATURE_SIZE, signature);
	signature_check(signature);  

	read_IHDR(img_file); 
	read_PLTE(img_file);   
	int data_length;  

	while((data_length = read_IDAT(img_file))!= -1){
		fseek(img_file, data_length-1, SEEK_CUR);			
	}

	read_IEND(img_file);
}  

void debug(char *msg){
#ifdef DEBUG
	printf("[DEBUG] %s\n", msg); 
#endif
}


