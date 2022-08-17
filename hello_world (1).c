/*
 * "Hello World" example.
 *
 * This example prints 'Hello from Nios II' to the STDOUT stream. It runs on
 * the Nios II 'standard', 'full_featured', 'fast', and 'low_cost' example
 * designs. It runs with or without the MicroC/OS-II RTOS and requires a STDOUT
 * device in your system's hardware.
 * The memory footprint of this hosted application is ~69 kbytes by default
 * using the standard reference design.
 *
 * For a reduced footprint version of this template, and an explanation of how
 * to reduce the memory footprint for a given application, see the
 * "small_hello_world" template.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <system.h>
#include <sys/alt_alarm.h>
#include <io.h>

#include "fatfs.h"
#include "diskio.h"

#include "ff.h"
#include "monitor.h"
#include "uart.h"

#include "alt_types.h"

#include <altera_up_avalon_audio.h>
#include <altera_up_avalon_audio_and_video_config.h>

#include "altera_avalon_pio_regs.h"
#include "sys/alt_irq.h"

uint8_t Buff[8192000] __attribute__ ((aligned(4)));  /* Working buffer */
uint8_t Buff2[1024] __attribute__ ((aligned(4)));  /* Working buffer */

alt_up_audio_dev * audio_dev;

int prev_down = 0;
int stop_down = 0;
int pause_play_down = 0;
int next_down = 0;

int prev_up = 0;
int stop_up = 0;
int pause_play_up = 0;
int next_up = 0;

int stopped = 1;

int i = 0;
int cnt;

#define _VOLUMES  1

uint8_t Buff[8192000] __attribute__ ((aligned(4)));
FATFS Fatfs[_VOLUMES];
FILINFO Finfo;
FATFS *fs;
FILE *display;
FIL File1;
char filenames[20][20];
unsigned long filesize[20];

char mode[20];
int tracknum = 1;

char *ptr = "";
DIR Dir;
long p1;
uint8_t res = 0;
uint32_t s1, s2 = sizeof(Buff);

static void BTN_ISR(void* context, alt_u32 id){
	IOWR(BUTTON_PIO_BASE, 2, 0x0);
	IOWR(TIMER_0_BASE, 2, 0xE360);
	IOWR(TIMER_0_BASE, 3, 0x16);
	IOWR(TIMER_0_BASE, 1, 0x5);
}

static void TIMER_ISR(void* context, alt_u32 id){
	//read button
	int button_state = IORD(BUTTON_PIO_BASE, 0);
		//FIRST RUN -- BUTTON PRESSED
	switch (button_state)
	{
	case 7:
		prev_down = 1;
		IOWR(LED_PIO_BASE, 0, 0x1);
		break;
	case 11:
		stop_down = 1;
		IOWR(LED_PIO_BASE, 0, 0x1);
		break;
	case 13:
		pause_play_down = 1;
		IOWR(LED_PIO_BASE, 0, 0x1);
		break;
	case 14:
		next_down = 1;
		IOWR(LED_PIO_BASE, 0, 0x1);
		break;
	case 15:
		//MAYBE CHANGE THIS IF QUALITY IS BAD:
		if (prev_down == 1) {
			prev_up = 1;
		} else if (stop_down == 1) {
			stop_up = 1;
		} else if (pause_play_down == 1) {
			pause_play_up = 1;
		} else if (next_down == 1) {
			next_up = 1;
		} else {
			// no buttons were pushed down before i guess we set all button down and button up to 0
			next_up = 0;
			pause_play_up = 0;
			stop_up = 0;
			prev_up = 0;
		}
		IOWR(LED_PIO_BASE, 0, 0x0);
		break;
	default:
		//set all button downs to 0
		next_down = 0;
		pause_play_down = 0;
		stop_down = 0;
		prev_down = 0;
	}

	IOWR(TIMER_0_BASE, 1, 0x0);
	IOWR(BUTTON_PIO_BASE, 3, 0x0);
	IOWR(BUTTON_PIO_BASE, 2, 0x0F);
//
//
//
//
//
//
//		if (button_state == 7) { //prev
//			prev_down = 1;
//			IOWR(LED_PIO_BASE, 0, 0x1);
//		} else if (button_state == 11) { //stop
//			stop_down = 1;
//			IOWR(LED_PIO_BASE, 0, 0x1);
//		} else if (button_state == 13) { // pause/play
//			pause_play_down = 1;
//			IOWR(LED_PIO_BASE, 0, 0x1);
//		} else if (button_state == 14) { //next
//			next_down = 1;
//			IOWR(LED_PIO_BASE, 0, 0x1);
//		} else if (button_state == 15) { //SECOND RUN -- BUTTON RELEASED
//			if (prev_down == 1) {
//				prev_up = 1;
//			} else if (stop_down == 1) {
//				stop_up = 1;
//			} else if (pause_play_down == 1) {
//				pause_play_up = 1;
//			} else if (next_down == 1) {
//				next_up = 1;
//			} else {
//				// no buttons were pushed down before i guess we set all button down and button up to 0
//				next_up = 0;
//				pause_play_up = 0;
//				stop_up = 0;
//				prev_up = 0;
//			}
//			IOWR(LED_PIO_BASE, 0, 0x0);
//		} else { //no correct input detected
//			//set all button downs to 0
//			next_down = 0;
//			pause_play_down = 0;
//			stop_down = 0;
//			prev_down = 0;
//		}
//
//	IOWR(TIMER_0_BASE, 1, 0x0);
//	IOWR(BUTTON_PIO_BASE, 3, 0x0);
//	IOWR(BUTTON_PIO_BASE, 2, 0x0F);
}

    void put_rc(FRESULT rc)
    {
        const char *str =
            "OK\0" "DISK_ERR\0" "INT_ERR\0" "NOT_READY\0" "NO_FILE\0" "NO_PATH\0"
            "INVALID_NAME\0" "DENIED\0" "EXIST\0" "INVALID_OBJECT\0" "WRITE_PROTECTED\0"
            "INVALID_DRIVE\0" "NOT_ENABLED\0" "NO_FILE_SYSTEM\0" "MKFS_ABORTED\0" "TIMEOUT\0"
            "LOCKED\0" "NOT_ENOUGH_CORE\0" "TOO_MANY_OPEN_FILES\0";
        FRESULT i;

        for (i = 0; i != rc && *str; i++) {
            while (*str++);
        }
        //xprintf("rc=%u FR_%s\n", (uint32_t) rc, str);
    }

int isWav(char *filename) {
	//char lastfour[10] = "";
	char wavchar[10] = ".wav";
	char wavchar2[10] = ".WAV";

	int length = strlen(filename);

	char *ptr = filename + (length - 4);

	return (!strcmp(ptr, wavchar) || !strcmp(ptr, wavchar2));
}

void play() {

	if (stopped) {
		//maybe something here
		return;
	}


    int i = 0;
    int j = 0;



    unsigned int l_buf; //512 (hakf if 1024 of other buffer)
    unsigned int r_buf;


//    if ( audio_dev == NULL)
//        alt_printf ("Error: could not open audio device \n");
//    else
//        alt_printf ("Opened audio device \n");
                    	               /* read and echo audio data */





    							if ((uint32_t) p1 >= 1024)
    							{
    								cnt = 1024;
    								//i = 32;
    								p1 -= 1024;
    							}
    							else
    							{
    								//printf("%d and cnt = %d", p1, cnt);
    								cnt = p1;
    								//i = p1;
    								p1 = 0;
    							}
    							res = f_read(&File1, Buff2, cnt, &s1);
    							if (res != FR_OK)
    							{
    								put_rc(res);
    								return;
    							}
    							if (!cnt)
    							{
    								//set stop to 1
    								strcpy(mode, "STOPPED");
    								fprintf(display, "%c%s", 27, "[2j");
    								fprintf(display, "%d. %s\n%s\n", tracknum, filenames[tracknum - 1], mode);
    								f_close(&File1);
    								stopped = 1;
    								return;
    							}

    							//While the input buffer is not empty
    							//int *index = Buff2;

    							unsigned int speed = IORD(SWITCH_PIO_BASE, 0) & 0x3;
    							switch (speed) {
    							case 1:
       								for (i=0; i<cnt; i+=4) {
        									//unsigned int l_buf; //512 (hakf if 1024 of other buffer)
        									//unsigned int r_buf;
        									for (j = 0; j <2; j+=1) {
        										l_buf = Buff2[i] | Buff2[i+1] << 8;
        										r_buf = Buff2[i+2] | Buff2[i+3] << 8;

        										//2 Bytes to left output buffer
        										int fifospace = alt_up_audio_write_fifo_space (audio_dev, ALT_UP_AUDIO_RIGHT);
        										if ( fifospace > 0 ) // check if data is available CHECK THAT WE HAVENT READ ALL BYTES
        										{
        											alt_up_audio_write_fifo (audio_dev, &r_buf, 1, ALT_UP_AUDIO_RIGHT);
        											alt_up_audio_write_fifo (audio_dev, &l_buf, 1, ALT_UP_AUDIO_LEFT);
        										} else {
        											j -= 1;
        										}
        									}
        								}
    								break;
    							case 2:
    								for (i=0; i<cnt; i+=8) {
    									l_buf = Buff2[i] | Buff2[i+1] << 8;
    									r_buf = Buff2[i+2] | Buff2[i+3] << 8;

    									//2 Bytes to left output buffer
    									int fifospace = alt_up_audio_write_fifo_space (audio_dev, ALT_UP_AUDIO_RIGHT);
    								    if ( fifospace > 0 ) // check if data is available CHECK THAT WE HAVENT READ ALL BYTES
    								    {
    								    	alt_up_audio_write_fifo (audio_dev, &r_buf, 1, ALT_UP_AUDIO_RIGHT);
    								        alt_up_audio_write_fifo (audio_dev, &l_buf, 1, ALT_UP_AUDIO_LEFT);
    								     } else {
    								        i -= 8;
    								     }
    								    //i+=4;
    								}
    								break;
    							case 3:
    								for (i=0; i<cnt; i+=4) {
    									//l_buf = Buff2[i] | Buff2[i+1] << 8;
    									r_buf = Buff2[i+2] | Buff2[i+3] << 8;

    									//2 Bytes to left output buffer
    									int fifospace = alt_up_audio_write_fifo_space (audio_dev, ALT_UP_AUDIO_RIGHT);
    								    if ( fifospace > 0 ) // check if data is available CHECK THAT WE HAVENT READ ALL BYTES
    								    {
    								    	alt_up_audio_write_fifo (audio_dev, &r_buf, 1, ALT_UP_AUDIO_RIGHT);
    								        alt_up_audio_write_fifo (audio_dev, &r_buf, 1, ALT_UP_AUDIO_LEFT);
    								     } else {
    								        i -= 4;
    								     }
    								    //i+=4;
    								}
    								break;
    							default:
    								for (i=0; i<cnt; i+=4) {
    									//unsigned int l_buf; //512 (hakf if 1024 of other buffer)
    									//unsigned int r_buf;

    									l_buf = Buff2[i] | Buff2[i+1] << 8;
    									r_buf = Buff2[i+2] | Buff2[i+3] << 8;

    									//2 Bytes to left output buffer
    									int fifospace = alt_up_audio_write_fifo_space (audio_dev, ALT_UP_AUDIO_RIGHT);
    									if ( fifospace > 0 ) // check if data is available CHECK THAT WE HAVENT READ ALL BYTES
    									{
    										alt_up_audio_write_fifo (audio_dev, &r_buf, 1, ALT_UP_AUDIO_RIGHT);
    										alt_up_audio_write_fifo (audio_dev, &l_buf, 1, ALT_UP_AUDIO_LEFT);
    									} else {
    										i -= 4;
    									}
    								}
    							}


//
//
//
//
//    							if(speed == 0) { //normal speed,  stereo
//    								for (i=0; i<cnt; i+=4) {
//    									//unsigned int l_buf; //512 (hakf if 1024 of other buffer)
//    									//unsigned int r_buf;
//
//    									l_buf = Buff2[i] | Buff2[i+1] << 8;
//    									r_buf = Buff2[i+2] | Buff2[i+3] << 8;
//
//    									//2 Bytes to left output buffer
//    									int fifospace = alt_up_audio_write_fifo_space (audio_dev, ALT_UP_AUDIO_RIGHT);
//    									if ( fifospace > 0 ) // check if data is available CHECK THAT WE HAVENT READ ALL BYTES
//    									{
//    										alt_up_audio_write_fifo (audio_dev, &r_buf, 1, ALT_UP_AUDIO_RIGHT);
//    										alt_up_audio_write_fifo (audio_dev, &l_buf, 1, ALT_UP_AUDIO_LEFT);
//    									} else {
//    										i -= 4;
//    									}
//    								}
//    							} else if (speed == 1) { //half speed, stereo
//    								for (i=0; i<cnt; i+=4) {
//    									//unsigned int l_buf; //512 (hakf if 1024 of other buffer)
//    									//unsigned int r_buf;
//    									for (j = 0; j <2; j+=1) {
//    										l_buf = Buff2[i] | Buff2[i+1] << 8;
//    										r_buf = Buff2[i+2] | Buff2[i+3] << 8;
//
//    										//2 Bytes to left output buffer
//    										int fifospace = alt_up_audio_write_fifo_space (audio_dev, ALT_UP_AUDIO_RIGHT);
//    										if ( fifospace > 0 ) // check if data is available CHECK THAT WE HAVENT READ ALL BYTES
//    										{
//    											alt_up_audio_write_fifo (audio_dev, &r_buf, 1, ALT_UP_AUDIO_RIGHT);
//    											alt_up_audio_write_fifo (audio_dev, &l_buf, 1, ALT_UP_AUDIO_LEFT);
//    										} else {
//    											j -= 1;
//    										}
//    									}
//    								}
//
//    							} else if (speed == 2) { //double speed, stereo
//    								for (i=0; i<cnt; i+=8) {
//    									l_buf = Buff2[i] | Buff2[i+1] << 8;
//    									r_buf = Buff2[i+2] | Buff2[i+3] << 8;
//
//    									//2 Bytes to left output buffer
//    									int fifospace = alt_up_audio_write_fifo_space (audio_dev, ALT_UP_AUDIO_RIGHT);
//    								    if ( fifospace > 0 ) // check if data is available CHECK THAT WE HAVENT READ ALL BYTES
//    								    {
//    								    	alt_up_audio_write_fifo (audio_dev, &r_buf, 1, ALT_UP_AUDIO_RIGHT);
//    								        alt_up_audio_write_fifo (audio_dev, &l_buf, 1, ALT_UP_AUDIO_LEFT);
//    								     } else {
//    								        i -= 8;
//    								     }
//    								    //i+=4;
//    								}
//
//    							} else if (speed == 3) {
//    								for (i=0; i<cnt; i+=4) {
//    									//l_buf = Buff2[i] | Buff2[i+1] << 8;
//    									r_buf = Buff2[i+2] | Buff2[i+3] << 8;
//
//    									//2 Bytes to left output buffer
//    									int fifospace = alt_up_audio_write_fifo_space (audio_dev, ALT_UP_AUDIO_RIGHT);
//    								    if ( fifospace > 0 ) // check if data is available CHECK THAT WE HAVENT READ ALL BYTES
//    								    {
//    								    	alt_up_audio_write_fifo (audio_dev, &r_buf, 1, ALT_UP_AUDIO_RIGHT);
//    								        alt_up_audio_write_fifo (audio_dev, &r_buf, 1, ALT_UP_AUDIO_LEFT);
//    								     } else {
//    								        i -= 4;
//    								     }
//    								    //i+=4;
//    								}
//
//
//    							} else {
//
//    							}

}


void printswitch() {
	unsigned int speed2 = IORD(SWITCH_PIO_BASE, 0) & 0x3;
	switch(speed2) {
	case 0:
		fprintf(display, "%d. %s\nPBACK-NORM SPD\n", tracknum, filenames[tracknum - 1]);
		break;
	case 1:
		fprintf(display, "%d. %s\nPBACK-HALF SPD\n", tracknum, filenames[tracknum - 1]);
		break;
	case 2:
		fprintf(display, "%d. %s\nPBACK-DBL SPD\n", tracknum, filenames[tracknum - 1]);
		break;
	case 3:
		fprintf(display, "%d. %s\nPBACK-MONO\n", tracknum, filenames[tracknum - 1]);
		break;
	}




//	if(speed2 == 0) { //normal speed,  stereo
//		//fprintf(display, "%c%s", 27, "[2j");
//		fprintf(display, "%d. %s\nPBACK-NORM SPD\n", tracknum, filenames[tracknum - 1]);
//	} else if (speed2 == 1) { //half speed, stereo
//		//fprintf(display, "%c%s", 27, "[2j");
//		fprintf(display, "%d. %s\nPBACK-HALF SPD\n", tracknum, filenames[tracknum - 1]);
//	} else if (speed2 == 2) { //double speed, stereo
//		//fprintf(display, "%c%s", 27, "[2j");
//		fprintf(display, "%d. %s\nPBACK-DBL SPD\n", tracknum, filenames[tracknum - 1]);
//	} else if (speed2 == 3) {
//		//fprintf(display, "%c%s", 27, "[2j");
//		fprintf(display, "%d. %s\nPBACK-MONO\n", tracknum, filenames[tracknum - 1]);
//	}
}


void pushButton() {
	if (prev_down == 1 && prev_up == 1) {
		//printf("prev button\n");
		if (tracknum == 1) {
			tracknum = i+1;
		}
		--tracknum;
		fprintf(display, "%c%s", 27, "[2j");
		if (stopped) {
			strcpy(mode, "STOPPED");
			fprintf(display, "%c%s", 27, "[2j");
			fprintf(display, "%d. %s\n%s\n", tracknum, filenames[tracknum - 1], mode);
		} else {
			fprintf(display, "%c%s", 27, "[2j");
			printswitch();
		}
		prev_down = 0;
		prev_up = 0;
		put_rc(f_open(&File1, filenames[tracknum-1], (uint8_t) 1));
		p1 = filesize[tracknum -1];

	} else if (stop_down == 1 && stop_up == 1) {
		//printf("stop button\n");
		stop_down = 0;
		stop_up = 0;
		if (strcmp(mode, "STOPPED")) {
			strcpy(mode, "STOPPED");
			fprintf(display, "%c%s", 27, "[2j");
			fprintf(display, "%d. %s\n%s\n", tracknum, filenames[tracknum - 1], mode);
			//close file
			f_close(&File1);
		}
	} else if (pause_play_down == 1 && pause_play_up == 1) {
		//printf("pause play button\n");
		pause_play_down = 0;
		pause_play_up = 0;
		if (!strcmp(mode, "PLAYING")) { //CHANGE TO SAY THE MODE (just do if not paused for next)
			strcpy(mode, "PAUSED");
			stopped = 1;
			fprintf(display, "%c%s", 27, "[2j");
			fprintf(display, "%d. %s\n%s\n", tracknum, filenames[tracknum - 1], mode);
		} else {
			if (!strcmp(mode, "STOPPED")) {
				put_rc(f_open(&File1, filenames[tracknum-1], (uint8_t) 1));
				p1 = filesize[tracknum -1];
			}
			strcpy(mode, "PLAYING");
			fprintf(display, "%c%s", 27, "[2j");
			printswitch();
			stopped = 0;
		}
	} else if (next_down == 1 && next_up == 1) {
		if (stopped) {
			strcpy(mode, "STOPPED");
		}
		//printf("next button\n");
		if (tracknum == i) {
			tracknum = 0;
		}
		++tracknum;
		fprintf(display, "%c%s", 27, "[2j");
				if (stopped) {
					strcpy(mode, "STOPPED");
					fprintf(display, "%c%s", 27, "[2j");
					fprintf(display, "%d. %s\n%s\n", tracknum, filenames[tracknum - 1], mode);
				} else {
					fprintf(display, "%c%s", 27, "[2j");
					printswitch();
				}
		next_down = 0;
		next_up = 0;
		put_rc(f_open(&File1, filenames[tracknum-1], (uint8_t) 1));
		p1 = filesize[tracknum -1];
	}
}

int main()
{
	//Button set up
	  alt_irq_register(BUTTON_PIO_IRQ, (void*)0, BTN_ISR );
	  alt_irq_register(TIMER_0_IRQ, (void*)0, TIMER_ISR );
	  //Masking the interrupt
	  IOWR(BUTTON_PIO_BASE, 2, 0x0F);

//  char *string = "hello.wav";
//  char *string2 = "hellothere.not";

//		  if (isWav(string)) {
//			  printf("i am wav\n");
//		  }
//
//  	  	  if (isWav(string2)) {
//			  printf("THIS IS WRONG \n");
//		  }
//
//  	  	if (!isWav(string2)) {
//  	  		printf("string2 is not a wav \n");
//  	  			  }


	//while (*ptr == ' ') {
	                //ptr++;
	 //xprintf("rc=%d\n", (uint16_t) disk_initialize((uint8_t) p1));
	put_rc(f_mount((uint8_t) p1, &Fatfs[p1]));
	int j = 0;
	                res = f_opendir(&Dir, ptr);
	                if (res) // if res in non-zero there is an error; print the error.
	                {
	                    put_rc(res);
	                    return 0;
	                }
	                p1 = s1 = s2 = 0; // otherwise initialize the pointers and proceed.
	                while (1)
	                {
	                    res = f_readdir(&Dir, &Finfo);
	                    if ((res != FR_OK) || !Finfo.fname[0])
	                        break;
	                    if (Finfo.fattrib & AM_DIR)
	                    {
	                        s2++;
	                    }
	                    else
	                    {
	                        s1++;
	                        p1 += Finfo.fsize;
	                    }
	                    if (isWav(&(Finfo.fname[0]))) {
	                    	strcpy(filenames[i], &(Finfo.fname[0]));
	                    	filesize[i] = Finfo.fsize;
	                    	i++;
	                    }
	                    /*xprintf("%c%c%c%c%c %u/%02u/%02u %02u:%02u %9lu  %s",
	                            (Finfo.fattrib & AM_DIR) ? 'D' : '-',
	                            (Finfo.fattrib & AM_RDO) ? 'R' : '-',
	                            (Finfo.fattrib & AM_HID) ? 'H' : '-',
	                            (Finfo.fattrib & AM_SYS) ? 'S' : '-',
	                            (Finfo.fattrib & AM_ARC) ? 'A' : '-',
	                            (Finfo.fdate >> 9) + 1980, (Finfo.fdate >> 5) & 15, Finfo.fdate & 31,
	                            (Finfo.ftime >> 11), (Finfo.ftime >> 5) & 63, Finfo.fsize, &(Finfo.fname[0]));*/
	#if _USE_LFN
	                    for (p2 = strlen(Finfo.fname); p2 < 14; p2++)
	                        //xputc(' ');
	                    //xprintf("%s\n", Lfname);
	#else
	                    //xputc('\n');
	#endif
	                }
//	                for (j = 0; j < i; j++) {
//	                	xprintf("%s \n", filenames[j]);
//	                	xprintf("%d \n", filesize[j]);
//	                }
	                //xprintf("%d \n", i);
	                //xprintf("%4u File(s),%10lu bytes total\n%4u Dir(s)", s1, p1, s2);
	                res = f_getfree(ptr, (uint32_t *) & p1, &fs);
	                if (res == FR_OK){
	                    //xprintf(", %10lu bytes free\n", p1 * fs->csize * 512);
	                }else{
	                    put_rc(res);
	                }


	//}
	                strcpy(mode, "STOPPED");

	                display= fopen("/dev/lcd_display", "w");
	            	fprintf(display, "%d. %s\n%s\n", tracknum, filenames[tracknum - 1], mode);

	            	put_rc(f_open(&File1, filenames[tracknum-1], (uint8_t) 1));


	                audio_dev = alt_up_audio_open_dev ("/dev/Audio");



	                p1 = filesize[tracknum -1];
	                //printf("%d", filesize[tracknum-1]);
	            	while (1) {
	            		//button function
	            		pushButton();
	            		play();
	            		//MAKE SURE TRACK PLAYS FROM BEGINNING AND WHOLE TRACK PLAYS
	            		//check mode
	            	}

  return 0;
}
