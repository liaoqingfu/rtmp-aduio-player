#include <stdio.h>

/*
sampleRate = 8000, channel = 2, [0x15,0x90]
sampleRate = 12000, channel = 2, [0x14,0x90]
sampleRate = 16000, channel = 2, [0x14,0x10]
sampleRate = 44100, channel = 2, [0x12,0x10]
sampleRate = 48000, channel = 2, [0x11,0x90]

*/
int main(int argc, char *argv[])
{
    int sampleRate = 44100;
    int channel = 2;
    unsigned char audioSpecificConfig[2];

    if(argc >= 2)
    {
        sampleRate = atoi(argv[1]);
    }

    if(argc >= 3)
    {
        channel = atoi(argv[2]);
    }

    audioSpecificConfig[0] = (2 << 3);//AAC Profile：1-main；2-LC；3-SSR
    audioSpecificConfig[1] = 0;
    switch(sampleRate)
    {
        case 48000:
            audioSpecificConfig[0] |= (3 >> 1);
            audioSpecificConfig[1] |= ((3 & 1) << 7);
            break;
		case 44100:
            audioSpecificConfig[0] |= (4 >> 1);
            audioSpecificConfig[1] |= ((4 & 1) << 7);
            break;
		case 32000:
            audioSpecificConfig[0] |= (5 >> 1);
            audioSpecificConfig[1] |= ((5 & 1) << 7);
            break;
		case 24000:
            audioSpecificConfig[0] |= (6 >> 1);
            audioSpecificConfig[1] |= ((6 & 1) << 7);
            break;
		case 22050:
            audioSpecificConfig[0] |= (7 >> 1);
            audioSpecificConfig[1] |= ((7 & 1) << 7);
            break;
        case 16000:
            audioSpecificConfig[0] |= (0x8 >> 1);
            audioSpecificConfig[1] |= ((0x8 & 1) << 7);
            break;  
        case 12000:
            audioSpecificConfig[0] |= (0x9 >> 1);
            audioSpecificConfig[1] |= ((0x9 & 1) << 7);
            break;  
        case 11025:
            audioSpecificConfig[0] |= (0xa >> 1);
            audioSpecificConfig[1] |= ((0xa & 1) << 7);
            break;    
        case 8000:
            audioSpecificConfig[0] |= (0xb >> 1);
            audioSpecificConfig[1] |= ((0xb & 1) << 7);
            break;    
        default:
            printf("rtmpPush帧采样率[%d]不支持!", sampleRate);
            break;
    }
    
    switch(channel)
    {
        case 1:
        case 2:
            audioSpecificConfig[1] |= (channel << 3);
            break;
        default:
            printf("rtmpPush帧声道数[%d]不支持!", channel);
            break;
    }

    printf("sampleRate = %d, channel = %d, [0x%02x,0x%02x]\n", sampleRate, channel, audioSpecificConfig[0], audioSpecificConfig[1]);
    return 0;
}
