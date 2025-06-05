/**
 * @file MusicManager.c
 * @author Baoqi (zzhongas@connect.ust.hk)
 *
 * @copyright Copyright (c) 2025
 */

#include "MusicManager.h"
#include "FOC.h"

#define MUSIC_LENGTH 16

// int16_t Melody[] = {
//     NOTE_G5, 8, NOTE_AS5, 4, NOTE_C6, 8, NOTE_D6, -8, NOTE_DS6, 16, NOTE_D6, 8, NOTE_C6, 4, NOTE_A5,  8, NOTE_F5, -8, NOTE_G5, 16,
//     NOTE_A5, 8, NOTE_AS5, 4, NOTE_G5, 8, NOTE_G5, -8, NOTE_FS5, 16, NOTE_G5, 8, NOTE_A5, 4, NOTE_FS5, 8, NOTE_D5, 4,  NOTE_G5, 8,
// };

int Melody[] = {
    NOTE_D6, -4, NOTE_E6, -4, NOTE_A5, 4, NOTE_E6, -4, NOTE_FS6, -4, NOTE_A6, 16, NOTE_G6, 16, NOTE_FS6, 8,
    NOTE_D6, -4, NOTE_E6, -4, NOTE_A5, 2, NOTE_A5, 16, NOTE_A5,  16, NOTE_B5, 16, NOTE_D6, 8,  NOTE_D6,  16,
};

float musicFrequency[MUSIC_LENGTH];
uint32_t musicDuration[MUSIC_LENGTH];
uint8_t delayBetweenNotes = 0;

uint32_t musicTIMCounter = 0;
uint32_t musicPluseCounter = 0;
uint16_t musicNoteCounter = 0;

extern int32_t openLoopTheta;

void MusicManagerInit()
{
    uint16_t WHOLE_NOTE_DURATION = 1800; // 控制播放速度
    for(int i = 0; i < MUSIC_LENGTH; i++)
    {
        musicFrequency[i] = Melody[i * 2];
        if(Melody[i * 2 + 1] > 0)
            musicDuration[i] = WHOLE_NOTE_DURATION / Melody[i * 2 + 1];
        else
            musicDuration[i] = WHOLE_NOTE_DURATION * 3 / 2 / - Melody[i * 2 + 1];
    }
}

void MusicManagerUpdate(int16_t* outputAngle, float* outputUqWithFeedForward, float* outputUdWithFeedForward)
{

    musicTIMCounter++;
    if(musicTIMCounter >= (float)(CURRENT_LOOP_FREQ) / musicFrequency[musicNoteCounter])
    {
        musicTIMCounter = 0;
        musicPluseCounter++;

        if(musicNoteCounter%2 == 0)
            openLoopTheta += 65536 / 16;
        else
            openLoopTheta -= 65536 / 16;
        *outputAngle = (int16_t)(openLoopTheta % 65536);
    }
    if(delayBetweenNotes && musicPluseCounter > musicDuration[musicNoteCounter] * (uint32_t)(musicFrequency[musicNoteCounter]) / 1000)
    {
        *outputUqWithFeedForward = 0;
        *outputUdWithFeedForward = 0;
    }
    if(musicPluseCounter > (musicDuration[musicNoteCounter] + delayBetweenNotes) * (uint32_t)(musicFrequency[musicNoteCounter]) / 1000)
    {
        *outputUqWithFeedForward = OPENLOOP_DRAG_VOLTAGE*2;
        *outputUdWithFeedForward = 0;

        musicPluseCounter = 0;
        musicNoteCounter++;
        if (musicNoteCounter >= MUSIC_LENGTH)
        {
            musicNoteCounter = 0;
        }
    }
}