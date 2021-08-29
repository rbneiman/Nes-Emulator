
#include "ppu.h"
#include <cstdint>
#include <iostream>
#include <algorithm>
#include "screen.h"

#define ppuInc(arg) ppuTime += 5 * arg

#define CHECK_BIT(var,pos) (((var)>>(pos)) & 1)
#define inRange(num, low, high) (num >= low && num <= high)
#define getQuadrant(scanline, tileNum) (((scanline%32)/16)*2 + (((tileNum)%4)/2))
#define attrOffset(scanline, currentTile) (((scanline)/32)*8 + (currentTile)/4)

#define SPRITE_CHECK(scanline, spriteY, longSprites) (longSprites ? ((scanline-spriteY) < 16) : ((scanline-spriteY) < 8))

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



//nameTable -> list of tile indices in the pattern table

//patternTable -> array of 16 byte tile shapes, really only 8 per tile since they are ORed together,
//they are ORed to determine what index in palette to use for color

//attributeTable -> array of blocks (4x4 tiles), each block one bite ->
// four 2bit values, one for each 2x2 quadrant of block, represent palettes

void PPU::fetchTile() {
    uint16_t v = ((yScroll & 0x3) << 12) | ((ppuCtrl&0x3) << 10) | ((yScroll&0xf8)<<2) | (((xScroll&0xf8) >> 3) + currentTile); //from nesdev wiki
    switch (tileProgress) {
        case 0:
            nameTableTemp = readPPUMemory8(0x2000 | (v & 0x0FFF));
            ++tileProgress;
            break;
        case 1:
            ++tileProgress;
            break;
        case 2:
            attrTableTemp = readPPUMemory8(0x23C0 | (v & 0x0C00) | ((v >> 4) & 0x38) | ((v >> 2) & 0x07));
            ++tileProgress;
            break;
        case 3:
            ++tileProgress;
            break;
        case 4:
            tiles[currentTile].attrTable = attrTableTemp;
            tiles[currentTile].nameTable = nameTableTemp;

            for(int i=0; i<8; i++){
                uint16_t patternAddr = bgPatternTableAddress + nameTableTemp * 16 + i;
                uint16_t patternAddr2 = bgPatternTableAddress + nameTableTemp * 16 + 8 + i;
                tiles[currentTile].patternTable[i] = readPPUMemory8(patternAddr);
                tiles[currentTile].patternTable[i+8] = readPPUMemory8(patternAddr2);
            }

            currentTile = (currentTile+1) % 0x20;
            ++tileProgress;
            break;
        case 5 ... 6:
            ++tileProgress;
            break;
        case 7:
            tileProgress = 0;
            break;
    }
}

void PPU::evalSprite(){
    if(numSpritesNext > 7)
        return;

    switch(spriteEvalProgress){
        case 0:
            secondaryOAM[numSpritesNext/4] = OAM[spriteEvalN * 4];
            if(SPRITE_CHECK(scanline, OAM[spriteEvalN * 4] ,longSprites)){
                secondaryOAM[numSpritesNext/4 + 1] = OAM[spriteEvalN * 4 + 1];
                secondaryOAM[numSpritesNext/4 + 2] = OAM[spriteEvalN * 4 + 2];
                secondaryOAM[numSpritesNext/4 + 3] = OAM[spriteEvalN * 4 + 3];
                ++numSpritesNext;
            }
            ++spriteEvalN;
            break;
        case 1:
            break;
        case 2:
            break;
    }

}
void PPU::fetchSprite(){
    if(spriteFetchCurrent >= numSpritesNext)
        return;

    uint16_t spriteTileAddr;
    switch (spriteProgress) {
        case 0 ... 3:
            ++spriteProgress;
            break;
        case 4:
            spritesNext[spriteFetchCurrent].y = secondaryOAM[spriteFetchCurrent * 4];
            spriteTileTemp = secondaryOAM[spriteFetchCurrent * 4 + 1];
            spritesNext[spriteFetchCurrent].attribute = secondaryOAM[spriteFetchCurrent * 4 + 2];
            spritesNext[spriteFetchCurrent].x = secondaryOAM[spriteFetchCurrent * 4 + 3];


            if(longSprites){
                spriteTileAddr = (spriteTileTemp & 1) * 0x1000 + ((spriteTileTemp & 0xFE)>>1) * 32;
                for(int i=0; i<16; i++){
                    uint16_t patternAddr = spriteTileAddr + i;
                    uint16_t patternAddr2 = spriteTileAddr + 16 + i;

                    spritesNext[spriteFetchCurrent].patternTable[i] = readPPUMemory8(patternAddr);
                    spritesNext[spriteFetchCurrent].patternTable[i+16] = readPPUMemory8(patternAddr2);
                }
            }else{
                spriteTileAddr = (spriteTileTemp & 1) * 0x1000 + ((spriteTileTemp & 0xFE)>>1) * 16;
                for(int i=0; i<8; i++){
                    uint16_t patternAddr = spriteTileAddr + i;
                    uint16_t patternAddr2 = spriteTileAddr + 8 + i;

                    spritesNext[spriteFetchCurrent].patternTable[i] = readPPUMemory8(patternAddr);
                    spritesNext[spriteFetchCurrent].patternTable[i+8] = readPPUMemory8(patternAddr2);
                }
            }


            ++spriteFetchCurrent;
            ++spriteProgress;
            break;
        case 5 ... 6:
            ++spriteProgress;
            break;
        case 7:
            spriteProgress = 0;
            break;
    }
}


void PPU::cycle(uint64_t runTo){
    uint16_t debug_addr;
    //run through scanCycles one at a time
    while(ppuTime < runTo){
        switch(scanline){
            case 0 ... 239: //0-239 visible lines
                switch (scanCycle){
                    case 0: //0 idle
                        if(!isOddFrame && !showBackground){
                            ppuInc(1);
                        }
                        numSpritesCurrent = numSpritesNext;
                        numSpritesNext = 0;
                        currentOAM = 0;

                        break;
                    case 1 ... 64:  //1-64 tile fetches, OAM clear
                        secondaryOAM[++currentOAM] = 0xFF;
                        fetchTile();
                        ppuInc(1);
                        break;
                    case 65 ... 256: //65-4 tile fetches, sprite eval
                        evalSprite();
                        fetchTile();
                        ppuInc(1);
                        break;
//                    case 249 ... 256: //unused tile fetch, sprite eval
//                        evalSprite();
//                        ppuInc(1);
//                        break;
                    case 257 ... 320: //257-320 fetch next scanline's sprites
                        fetchSprite();
                        ppuInc(1);
                        break;
                    case 321 ... 336: //321-336 first two tiles next scanline
//                        fetchTile();
                        ppuInc(1);
                        break;
                    case 337 ... 340: //337-340 useless tile fetches
                        ppuInc(1);
                        break;
                }
                ++scanCycle;
                break;
            case 240: //240 post-render line
                scanCycle++;
                ppuInc(1);
                break;
            case 241 ... 260: //241-260 vBlank lines
                if(scanline == 241 && scanCycle == 1){
                    if(generateNMI)
                        cpu->doNMI();
                    ppuStatus |= 0x80; //set VBlank flag
                }
                scanCycle++;
                ppuInc(1);
                break;
            case 261: //261 pre-render line
                if(scanCycle == 0){
                    ppuStatus &= 0x9F; //clear sprite 0 hit and sprite overflow flags
                    spriteFetchCurrent = 0;
                }
//                else if(inRange(scanCycle, 321, 336)){
//                    fetchTile();
//                }

                scanCycle +=1;
                ppuInc(1);
                break;
            case 262:
                scanline = 0;
                isOddFrame = !isOddFrame;
                break;
        }

        if(scanCycle >= 341){
            if(scanline<240)
                drawScanline();

            scanline++;
            scanCycle = 0;
        }
    }

}

//draw a single scanline to the actual screen
void PPU::drawScanline(){
    if(!this->showBackground) return;

    int offY = scanline%8;
    //draw BG
    for(int tileNum=0; tileNum<0x20; tileNum++){
        tile_t tile = tiles[tileNum];

        int quadrant = getQuadrant(scanline, tileNum);
        uint8_t paletteSection = (tile.attrTable >> (quadrant * 2)) & 0x03;

        uint8_t drawByte1 = tile.patternTable[offY];
        uint8_t drawByte2 = tile.patternTable[offY + 8];
        sf::Color* drawColor;
        uint8_t lower;
        uint8_t upper;
        uint8_t paletteIndex;
        for(int i=0; i<8; i++){
            lower = CHECK_BIT(drawByte1, i);
            upper = CHECK_BIT(drawByte2, i);
            paletteIndex = (upper << 1) | lower;
            if(paletteIndex == 0)
                drawColor = &palette[readPPUMemory8(0x3f00)];
            else{
                drawColor = &palette[readPPUMemory8(0x3f00 + (paletteSection * 4) + paletteIndex)];
            }
            scanlineColors[tileNum*8 + (7-(i + (xScroll & 0x7)))] = 0x3f00 + (paletteSection * 4) + paletteIndex;
            pixelSet(tileNum*8 + (7-(i+ (xScroll & 0x7))), scanline, *drawColor + ((offY == 7 || (7-i) == 7) ? sf::Color(20,20,20) : sf::Color(0,0,0)));
        }
    }

    //draw sprites
    for(int spriteNum=0; spriteNum<numSpritesCurrent; spriteNum++){
        sprite_t sprite = sprites[spriteNum];
        uint8_t spriteOffY = sprite.y - scanline;

        uint8_t paletteSection = (sprite.attribute) & 0x03;
        uint8_t drawByte1 = sprite.patternTable[spriteOffY];
        uint8_t drawByte2 = sprite.patternTable[spriteOffY + 8];
        sf::Color* drawColor;
        uint8_t paletteIndex;
        if(longSprites){
            for(int i=0; (sprite.x + i) < std::min(sprite.x + 8, 256); i++){
                uint8_t lower = CHECK_BIT(drawByte1, i);
                uint8_t upper = CHECK_BIT(drawByte2, i);
                paletteIndex = (upper << 1) | lower;
                if(paletteIndex == 0)
                    drawColor = &palette[readPPUMemory8(0x3f10)];
                else{
                    if(spriteNum == 0 && scanlineColors[sprite.x+i] == 0x3f00)
                        ppuStatus |= 0x40; //set sprite 0 hit flag
                        drawColor = &palette[readPPUMemory8(0x3f10 + (paletteSection * 4) + paletteIndex)];
                }

                pixelSet((sprite.x+i), scanline, *drawColor);
            }
        }else{
            for(int i=0; (sprite.x + i) < std::min(sprite.x + 8, 256); i++){
                uint8_t lower = CHECK_BIT(drawByte1, i);
                uint8_t upper = CHECK_BIT(drawByte2, i);
                paletteIndex = (upper << 1) | lower;
                if(paletteIndex == 0)
                    drawColor = &palette[readPPUMemory8(0x3f10)];
                else{
                    if(spriteNum == 0 && scanlineColors[sprite.x+i] == 0x3f00)
                        ppuStatus |= 0x40; //set sprite 0 hit flag
                    drawColor = &palette[readPPUMemory8(0x3f10 + (paletteSection * 4) + paletteIndex)];
                }

                pixelSet((sprite.x+i), scanline, *drawColor);
            }
        }
    }

}

uint8_t PPU::readPPUMemory8(uint16_t address){
    if(address>0x2EFFu && address<0x3F00u){ address = address % 0xF00u + 0x2000u;} //mirror
    else if(address>=0x3F20u){ address = address % 0x20u + 0x3F00u;}
    switch (address) {
        case 0x3f10:
            return paletteRAM[0];
        case 0x3f14:
            return paletteRAM[4];
        case 0x3f18:
            return paletteRAM[8];
        case 0x3f1c:
            return paletteRAM[0xC];
        default:
            break;
    }
    if(inRange(address, 0x3f00, 0x3f1f)){
        return paletteRAM[address - 0x3f00];
    }else{
        return rom->read8(address);
    }
}

void PPU::writePPUMemory8(uint16_t address, uint8_t arg){
    if(address>0x2EFFu && address<0x3F00u){ address = address % 0xF00u + 0x2000u;} //mirror
    if(address>=0x3F20u){ address = address % 0x20u + 0x3F00u;}
    switch (address) {
        case 0x3f10:
            paletteRAM[0] = arg;
            return;
        case 0x3f14:
            paletteRAM[4] = arg;
            return;
        case 0x3f18:
            paletteRAM[8] = arg;
            return;
        case 0x3f1c:
            paletteRAM[0xC] = arg;
            return;
        default:
            break;
    }
    if(inRange(address, 0x3f00, 0x3f1f)){
        paletteRAM[address - 0x3f00] = arg;
    }else{
        rom->write8(address, arg);
    }

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

uint8_t PPU::getOamData() const{
    return OAM[oamAddr];
}

void PPU::setOamData(uint8_t oamData) {
    PPU::oamData = oamData;
    ppuStatus = (ppuStatus & 0xE0) | (oamData & 0x1F);
    OAM[oamAddr++] = oamData;
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
    settingPPUAddrMSB = false;
    loadBuffer = true;
    ppuStatus = (ppuStatus & 0xE0) | (ppuAddr & 0x1F);
}

uint8_t PPU::getPpuData(){
    if(loadBuffer && inRange(ppuAddr, 0, 0x3eff)){
        loadBuffer = false;
        return readPPUMemory8(ppuAddr);
    }
    loadBuffer = false;
    uint8_t out = readPPUMemory8(ppuAddr);
    ppuAddr += vramAddressIncrement;
    return out;
}

void PPU::setPpuData(uint8_t ppuData) {
    PPU::ppuData = ppuData;
    writePPUMemory8(ppuAddr, ppuData);
    ppuAddr += vramAddressIncrement;
    ppuStatus = (ppuStatus & 0xE0) | (ppuData & 0x1F);

}

void PPU::setOamDma(uint8_t addr, uint8_t data) {
    OAM[addr] = data;
    ppuStatus = (ppuStatus & 0xE0) | (data & 0x1F);
}

void PPU::printMemoryDebug(int start, int end){
    printf("\n      ");
    for(int i = 0; i<0x10; i++){
        printf("%02x ", i);
    }
    printf("\n\n%04x  ", start-start%16);

    for(int i=start-start%16; i<=end; i++){
        if(i%0x10==0 && i!=start-start%16){
            printf("\n%04x  ", i);
        }
        printf("%02x ", readPPUMemory8(i));
    }
    printf("\n");
    fflush(stdout);
}





