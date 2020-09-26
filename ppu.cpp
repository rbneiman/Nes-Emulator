
#include "ppu.h"
#include <cstdint>
#include "memory.h"
#include "screen.h"

#define PPUCTRL   getMemory8(0x2000u)
#define PPUMASK   getMemory8(0x2001u)
#define PPUSTATUS getMemory8(0x2002u)
#define OAMADDR   getMemory8(0x2003u)
#define OAMDATA   getMemory8(0x2004u)
#define PPUSCROLL getMemory8(0x2005u)
#define PPUADDR   getMemory8(0x2006u)
#define PPUDATA   getMemory8(0x2007u)
#define OAMDMA    getMemory8(0x4014u)

#define cpuInc(arg) ppuTime += 5 * arg

#define Xscroll (PPUSCROLL>>4)
#define Yscroll (PPUSCROLL&~0xF0)


sf::Color palette[0x40] = {
sf::Color(84 ,84 ,84 ),
sf::Color(0  ,30 ,116),
sf::Color(8  ,16 ,144),
sf::Color(48 ,0  ,136),
sf::Color(68 ,0  ,100),
sf::Color(92 ,0  ,48 ),
sf::Color(84 ,4  ,0  ),
sf::Color(60 ,24 ,0  ),
sf::Color(32 ,42 ,0  ),
sf::Color(8  ,58 ,0  ),
sf::Color(0  ,64 ,0  ),
sf::Color(0  ,60 ,0  ),
sf::Color(0  ,50 ,60 ),
sf::Color(0  , 0 ,0  ),

sf::Color(152,150,152),
sf::Color(8  ,76 ,196),
sf::Color(48 ,50 ,236),
sf::Color(92 ,30 ,228),
sf::Color(136,20 ,76 ),
sf::Color(160,20 ,00 ),
sf::Color(152,34 ,32 ),
sf::Color(120,60 ,0  ),
sf::Color(84 ,90 ,0  ),
sf::Color(40 ,114,0  ),
sf::Color(8  ,124,0  ),
sf::Color(0  ,118,40 ),
sf::Color(0  ,102,120),
sf::Color(0  ,0  ,0  ),

sf::Color(236,238,236),
sf::Color(76 ,154,236),
sf::Color(120,124,236),
sf::Color(176,98 ,236),
sf::Color(228,84 ,236),
sf::Color(236,88 ,180),
sf::Color(236,106,100),
sf::Color(212,136, 32),
sf::Color(160,170,  0),
sf::Color(116,196,  0),
sf::Color(76 ,208, 32),
sf::Color(56 ,204,108),
sf::Color(56 ,180,204),
sf::Color(60 ,60 ,60 ),

sf::Color(236,238,236),
sf::Color(168,204,236),
sf::Color(188,188,236),
sf::Color(212,178,236),
sf::Color(236,174,236),
sf::Color(236,174,212),
sf::Color(236,180,176),
sf::Color(228,196,144),
sf::Color(204,210,120),
sf::Color(180,222,120),
sf::Color(168,226,144),
sf::Color(152,226,180),
sf::Color(160,214,228),
sf::Color(160,162,160)};

/* PPU MEMORY MAP
Address range	Size	Description
$0000-$0FFF	$1000	Pattern table 0 "left"
$1000-$1FFF	$1000	Pattern table 1 "right"
$2000-$23FF	$0400	Nametable 0
$2400-$27FF	$0400	Nametable 1
$2800-$2BFF	$0400	Nametable 2
$2C00-$2FFF	$0400	Nametable 3
$3000-$3EFF	$0F00	Mirrors of $2000-$2EFF
$3F00-$3F1F	$0020	Palette RAM indexes
$3F20-$3FFF	$00E0	Mirrors of $3F00-$3F1F
 */


/* OAM MEMORY (acts like array of 64 sprites)
Address Low Nibble	Description
$00, $04, $08, $0C	Sprite Y coordinate
$01, $05, $09, $0D	Sprite tile #
$02, $06, $0A, $0E	Sprite attribute
$03, $07, $0B, $0F	Sprite X coordinate
*/

typedef struct sprite_{
    uint8_t y;
    uint8_t tile;
    uint8_t atrib;
    uint8_t x;
}sprite;

uint8_t ppuMem[0x4000u];
uint8_t OAM[0x100u];

uint32_t ppuTime;
uint32_t scanline;
uint32_t scanCycle;
uint32_t vBlankTime;

uint16_t vramAddr;
uint16_t tempAddr;

uint8_t fineXScroll;

sprite sprites[8];

void InitPpu(){
    //PALETTE TEST
    for(int i=0; i<0x4; i++){
        for(int j=0; j<0x10; j++){
            pixelSet(j,i,palette[i*0x10+j]);
        }
    }
    //
    scanline = -1;
    scanCycle = 0;
    ppuTime = 0;
    vBlankTime = 0;
}

void fillSprites(int line){
    int num = 0;
    for(int i=0; i<0x100 && num<8; i+=4){
        if( ((line - OAM[i]) < 8) && ((line - OAM[i]) > 0)){
            sprites[num++] = sprite{OAM[i],OAM[i+1],OAM[i+2],OAM[i+3]};
        }
    }
    while(num<8){
        sprites[num++] = sprite{0,0,0,0};
    }
}

void cyclePPU(int runTo){

    if(ppuTime < vBlankTime){
        ppuTime = vBlankTime;
        scanline = -1;
        scanCycle = 0;
    }

    if(runTo < ppuTime){return;}

    if(scanline == -1){
        //TODO
    }

    while(scanline < 240){
        if(scanline%2!=0){ppuTime += 5;}
        scanCycle++;
        while(scanCycle < 257){ //fetch and place pixels
            //TODO

            scanCycle++;
            ppuTime += 5;
            if(runTo < ppuTime){return;}
        }
        while(scanCycle < 321){ //idle
            scanCycle++;
            ppuTime += 5;
            if(runTo < ppuTime){return;}
        }
        while(scanCycle < 341){ //fetch for next
            //TODO
            scanCycle++;
            ppuTime += 5;
            if(runTo < ppuTime){return;}
        }
        scanCycle = 0;
        scanline++;
    }

}

uint8_t readPPUMemory8(uint16_t address){
    if(address>0x2EFFu && address<0x3F00u){ address = address % 0xF00u + 0x2000u;} //mirror
    else if(address>0x3F20u){ address = address % 0x20u + 0x3F00u;}
    return ppuMem[address];
}

void writePPUMemory8(uint16_t address, uint8_t arg){
    if(address>0x2EFFu && address<0x3F00u){ address = address % 0xF00u + 0x2000u;} //mirror
    else if(address>0x3F20u){ address = address % 0x20u + 0x3F00u;}
    ppuMem[address] = arg;
}




