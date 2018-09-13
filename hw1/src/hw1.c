#include <stdlib.h>
#include <stdio.h>

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

int myA2I(const char *str){
    int i=0;
    int myInt=0;
    int intInASCII;
    while( *(str+i)!=0){
        intInASCII = *(str+i);
        myInt *=10;
        myInt += (intInASCII - '0');
        i++;
    }
    //print/f("%i\n",myInt );
    return myInt;
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
int validargs(int argc, char **argv){
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
                unsigned long factor = myA2I(((*(argv+currentArgPosition))));
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
                // printf("%s\n","it's 'p'" )

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
                // printf("%s\n","it's 'k'" );

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

                // printf("%s\n","is alnum" );

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

    ("The value of i is %d, and its address is %p\n", i, &i);
    */
    // ("successful\n");
    // printf("%016lx\n", global_options);
    return 1;
}

int charToHex(char c){
    int hexVal;
    if((c>='0' && c<='9'))
                    {
                        hexVal = c-'0';
                    }
                    else if((c>='a' && c<='f')){
                        hexVal = c-'a'+10;
                    }
                     else if((c>='A' && c<='F')){
                        hexVal = c-'A'+10;
                    }
                    else{
                        return 0;
                    }
    return hexVal;
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

    static AUDIO_HEADER header;
    AUDIO_HEADER *hp = &header;

    // = malloc(sizeof(AUDIO_HEADER));

    // if read header or read annotation return 0, return 0
    if(!read_header(hp)){
        return 0;
    }

    // annotation size = data offset - header size
    int annotation_size = (hp->data_offset)-24;
    //output_annotation_pos, aka output annotation size
    int output_annotation_pos = 0;

    //read annoatation from input file and store at array input_annotation
    if (read_annotation(input_annotation,annotation_size)){

        //if has annotation, write  ' ' seperated audible line with \n
        //then audible command line wiht \0 terminator.

        //position of arg in argv
        int argPos =0;
        //current char traversed
        char currentChar;

        //loop through argv
        while(*(argv+argPos)!=NULL){
            // printf("outerLoop:%d\n",argPos );

            // position of char in arg
            int charPos = 0;

            //add ' ' between args,
            //this is done by skipping the first arg then append ' ' in front of each arg
            if(argPos!= 0){
            *(output_annotation+output_annotation_pos) = ' ';
            output_annotation_pos++;
            }

            //iterate through each arg string
            while( ( currentChar =*(*(argv+argPos)+charPos))  != '\0'){
                // printf("inner loop:%d\n",charPos );

                //store char read to output_annotation array
                *(output_annotation+output_annotation_pos) = currentChar;
                charPos++;
                output_annotation_pos++;
            }
            argPos++;
        }

        // add '\n' no matter what, apparently
        *(output_annotation+output_annotation_pos) = '\n';
        output_annotation_pos++;

        //to make sure annotation is actually ended with '\0'
        if(annotation_size==0){
            *(output_annotation+output_annotation_pos) = '\0';
            output_annotation_pos++;
        }

        //offset must divisible by 8,append \0 to annotation until that's true
        while(output_annotation_pos%8 != 0){

            *(output_annotation+ (++output_annotation_pos)) = '\0';
        }
        if(annotation_size+output_annotation_pos>=ANNOTATION_MAX){
            return 0;
        }
        hp->data_offset = 24 + annotation_size+output_annotation_pos;
    }
    //return 0 if failed to read annoatation
    else{
        return 0;
    }

    /* modify data_size based on speed up/down*/

    //factor to speed up | slow down
    unsigned int factor = ((global_options>>48) & 0x3ff )+1;
    //byte per sample
    int encoding_bytes =  ((hp->encoding)-1);
    //num of sample per frame
    int channels = hp -> channels;

    // frame size = byte per sample * num of sample
    unsigned int num_frame = hp->data_size / (encoding_bytes*channels);

    //data size changes depends on -u -d factor
    if(global_options & 1ul<<62){
        if(num_frame % factor==0){
            hp->data_size /= factor;
        }
        // if num of frame is not multiple of factor, then there's an extra frame
        else{
            // printf("data_size %i\n",hp->data_size );
            // printf("num_frame %i\n",num_frame );
            hp->data_size = (num_frame/2 +1)* encoding_bytes*channels;

        }

    }
    //slow down, size increase,((frame#-1)*factor+1)*frame size
    else if(global_options & 1ul<<61){

        hp->data_size -= encoding_bytes*channels;
        hp->data_size *=factor;
        hp->data_size += encoding_bytes*channels;

    }
    write_header(hp);
    write_annotation(output_annotation,output_annotation_pos);
    write_annotation(input_annotation,annotation_size);

    /**
    (bit 62) is 1 if -u
    (i.e. the user wants speed-up mode).
    (bit 61) is 1 if -d
    (i.e. the user wants slow-down mode).
     (bit 60) is 1 if -c
    (i.e. the user wants crypt mode).
    (bit 59) is 1 if -p
    (i.e. the user wants annotations left unmodified).
    -f  then the factor (minus one) is recorded (bits 57 - 48)
    the secret key is recorded  (bits 31 - 0).
    If the -k option was not specified, then these bits of global_options
    should all be zero.
    */

    int index = 0;
    int * previous_frame_int = ((int*)previous_frame);
    int * input_frame_int = ((int*)input_frame);
    int * output_frame_int = ((int*)output_frame);

    // printf("%d\n",factor );

    //initialize my rand generator
    unsigned int seed = global_options & 0xFFFFFFFF;
    mysrand(seed);
    //read frame by frame,exit loop when read frame return 0, aka end of file
    // printf("%s\n","read_frame" );
    while(read_frame(input_frame_int,channels,encoding_bytes)){
        // printf("%s\n","calc ing offset" );
        // offset = index* ((int)hp->channels);
        // printf("%s\n", "clac ing current_frame_p");
        // current_frame_p = ((int*)input_frame)+offset;

        // if '-u',speed up, by change the factor
        if(global_options & 1ul<<62){
            // printf("%s\n","is -u" );
            // writing by multiple of factor
            if (index++%factor==0)
            {
                // printf("%s\n","write_frame" );
                write_frame(input_frame_int,channels,encoding_bytes);
            }

        }
        // if '-d',slow down, by inserting N-1 frame in between every two frames.
        //@detail: store the previous frame, write frame (N-1) times before write current frame,
        //store new frame into output frame and write it
        else if(global_options & 1ul<<61){
            //slow down
            if(index++!=0){
                for (int i = 1; i < factor; i++)
                {
                    for (int channel = 0; channel < channels; ++channel)
                    {
                        //make new frame using S + (T - S)*k/N, depends on # of cannals
                        int new_sample =
                            *(previous_frame_int+channel)+
                            (*(input_frame_int+channel) - *(previous_frame_int+channel)) * i / factor;
                        *(output_frame_int + channel) = new_sample;
                    }
                    write_frame(output_frame_int,channels,encoding_bytes );
                }
            }
            write_frame(input_frame_int,channels,encoding_bytes);

            for (int channel = 0; channel < channels; ++channel)
            {
                //make new frame using S + (T - S)*k/N, depends on # of cannals
                int new_sample = *(input_frame_int+channel);
                *(previous_frame_int + channel) = new_sample;
            }
        }
        //crpt
        else if(global_options & 1ul<<60){

            for (int channel = 0; channel < channels; ++channel)
            {
                //make new frame using S + (T - S)*k/N, depends on # of cannals
                int new_sample = (*(input_frame_int+channel));
                *(input_frame_int + channel) = new_sample ^ myrand32();
            }
            write_frame(input_frame_int,channels,encoding_bytes );
        }

        // if(global_options & 1ul<<62){
        //     //speed up
        // }

    }


    // int frame = 0 ;
    // int *fp = &frame;

    // int bytes_per_sample =  ((hp->encoding)-1);
    // // printf("%i\n", bytes_per_sample );
    // if(read_frame(fp,hp->channels,bytes_per_sample)){
    //     write_frame(fp,hp->channels,((hp->encoding)-1));
    // }


    return 1;
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
    // printf("%s\n","reading file" );

    unsigned int intValue ;
    unsigned int inputChar ;

     /*
    store byte by byte
    does not work, header_start+i adds 4*i not i
    */

    // char *header_start = (char*)&(hp->magic_number);


    // for(int i = 0;i<24;i++){
    //     inputChar = getchar();
    //     *(header_start+i) = inputChar;
    // }

    /* store int by int */

    unsigned int *header_start = &(hp->magic_number);
    for(int j = 0;j<6;j++){
        for(int i = 0;i<4;i++){
            inputChar = getchar();
            intValue <<= 8;
            intValue |= inputChar;
        }
        // printf("%x\n",j );
        // printf("address of value:%p\n",header_start+(j) );
        *(header_start+j) = intValue;

    }

    // printf("%08x\n",hp->magic_number);
    // printf("%d\n",hp->data_offset);
    // printf("%d\n",hp->data_size);
    // printf("%d\n",hp->encoding );
    // printf("%08x\n",hp->sample_rate );
    // printf("%d\n",hp->channels );

    unsigned int magic_number;//must be 0x2e736e64
    unsigned int data_offset;
    // unsigned int data_size;
    unsigned int encoding;
    // unsigned int sample_rate;
    unsigned int channels;

    magic_number = (*hp).magic_number;
    data_offset = (*hp).data_offset;
    // data_size = (*hp).data_size;
    encoding = (*hp).encoding;
    // sample_rate = (*hp).sample_rate;
    channels = (*hp).channels;


    // printf("%08x\n",hp->magic_number);
    if(magic_number != AUDIO_MAGIC){
        // printf("%s\n","number not magic" );
        return 0;
    }

    if (data_offset % 8 != 0){
        return 0;
    }
    if(encoding<PCM8_ENCODING || encoding>PCM32_ENCODING){
        return 0;
    }
    if(channels<1 || channels>CHANNELS_MAX){
        return 0;
    }

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
    // printf("%s\n","writing header" );

    AUDIO_HEADER header = *hp;
    //reading
    unsigned int intValue;
    unsigned int outChar;
    unsigned int *header_start = &(header.magic_number);

    //for each members in the struct
    for(int j = 0;j<6;j++){
        //incredment pointer to next member and get value

        // printf("address of value:%p\n",header_start+j );
        intValue = *(header_start+j);
        // printf("%08x\n", intValue );
        // printf("%x\n",header.magic_number );
        for(int i = 0;i<4;i++){
            outChar =  intValue >> (8*(3-i));
            printf("%c",outChar );
        }
        // printf("%x\n",j );
        // printf("address of value:%p\n",header_start+j );
    }

    // magic_number = (*hp).magic_number;
    // data_offset = (*hp).data_offset;
    // data_size = (*hp).data_size;
    // encoding = (*hp).encoding;
    // sample_rate = (*hp).sample_rate;
    // channels = (*hp).channels;
    return 1;
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
int read_annotation(char *ap, unsigned int size){
    // printf("%s\n","reading annotation" );
    // if size is 0, no annotation provided, true by default
    if(size==0){
        return 1;

    }
    //size cannot exceed ANNOTATION_MAX
    if(size> ANNOTATION_MAX)
        return 0;


    unsigned int inputChar ;
    /*
    store byte by byte
    does not work, header_start+i adds 4*i not i
    */
    for(int i = 0;i<size;i++){
        inputChar = getchar();
        *(ap+i) = inputChar;
        if(i==(size-1) && inputChar==0)
            return 1;
    }

    return 0;

}

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
int write_annotation(char *ap, unsigned int size){

    unsigned int outChar;

    //print byte by byte
    for(int i = 0;i<size;i++){
        outChar = *(ap+i);
        printf("%c", outChar);
    }

    return 1;
}

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
int read_frame(int *fp, int channels, int bytes_per_sample){

    // printf("%s\n","read_frame" );

    int inputChar ;
    int intValue  ;
    /*
    store byte by byte
    does not work, header_start+i adds 4*i not i
    */
    // for(int i=0;i < channels*bytes_per_sample;i++){
    //         inputChar = getchar();
    //         *(fp+i) = inputChar;
    //  }
    // printf("%i\n", channels);
    // printf("%i\n",bytes_per_sample );
    for(int j = 0;j<channels;j++){
        intValue = 0;
        for(int i = 0;i<bytes_per_sample;i++){
            inputChar = getchar();
            if(inputChar==EOF)
                return 0;
            // printf("gotchar:%x\n",inputChar );
            intValue <<= 8;
            intValue |= inputChar;
        }
        // printf("intValue:%x\n",intValue );
        // printf("%x\n",j );
        // printf("address of value:%p\n",header_start+(j) );
        *(fp+j) = intValue;
        // printf("%i\n",*(fp+j) );

    }

    return 1;
}

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
int write_frame(int *fp, int channels, int bytes_per_sample){
    int outChar ;
    int intValue ;
    /*
    store byte by byte
    does not work, header_start+i adds 4*i not i
    */
    // for(int i=0;i < channels*bytes_per_sample;i++){
    //         outChar = *(fp+i);
    //         printf("%c\n", outChar );

    //  }

    // printf("%i\n", channels);
    // printf("%i\n",bytes_per_sample );

    for(int j = 0;j<channels;j++){

        // printf("address of value:%p\n",header_start+j );
        intValue = *(fp+j);
        // printf("intValue:%08x\n", intValue );
        // printf("%x\n",header.magic_number );
        for(int i = 0;i < bytes_per_sample;i++){
            outChar =  intValue >> (8*(bytes_per_sample-1-i));
            // printf("%d\n",i );

            printf("%c",outChar);
            // printf("outChar:%x\n",outChar);
        }
        // printf("%x\n",j );
        // printf("address of value:%p\n",header_start+j );
    }

    return 1;
}
