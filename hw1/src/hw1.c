#include <stdlib.h>

#include "debug.h"
#include "hw1.h"

#ifdef _STRING_H
#error "Do not #include <string.h>. You will get a ZERO."
#endif

#ifdef _STRINGS_H
#error "Do not #include <strings.h>. You will get a ZERO."
#endif

#ifdef _CTYPE_H
#error "Do not #include <ctype.h>. You will get a ZERO."
#endif

/*
 * You may modify this file and/or move the functions contained here
 * to other source files (except for main.c) as you wish.
 *
 * IMPORTANT: You MAY NOT use any array brackets (i.e. [ and ]) and
 * you MAY NOT declare any arrays or allocate any storage with malloc().
 * The purpose of this restriction is to force you to use pointers.
 * Variables to hold the content of three frames of audio data and
 * two annotation fields have been pre-declared for you in const.h.
 * You must use those variables, rather than declaring your own.
 * IF YOU VIOLATE THIS RESTRICTION, YOU WILL GET A ZERO!
 *
 * IMPORTANT: You MAY NOT use floating point arithmetic or declare
 * any "float" or "double" variables.  IF YOU VIOLATE THIS RESTRICTION,
 * YOU WILL GET A ZERO!
 */
int checkChar(char **argv, int row, int col, char charToMatch){
    if(*(*(argv+row)+col)==charToMatch)
        return 1;
    else
        return 0;
}
/**
 * @brief Validates command line arguments passed to the program.
 * @details This function will validate all the arguments passed to the
 * program, returning 1 if validation succeeds and 0 if validation fails.
 * Upon successful return, the selected program options will be set in the
 * global variables "global_options", where they will be accessible
 * elsewhere in the program.
 *
 * @param argc The number of arguments passed to the program from the CLI.
 * @param argv The argument strings passed to the program from the CLI.
 * @return 1 if validation succeeds and 0 if validation fails.
 * Refer to the homework document for the effects of this function on
 * global variables.
 * @modifies global variable "global_options" to contain a bitmap representing
 * the selected options.
 */
int validargs(int argc, char **argv)
{
    int currentArgPosition = 1;
    global_options = 0;

    //too few args
    if(argc<=1)
        return 0;

    // printf("%c\n",*(*(argv+1)));
    // printf("%c\n",*(*(argv+1)+1));

    //true if starts with '-h'
    if(checkChar(argv,currentArgPosition,0,'-') && checkChar(argv,currentArgPosition,1,'h') && checkChar(argv,currentArgPosition,2,'\0'))
    {
          // printf("starts with '-h'\n");
          global_options |=((unsigned long)1)<<63;
          printf("%lx\n",global_options );
          return 1;
    }
    //too many args
    if(argc>5)
        return 0;

    //if stats with 'u' or 'd'
    if (checkChar(argv,currentArgPosition,0,'-')
        && (checkChar(argv,currentArgPosition,1,'u')|| checkChar(argv,currentArgPosition,1,'d') )
        && checkChar(argv,currentArgPosition,2,'\0') )
    {
        //position = 2
        currentArgPosition++;
        //check the rest of the args
        while(currentArgPosition<argc){
            // printf("%d\n",currentArgPosition );
            //if it is 'f', then must followed by a int
            if(checkChar(argv,currentArgPosition,0,'-')
                && checkChar(argv,currentArgPosition,1,'f')
                && checkChar(argv,currentArgPosition,2,'\0')){

                if(currentArgPosition++>=argc){
                    return 0;
                }
                //the next input must be a int, atoi takes a char pointer and convert to int, return 0 if not int
                unsigned long factor = atoi(((*(argv+currentArgPosition))));
                // printf("%d\n",atoi(((*(argv+currentArgPosition))) ));

                if(factor <1 || factor>1024){
                    return 0;
                }
                global_options = global_options | ((factor-1)<<48);
                // unsigned int factorHex =  (unsigned int)(*(argv+currentArgPosition));
                // printf("hex value: %lx\n", factor);

            }
            //if it is 'p'
            else if(checkChar(argv,currentArgPosition,0,'-')
                && checkChar(argv,currentArgPosition,1,'p')
                && checkChar(argv,currentArgPosition,2,'\0')){
                global_options = global_options | 1ul<<59;
                printf("%s\n","it's 'p'" );
            }
            else{
                return 0;
            }

            currentArgPosition++;
            // printf("end with %d\n",currentArgPosition );
        }

        if(checkChar(argv,1,1,'u')){
            global_options |= 1ul<<62;
        }
        else{
            // printf("%lx\n",global_options );
            global_options |= 1ul<<61;
        }
        // printf("%lx\n",global_options );
    }

    //if starts with '-c'
    else if (checkChar(argv,currentArgPosition,0,'-')
        && checkChar(argv,currentArgPosition,1,'c')
        && checkChar(argv,currentArgPosition,2,'\0') )
    {
        //position = 2
        currentArgPosition++;

        //check the rest of the args
        while(currentArgPosition<argc){
            // printf("%d\n",currentArgPosition );
            //if it is '-k', then must followed by a alphaNum
            if(checkChar(argv,currentArgPosition,0,'-')
                && checkChar(argv,currentArgPosition,1,'k')
                && checkChar(argv,currentArgPosition,2,'\0')){
                printf("%s\n","it's 'k'" );
                currentArgPosition++;
                if(currentArgPosition>=argc){
                    return 0;
                }
                //the next input must be alphaNum, must be len<=8
                int i = 0;
                char currentChar;
                unsigned long  key = 0 ;
                unsigned long  keyFragment ;
                do{
                    currentChar = *(*(argv+currentArgPosition)+i);
                    // printf("char:%c\n",currentChar );
                    if(currentChar == '\0'){
                        break;
                    }

                    // key=key <<4;
                    // int keyFragment = (int)strtol(*(argv+currentArgPosition), NULL, 16);
                    // printf("int:%d\n",keyFragment );
                    // key|=keyFragment;
                    // printf("%c\n",currentChar );
                    //return 0 if is not alphaNum
                    // if( !((currentChar>='0' && currentChar<='9')
                    //     || (currentChar>='a' && currentChar<='f')
                    //     || (currentChar>='A' && currentChar<='F')) ){
                    //     return 0;
                    // }
                    if((currentChar>='0' && currentChar<='9'))
                    {
                        keyFragment = currentChar-'0';
                    }
                    else if((currentChar>='a' && currentChar<='f')){
                        keyFragment = currentChar-'a'+10;
                    }
                     else if((currentChar>='A' && currentChar<='F')){
                        keyFragment = currentChar-'A'+10;
                    }
                    else{
                        return 0;
                    }
                    key <<=4;
                    key |=keyFragment;
                    // if len>8
                    if(i++>8){
                        return 0;
                    }
                }while(currentChar != '\0');
                // key = (int)strtol(*(argv+currentArgPosition), NULL, 16);
                // printf("%lx\n", key );
                global_options |= key;

                printf("%s\n","is alnum" );

            }
            //if it is 'p'
            else if(checkChar(argv,currentArgPosition,0,'-')
                && checkChar(argv,currentArgPosition,1,'p')
                && checkChar(argv,currentArgPosition,2,'\0')){
                global_options = global_options | 1ul<<59;
                // printf("%s\n","it's 'p'" );
            }
            else{
                return 0;
            }
            currentArgPosition++;
        }
        global_options |=  1ul<<60;
    }
    else{
        return 0;
    }

    /*printf("a long uses %lu bytes of memory\n", sizeof(long));
    printf("a char uses %lu bytes of memory\n", sizeof(char));
    printf("a int uses %lu bytes of memory\n", sizeof(int));
    printf("a float uses %lu bytes of memory\n", sizeof(float));
    int i = 10;

    printf("The value of i is %d, and its address is %p\n", i, &i);
    */
    printf("successful\n");
    printf("%016lx\n", global_options);
    return 1;
}

/**
 * @brief  Recodes a Sun audio (.au) format audio stream, reading the stream
 * from standard input and writing the recoded stream to standard output.
 * @details  This function reads a sequence of bytes from the standard
 * input and interprets it as digital audio according to the Sun audio
 * (.au) format.  A selected transformation (determined by the global variable
 * "global_options") is applied to the audio stream and the transformed stream
 * is written to the standard output, again according to Sun audio format.
 *
 * @param  argv  Command-line arguments, for constructing modified annotation.
 * @return 1 if the recoding completed successfully, 0 otherwise.
 */
int recode(char **argv) {
    AUDIO_HEADER header = {0x2e736e64,80,3,3,2,1} ;
    AUDIO_HEADER *noob = &header;


    // header.magic_number = **argv;
    // header.data_offset = *(*argv)+4;
    // header.data_size =  *(*argv)+8;
    // header.encoding = *(*argv)+12;
    // header.sample_rate = *(*argv)+16;
    // header.channels = *(*argv)+20;

    int answer = read_header(noob);

    printf("audio header is %d\n",answer);
    return 0;
}

/**
 * @brief Read the header of a Sun audio file and check it for validity.
 * @details  This function reads 24 bytes of data from the standard input and
 * interprets it as the header of a Sun audio file.  The data is decoded into
 * six unsigned int values, assuming big-endian byte order.   The decoded values
 * are stored into the AUDIO_HEADER structure pointed at by hp.
 * The header is then checked for validity, which means:  no error occurred
 * while reading the header data,
    the magic number is valid,
    the data offset is a multiple of 8,
    the value of encoding field is one of {2, 3, 4, 5},
    the value of the channels field is one of {1, 2}.
 *
 * @param hp  A pointer to the AUDIO_HEADER structure that is to receive
 * the data.
 * @return  1 if a valid header was read, otherwise 0.
 */
int read_header(AUDIO_HEADER *hp){
    printf("%s\n","reading header" );
    unsigned int magic_number;//must be 0x2e736e64
    unsigned int data_offset;
    unsigned int data_size;
    unsigned int encoding;
    unsigned int sample_rate;
    unsigned int channels;

    magic_number = (*hp).magic_number;
    data_offset = (*hp).data_offset;
    // data_size = (*hp).data_size;
    encoding = (*hp).encoding;
    // sample_rate = (*hp).sample_rate;
    channels = (*hp).channels;

    if(magic_number !=  0x2e736e64){
        return 0;
    }
    if (data_offset % 8 != 0){
        return 0;
    }
    if(encoding<2 || encoding>5){
        return 0;
    }
    if(channels<1 || channels>2){
        return 0;
    }


    // printf("%d\n",headerFragment );

    return 1;

}
/**
 * @brief  Write the header of a Sun audio file to the standard output.
 * @details  This function takes the pointer to the AUDIO_HEADER structure passed
 * as an argument, encodes this header into 24 bytes of data according to the Sun
 * audio file format specifications, and writes this data to the standard output.
 *
 * @param  hp  A pointer to the AUDIO_HEADER structure that is to be output.
 * @return  1 if the function is successful at writing the data; otherwise 0.
 */
int write_header(AUDIO_HEADER *hp){
    printf("%s\n","writing header" );
    unsigned int magic_number;//must be 0x2e736e64
    unsigned int data_offset;
    unsigned int data_size;
    unsigned int encoding;
    unsigned int sample_rate;
    unsigned int channels;

    magic_number = (*hp).magic_number;
    data_offset = (*hp).data_offset;
    data_size = (*hp).data_size;
    encoding = (*hp).encoding;
    sample_rate = (*hp).sample_rate;
    channels = (*hp).channels;
    return 0;
}

/**
 * @brief  Read annotation data for a Sun audio file from the standard input,
 * storing the contents in a specified buffer.
 * @details  This function takes a pointer 'ap' to a buffer capable of holding at
 * least 'size' characters, and it reads 'size' characters from the standard input,
 * storing the characters read in the specified buffer.  It is checked that the
 * data read is terminated by at least one null ('\0') byte.
 *
 * @param  ap  A pointer to the buffer that is to receive the annotation data.
 * @param  size  The number of bytes of data to be read.
 * @return  1 if 'size' bytes of valid annotation data were successfully read;
 * otherwise 0.
 */
int read_annotation(char *ap, unsigned int size);

/**
 * @brief  Write annotation data for a Sun audio file to the standard output.
 * @details  This function takes a pointer 'ap' to a buffer containing 'size'
 * characters, and it writes 'size' characters from that buffer to the standard
 * output.
 *
 * @param  ap  A pointer to the buffer containing the annotation data to be
 * written.
 * @param  size  The number of bytes of data to be written.
 * @return  1 if 'size' bytes of data were successfully written; otherwise 0.
 */
int write_annotation(char *ap, unsigned int size);

/**
 * @brief Read, from the standard input, a single frame of audio data having
 * a specified number of channels and bytes per sample.
 * @details  This function takes a pointer 'fp' to a buffer having sufficient
 * space to hold 'channels' values of type 'int', it reads
 * 'channels * bytes_per_sample' data bytes from the standard input,
 * interpreting each successive set of 'bytes_per_sample' data bytes as
 * the big-endian representation of a signed integer sample value, and it
 * stores the decoded sample values into the specified buffer.
 *
 * @param  fp  A pointer to the buffer that is to receive the decoded sample
 * values.
 * @param  channels  The number of channels.
 * @param  bytes_per_sample  The number of bytes per sample.
 * @return  1 if a complete frame was read without error; otherwise 0.
 */
int read_frame(int *fp, int channels, int bytes_per_sample);

/**
 * @brief  Write, to the standard output, a single frame of audio data having
 * a specified number of channels and bytes per sample.
 * @details  This function takes a pointer 'fp' to a buffer that contains
 * 'channels' values of type 'int', and it writes these data values to the
 * standard output using big-endian byte order, resulting in a total of
 * 'channels * bytes_per_sample' data bytes written.
 *
 * @param  fp  A pointer to the buffer that contains the sample values to
 * be written.
 * @param  channels  The number of channels.
 * @param  bytes_per_sample  The number of bytes per sample.
 * @return  1 if the complete frame was written without error; otherwise 0.
 */
int write_frame(int *fp, int channels, int bytes_per_sample);
