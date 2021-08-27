
#include "ppu.h"
#include <cstdint>
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

#define ppuInc(arg) ppuTime += 5 * arg

#define Xscroll (PPUSCROLL>>4)
#define Yscroll (PPUSCROLL&~0xF0)

#define CHECK_BIT(var,pos) (((var)>>(pos)) & 1)

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

sf::Color(0  , 0 ,0  ),
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

sf::Color(0  , 0 ,0  ),
sf::Color(0  , 0 ,0  ),

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

sf::Color(0  , 0 ,0  ),
sf::Color(0  , 0 ,0  ),

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
sf::Color(160,162,160),
sf::Color(0  , 0 ,0  ),
sf::Color(0  , 0 ,0  )};

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





PPU::PPU(CPU6502* cpu, RomFile* rom):
    cpu(cpu), rom(rom){

    //PALETTE TEST
    for(int i=0; i<0x4; i++){
        for(int j=0; j<0x10; j++){
            pixelSet(j,i,palette[i*0x10+j]);
        }
    }
}

void PPU::fillSprites(int line){
    int num = 0;
    for(int i=0; i<0x100 && num<8; i+=4){
        if( ((line - OAM[i]) < 8) && ((line - OAM[i]) > 0)){
            sprites[num++] = sprite_t{OAM[i],OAM[i+1],OAM[i+2],OAM[i+3]};
        }
    }
    while(num<8){
        sprites[num++] = sprite_t{0,0,0,0};
    }
}

/** Calculate attribute table offset from the given tile index.
 *
 * @param currentTile The number of the tile to calculate.
 * @return The index in the attribute table associated with this tile.
 */
int attrOffset(int currentTile){
//    int rowNum = (currentTile/0x20);
//    int colNum = (currentTile%0x20);
//    int blockRow = rowNum/4;
//    int blockCol = colNum/4;

    return ((currentTile/0x20) * 8 + (currentTile%0x20))/4;
}

//nameTable -> list of tile indices in the pattern table

//patternTable -> array of 16 byte tile shapes, really only 8 per tile since they are ORed together,
//they are ORed to determine what index in palette to use for color

//attributeTable -> array of blocks (4x4 tiles), each block one bite ->
// four 2bit values, one for each 2x2 quadrant of block, represent palettes

void PPU::cycle(int runTo){
    uint16_t debug_addr;
    //run through scanCycles one at a time
    while(ppuTime < runTo){
        if(scanline < 240){ //0-239 visible lines
            if(scanCycle == 0){ //0 idle
                scanCycle++;
                ppuInc(1);
            }else if(scanCycle < 257){ //1-256 tile fetches
                switch (tileProgress) {
                    case 0:
                        nameTableTemp = readPPUMemory8(baseNametableAddress + 0x20 * (scanline/8) + currentTile);
                        tileProgress++;
                        scanCycle+=2;
                        ppuInc(2);
                        break;
                    case 1:
                        attrTableTemp = readPPUMemory8(baseNametableAddress + 0x3c0 + attrOffset(currentTile));
                        tileProgress++;
                        scanCycle+=2;
                        ppuInc(2);
                        break;
                    case 2:
//                        patternAddr = readPPUMemory8(bgPatternTableAddress + nameTableTemp * 16);
//                        patternAddr |= readPPUMemory8(bgPatternTableAddress + nameTableTemp * 16 + 8);
                        tiles[currentTile].attrTable = attrTableTemp;
                        tiles[currentTile].nameTable = nameTableTemp;

                        for(int i=0; i<7; i++ ){
                            uint16_t patternAddr = bgPatternTableAddress + nameTableTemp * 16 + i;
                            uint16_t patternAddr2 = bgPatternTableAddress + nameTableTemp * 16 + 8 + i;
                            tiles[currentTile].patternTable[i] = readPPUMemory8(patternAddr);
                            tiles[currentTile].patternTable[i+8] = readPPUMemory8(patternAddr2);
                        }

                        currentTile = (currentTile+1) % 0x20;
                        tileProgress = 0;
                        scanCycle +=4;
                        ppuInc(4);
                        break;
                }
            }else if(scanCycle < 321){ //257-320 next scanline's sprites
                scanCycle +=1;
                ppuInc(1);
            }else if(scanCycle < 337){ //321-336 first two tiles next scanline
//                switch (tileProgress) {
//                    case 0:
//                        nameTableTemp = readPPUMemory8(baseNametableAddress + 0x20 * scanline/8);
//                        tileProgress++;
//                        scanCycle+=2;
//                        ppuInc(2);
//                        break;
//                        case 1:
//                            attrTableTemp = readPPUMemory8(baseNametableAddress + 0x3c0 + attrOffset(currentTile));
//                            tileProgress++;
//                            scanCycle+=2;
//                            ppuInc(2);
//                            break;
//                        case 2:
//                            //                        patternAddr = readPPUMemory8(bgPatternTableAddress + nameTableTemp * 16);
//                            //                        patternAddr |= readPPUMemory8(bgPatternTableAddress + nameTableTemp * 16 + 8);
//                            tiles[currentTile].attrTable = attrTableTemp;
//                            tiles[currentTile].nameTable = nameTableTemp;
//
//                            for(int i=0; i<7; i++ ){
//                                uint16_t patternAddr = bgPatternTableAddress + nameTableTemp * 16 + i;
//                                uint16_t patternAddr2 = bgPatternTableAddress + nameTableTemp * 16 + 8 + i;
//                                tiles[currentTile].patternTable[i] = readPPUMemory8(patternAddr);
//                                tiles[currentTile].patternTable[i+8] = readPPUMemory8(patternAddr2);
//                            }
//
//                            currentTile = (currentTile+1) % 0x20;
//                            tileProgress = 0;
//                            scanCycle +=4;
//                            ppuInc(4);
//                            break;
//                }
                scanCycle +=2;
                ppuInc(2);
            }else{ //337-340 useless tile fetches
                scanCycle +=1;
                ppuInc(1);
            }
        }else if(scanline == 240){ //240 post-render line
            scanCycle++;
            ppuInc(1);
        }else if(scanline < 261){ //241-260 vBlank lines
            if(scanline == 241 && scanCycle == 1){
                if(generateNMI)
                    cpu->doNMI();
                ppuStatus |= 0x80;
            }
            scanCycle++;
            ppuInc(1);
        }else{ //261 pre-render line
            if(scanCycle == 0){

            }
            scanCycle +=1;
            ppuInc(1);
        }


        if(scanCycle >= 341){
            if(scanline<240)
                drawScanline();

            scanline++;
            scanCycle = 0;
        }
        if(scanline == 262){
            scanline = 0;
            isOddFrame = !isOddFrame;
        }

    }

}

int getQuadrant(uint32_t scanLine, int tileNum){
    return ((scanLine/16)%2)*2 + (tileNum%2);
}

//draw a single scanline to screen
void PPU::drawScanline(){
    if(!this->showBackground) return;

    int offY = scanline%8;
    for(int tileNum=0; tileNum<0x20; tileNum++){
        tile_t tile = tiles[tileNum];

        int quadrant = getQuadrant(scanline, tileNum);


        uint8_t drawByte1 = tiles[tileNum].patternTable[offY];
        uint8_t drawByte2 = tiles[tileNum].patternTable[offY + 8];
        sf::Color drawColor;
        uint8_t lower;
        uint8_t upper;
        uint8_t paletteIndex;
        for(int i=0; i<8; i++){
            lower = CHECK_BIT(drawByte1, i);
            upper = CHECK_BIT(drawByte2, i);
            paletteIndex = (upper << 1) | lower;
//            drawColor = sf::Color(quadrant*50,quadrant*50 + i*5, quadrant*50 + offY*5);
            drawColor = sf::Color(scanline%8 * 20, paletteIndex*70, (i==0 ? 200 : paletteIndex * 70));
            pixelSet(tileNum*8 + (7-i), scanline, drawColor);
        }
    }


}

uint8_t PPU::readPPUMemory8(uint16_t address){
    if(address>0x2EFFu && address<0x3F00u){ address = address % 0xF00u + 0x2000u;} //mirror
    else if(address>0x3F20u){ address = address % 0x20u + 0x3F00u;}

    return rom->read8(address);
}

void PPU::writePPUMemory8(uint16_t address, uint8_t arg){
    if(address>0x2EFFu && address<0x3F00u){ address = address % 0xF00u + 0x2000u;} //mirror
    else if(address>0x3F20u){ address = address % 0x20u + 0x3F00u;}

    rom->write8(address, arg);
}


void PPU::setPpuCtrl(uint8_t ppuCtrl) {
    PPU::ppuCtrl = ppuCtrl;

    generateNMI = (ppuCtrl&0b10000000) >> 7;
    masterSlaveMode = (ppuCtrl&0b1000000) >> 6;
    longSprites = (ppuCtrl&0b100000) >> 5;
    bgPatternTableAddress = (ppuCtrl&0b10000) >> 4 ? 0x1000 : 0;
    spritePatternTableAddress = (ppuCtrl&0b1000) >> 3 ? 0x1000 : 0;
    vramAddressIncrement = (ppuCtrl&0b100) >> 2 ? 32 : 1;
    int nameTableBits = (ppuCtrl&0b11);
    baseNametableAddress = 0x2000 + nameTableBits * 0x400;


    ppuStatus = (ppuStatus & 0xE0) | (ppuCtrl & 0x1F);
}


void PPU::setPpuMask(uint8_t ppuMask) {
    PPU::ppuMask = ppuMask;

    emphasizeBlue = (ppuMask&0b1000000) >> 7;
    emphasizeGreen = (ppuMask&0b1000000) >> 6;
    emphasizeRed = (ppuMask&0b100000) >> 5;
    showSprites = (ppuMask&0b10000) >> 4;
    showBackground = (ppuMask&0b1000) >> 3;
    showSpritesLeft = (ppuMask&0b100) >> 2;
    showBackgroundLeft = (ppuMask&0b10) >> 1;
    grayscale = (ppuMask&0b1);

    ppuStatus = (ppuStatus & 0xE0) | (ppuMask & 0x1F);
}

uint8_t PPU::getPpuStatus(){
    uint8_t out = ppuStatus;

    ppuStatus &= 0x7F;
    settingXScroll = true;
    settingPPUAddrMSB = true;

    return out;
}

void PPU::setOamAddr(uint8_t oamAddr) {
    PPU::oamAddr = oamAddr;
    ppuStatus = (ppuStatus & 0xE0) | (oamAddr & 0x1F);
}

uint8_t PPU::getOamData(){
    return oamData;
}

void PPU::setOamData(uint8_t oamData) {
    PPU::oamData = oamData;
    ppuStatus = (ppuStatus & 0xE0) | (oamData & 0x1F);
}

void PPU::setPpuScroll(uint8_t ppuScroll) {
    settingXScroll ? xScroll = ppuScroll : yScroll = ppuScroll;
    settingXScroll = !settingXScroll;
    ppuStatus = (ppuStatus & 0xE0) | (ppuScroll & 0x1F);
}


void PPU::setPpuAddr(uint8_t ppuAddr) {
    this->ppuAddr = settingPPUAddrMSB ?
            (this->ppuAddr&0x00FF) | (((uint16_t) ppuAddr) << 8):
            (this->ppuAddr&0xFF00) | ((uint16_t) ppuAddr);
    settingPPUAddrMSB = !settingPPUAddrMSB;
    ppuStatus = (ppuStatus & 0xE0) | (ppuAddr & 0x1F);
}

uint8_t PPU::getPpuData(){
    return ppuData;
}

void PPU::setPpuData(uint8_t ppuData) {
    PPU::ppuData = ppuData;
    writePPUMemory8(ppuAddr, ppuData);
    ppuAddr += vramAddressIncrement;
    ppuStatus = (ppuStatus & 0xE0) | (ppuData & 0x1F);

}

void PPU::setOamDma(uint8_t oamDma) {
    oamDMA = oamDma;
    ppuStatus = (ppuStatus & 0xE0) | (oamDma & 0x1F);
}





