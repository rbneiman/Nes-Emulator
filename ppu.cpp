
#include "ppu.h"
#include <cstdint>
#include <iostream>
#include <algorithm>
#include "screen.h"

#define ppuInc(arg) ppuTime += 5 * arg

#define CHECK_BIT(var,pos) (((var)>>(pos)) & 1)
#define inRange(num, low, high) (num >= low && num <= high)
//#define getQuadrant(scanline, tileNum) (((scanline%32)/16)*2 + (((tileNum)%4)/2))
#define attrOffset(scanline, currentTile) (((scanline)/32)*8 + (currentTile)/4)
#define COARSE_X vramAddr&0x001F
#define SPRITE_CHECK(scanline, spriteY, longSprites) ((longSprites) ? ((uint32_t) scanline)-((uint32_t)spriteY) < 16 : ((uint32_t)scanline)-((uint32_t)spriteY) < 8)
//#define DRAW_TILE_BOUNDS
//#define DRAW_SPRITE_BOX

sf::Color boundColor(20,20,20);

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



uint8_t getQuadrant(uint8_t nameAddr){
    nameAddr -= 0x2000;
    int yPart = (nameAddr % 0x80) / 0x40;
    int xPart = (nameAddr % 4) / 2;
    return yPart*2 + xPart;
}
//nameTable -> list of tile indices in the pattern table

//patternTable -> array of 16 byte tile shapes, really only 8 per tile since they are ORed together,
//they are ORed to determine what index in palette to use for color

//attributeTable -> array of blocks (4x4 tiles), each block one bite ->
// four 2bit values, one for each 2x2 quadrant of block, represent palettes

void PPU::fetchTile(uint8_t offX, uint16_t offY){
    uint16_t patternAddr;
    switch (tileProgress) {
        case 0:

            shiftQuadrant = getQuadrant(0x2000 | (vramAddr & 0x0FFF));
            nameTableTemp = readPPUMemory8(0x2000 | (vramAddr & 0x0FFF));
            ++tileProgress;
            break;
        case 1:
            ++tileProgress;
            break;
        case 2:
            shiftAttrTableTemp = readPPUMemory8(0x23C0 | (vramAddr & 0x0C00) | ((vramAddr >> 4) & 0x38) | ((vramAddr >> 2) & 0x07));
            shiftAttrTableTemp = (shiftAttrTableTemp >> (shiftQuadrant * 2)) & 0x03;
            ++tileProgress;
            break;
        case 3:
            ++tileProgress;
            break;
        case 4:
            patternAddr = bgPatternTableAddress + nameTableTemp * 16 + ((vramAddr & 0x7000) >> 12);
            shiftPatternTableTemp[0] = readPPUMemory8(patternAddr);
            shiftPatternTableTemp[1] = readPPUMemory8(patternAddr + 8);
            ++tileProgress;
            break;
        case 5 ... 6:
            ++tileProgress;
            break;
        case 7:
            latchAttrTable[0] = (shiftAttrTableTemp & 0x1);
            latchAttrTable[1] = (shiftAttrTableTemp & 0x2) >> 1;
            shiftPatternTable[0] = (shiftPatternTable[0] & 0xFF00) | (shiftPatternTableTemp[0]);
            shiftPatternTable[1] = (shiftPatternTable[1] & 0xFF00) | (shiftPatternTableTemp[1]);
            tileProgress = 0;
            currentTile = (currentTile+1) % 0x20;
            break;
        case 8:
            tileProgress = 0;
            break;
    }
}



void PPU::evalSprite(){
    if(numSpritesNext > 7 || spriteEvalN >= 63 )
        return;

    secondaryOAM[numSpritesNext*4] = OAM[spriteEvalN * 4] + 1;
    if(SPRITE_CHECK((scanline+1), OAM[spriteEvalN * 4] + 1,longSprites)){
        if(spriteEvalN == 0){
            spriteZeroActive = true;
        }
        secondaryOAM[numSpritesNext*4 + 1] = OAM[spriteEvalN * 4 + 1];
        secondaryOAM[numSpritesNext*4 + 2] = OAM[spriteEvalN * 4 + 2];
        secondaryOAM[numSpritesNext*4 + 3] = OAM[spriteEvalN * 4 + 3];
        ++numSpritesNext;
    }
    ++spriteEvalN;
}


void PPU::fetchSprite(){
    if(spriteFetchCurrent >= numSpritesNext)
        return;
    int spriteY;
    uint16_t spriteTileAddr;
    uint8_t offY;
    bool horizFlip;
    sprite_t* spriteFetch;
    switch (spriteProgress) {
        case 0 ... 3:
            ++spriteProgress;
            break;
        case 4:
            spriteFetch = &spritesNext[spriteFetchCurrent];
            spriteFetch->y = secondaryOAM[spriteFetchCurrent * 4];
            spriteTileTemp = secondaryOAM[spriteFetchCurrent * 4 + 1];
            spriteFetch->attribute = secondaryOAM[spriteFetchCurrent * 4 + 2];
            spriteFetch->x = secondaryOAM[spriteFetchCurrent * 4 + 3];
            spriteFetch->active = 0;
            spriteY = spriteFetch->y;
            horizFlip = (secondaryOAM[spriteFetchCurrent * 4 + 2] & 0x40);
            if(spriteZeroActive && spriteFetchCurrent==0){
                spriteFetch->isSprite0 = true;
            }else{
                spriteFetch->isSprite0 = false;
            }
//            uint8_t spriteOffY = sprite.y - scanline;
            if(longSprites){
                offY = (spritesNext[spriteFetchCurrent].attribute & 0x80) ? 16 : 32;
                spriteTileAddr = (spriteTileTemp & 1) * 0x1000 + ((spriteTileTemp & 0xFE)>>1) * 32;
                uint16_t patternAddr = spriteTileAddr + ((scanline) - spriteY);
                if(!horizFlip){
                    //magic bit flipping stuff I took from somewhere
                    spriteFetch->patternTable[0] =
                            ((readPPUMemory8(patternAddr) * 0x80200802ULL) & 0x0884422110ULL) * 0x0101010101ULL >> 32;
                    spriteFetch->patternTable[1] =
                            ((readPPUMemory8(patternAddr + 16) * 0x80200802ULL) & 0x0884422110ULL) * 0x0101010101ULL >> 32;
                }else{
                    spriteFetch->patternTable[0] = readPPUMemory8(patternAddr);
                    spriteFetch->patternTable[1] = readPPUMemory8(patternAddr + 16);
                }
            }else{
                offY = (spriteFetch->attribute & 0x80) ? (7 - ((scanline + 1) - spriteY)) : (((scanline + 1) - spriteY));
                spriteTileAddr = (spritePatternTableAddress) + (spriteTileTemp) * 16;
                uint16_t patternAddr = spriteTileAddr + offY;
                if(!horizFlip){
                    //magic bit flipping stuff I took from somewhere
                    spriteFetch->patternTable[0] =
                            ((readPPUMemory8(patternAddr) * 0x80200802ULL) & 0x0884422110ULL) * 0x0101010101ULL >> 32;
                    spriteFetch->patternTable[1] =
                            ((readPPUMemory8(patternAddr + 8) * 0x80200802ULL) & 0x0884422110ULL) * 0x0101010101ULL >> 32;
                }else{
                    spriteFetch->patternTable[0] = readPPUMemory8(patternAddr);
                    spriteFetch->patternTable[1] = readPPUMemory8(patternAddr + 8);
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

void PPU::incrementHoriz(){ //increment course X, accounting for nameTable wrapping
    if ((vramAddr & 0x001F) == 31){// if coarse X == 31, i.e. nameTable change
        vramAddr &= ~0x001F; //clear coarse X
        vramAddr ^= 0x0400;// switch horizontal nameTable
    }else{
        vramAddr += 1;
    }

}

void PPU::incrementY(){ //increment Y, accounting for nameTable wrapping
    if((vramAddr & 0x7000) != 0x7000){ //no coarse overflow
        vramAddr += 0x1000;
    }else{
        vramAddr &= ~0x7000; //set fine y = 0
        int coarseY = (vramAddr & 0x03E0) >> 5;
        if(coarseY == 29){ //switch vertical nameTable, set 0
            coarseY = 0;
            vramAddr ^= 0x0800;
        }else if(coarseY == 31){ //no nameTable switch, set 0
            coarseY = 0;
        }else{ //increment normally
            coarseY += 1;
        }

        vramAddr = (vramAddr & ~0x03E0) | (coarseY << 5); //course Y goes back in v
    }
}

void PPU::decrementSprites(){
    for(int i=0; i<numSpritesCurrent; i++){
        sprite_t* sprite = &sprites[i];
        sprite->x -=1;
        if(sprite->x == 0){
            sprite->active = 8;
        }
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
                            decrementSprites();
                            ppuInc(1);
                        }
                        numSpritesCurrent = numSpritesNext;
                        numSpritesNext = 0;
                        currentOAM = 0;
                        spriteEvalN = 0;
                        spriteFetchCurrent = 0;
                        tileProgress = 8;
                        spriteZeroActive = false;
                        break;
                    case 1:  //1-64 tile fetches, OAM clear
                        if(showBackground || showSprites){
                            if(scanCycle % 8 == 0){
                                incrementHoriz();
                            }
                            secondaryOAM[++currentOAM] = 0xFF;
                            if(showBackground)
                                fetchTile();
                            drawDot();
                        }
                        shift();
                        decrementSprites();
                        ppuInc(1);
                        break;
                    case 2 ... 64:
                        if(showBackground || showSprites){
                            if(scanCycle % 8 == 0){
                                incrementHoriz();
                            }
                            secondaryOAM[++currentOAM] = 0xFF;
                            if(showBackground)
                                fetchTile();
                            drawDot();
                        }
                        shift();
                        decrementSprites();
                        ppuInc(1);
                        break;
                    case 65 ... 255: //65-255 tile fetches, sprite eval
                        if(showBackground || showSprites){
                            if(scanCycle % 8 == 0)
                                incrementHoriz();
                            evalSprite();
                            if(showBackground)
                                fetchTile();
                            drawDot();
                        }
                        shift();
                        decrementSprites();
                        ppuInc(1);
                        break;
                    case 256: //256 tile fetches, sprite eval, y increment
                        if(showBackground || showSprites){
                            incrementHoriz();
                            incrementY();
                            evalSprite();
                            if(showBackground)
                                fetchTile();
                            drawDot();
                        }
                        shift();
                        decrementSprites();
                        ppuInc(1);
                        break;
                    case 257: //257 fetch next scanline's sprites, copy horizontal position to v
                        if(showBackground || showSprites)
                            vramAddr = (vramAddr & 0x7BE0) | (tVramAddr & 0x041F);
                        if(showSprites)
                            fetchSprite();
                        if(showBackground)
                            fetchTile();
                        shift();
                        decrementSprites();
                        ppuInc(1);
                        break;
                    case 258 ... 319: //258-320 fetch next scanline's sprites
                        if(showSprites)
                            fetchSprite();
                        decrementSprites();
                        ppuInc(1);
                        break;
                    case 320:
                        if(showSprites)
                            fetchSprite();
                        tileProgress = 8;
                        decrementSprites();
                        ppuInc(1);
                        break;
                    case 321: //321-327 first two tiles next scanline
                        if(showBackground)
                            fetchTile();
                        decrementSprites();
                        ppuInc(1);
                        break;
                    case 322 ... 327:
                        if(showBackground)
                            fetchTile();
                        shift();
                        decrementSprites();
                        ppuInc(1);
                        break;
                    case 328 ... 336: //321-336 first two tiles next scanline, increment horizontal v
                        if(showBackground)
                            fetchTile();
                        if((scanCycle % 8 == 0) && (showBackground || showSprites)){
                            incrementHoriz();
                        }
                        shift();
                        decrementSprites();
                        ppuInc(1);
                        break;
                    case 337:
                        if(showBackground)
                            fetchTile();
//                        shift();
                        decrementSprites();
                        ppuInc(1);
                        break;
                    case 338 ... 340: //337-340 useless tile fetches
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
                switch (scanCycle) {
                    case 0:
                        ppuStatus &= 0x9F; //clear sprite 0 hit and sprite overflow flags
                        spriteFetchCurrent = 0;
                        tileProgress = 8;
                        break;
                    case 1:
                        if((scanCycle % 8 == 0) && (showBackground || showSprites)){
                            incrementHoriz();
                            incrementY();
                        }
                        if(showBackground)
                            fetchTile();
                        break;
                    case 2 ... 255:
                        if((scanCycle % 8 == 0) && (showBackground || showSprites)){
                            incrementHoriz();
                            incrementY();
                        }
                        if(showBackground)
                            fetchTile();
                        shift();
                        break;
                    case 256:
                        if(showBackground || showSprites){
                            incrementHoriz();
                            incrementY();
                        }
                        if(showBackground)
                            fetchTile();
                        shift();
                        break;
                    case 257:
                        if(showBackground || showSprites)
                            vramAddr = (vramAddr & 0x7BE0) | (tVramAddr & 0x0400) | (tVramAddr & 0x001F);
                        if(showBackground)
                            fetchTile();
                        shift();
                        break;
                    case 280 ... 304:
                        if(showBackground || showSprites)
                            vramAddr = (vramAddr & 0x041f) | (tVramAddr & 0x7BE0);
                        break;
                    case 321:
                        tileProgress = 8;
                        if((scanCycle % 8 == 0) && (showBackground || showSprites))
                            incrementHoriz();
                        if(showBackground)
                            fetchTile();
                        break;
                    case 322 ... 336:
                        if((scanCycle % 8 == 0) && (showBackground || showSprites))
                            incrementHoriz();
                        if(showBackground)
                            fetchTile();
                        shift();
                        break;
                    case 337:
                        if(showBackground)
                            fetchTile();
                        shift();
                        break;
                }

                scanCycle +=1;

                ppuInc(1);
                break;
            case 262:
                scanline = 0;
                isOddFrame = !isOddFrame;
                sf::sleep(sf::milliseconds(17 - clock.getElapsedTime().asMilliseconds()));
                clock.restart();
//                std::cout << clock.restart().asMilliseconds() << std::endl;
                break;
        }

        if(scanCycle >= 341){
            if(scanline<240){
                for(int i=0; i<numSpritesNext; i++){
                    sprites[i] = spritesNext[i];
                    if(sprites[i].x == 0){
                        sprites[i].active = 8;
                    }
                }
            }
            scanline++;
            scanCycle = 0;
        }

    }

}

void PPU::shift() {
    shiftPatternTable[0] <<= 1;
    shiftPatternTable[1] <<= 1;
    shiftAttrTable[0] = (shiftAttrTable[0] >> 1) | (latchAttrTable[0] << 7);
    shiftAttrTable[1] = (shiftAttrTable[1] >> 1) | (latchAttrTable[1] << 7);
    for(int i=0; i<numSpritesCurrent; i++){
        sprite_t* sprite = &sprites[i];
        if(sprite->active){
            sprite->patternTable[0] >>= 1;
            sprite->patternTable[1] >>= 1;
            --sprite->active;
        }
    }
}


void PPU::drawDot(){
    uint8_t patternDataBG = (((shiftPatternTable[1] << fineXScroll) & 0x8000) >> 14) | (((shiftPatternTable[0] << fineXScroll) & 0x8000) >> 15);
    uint8_t paletteAttrBG = (((shiftAttrTable[1] >> fineXScroll) & 0x01)<<1) | ((shiftAttrTable[0] >> fineXScroll) & 0x01);
    uint8_t paletteIndexBG = (paletteAttrBG * 4) + patternDataBG;

    sf::Color* drawColor;
    if(patternDataBG == 0){
        drawColor = &palette[readPPUMemory8(0x3f00)];
    }else{
        drawColor = &palette[readPPUMemory8(0x3f00 + paletteIndexBG)];
    }
    bool hadSprite = false;
    bool spriteDrawn = false;
    uint8_t spriteActive = 255;
    uint8_t spriteY = 255;
    for(int spriteNum=0; spriteNum<numSpritesCurrent; spriteNum++){
        sprite_t sprite = sprites[spriteNum];
        if(!sprite.active)
            continue;

        bool vertFlip = (sprite.attribute & 0x80);
        bool behindBG = (sprite.attribute & 0x20);

        uint8_t patternDataSprite = ((sprite.patternTable[1] & 0x1) << 1) | (sprite.patternTable[0] & 0x1);
        uint8_t paletteSectionSprite = (sprite.attribute) & 0x03;
        uint8_t paletteIndexSprite = (paletteSectionSprite * 4) + patternDataSprite;
        spriteActive = sprite.active;
        spriteY = sprite.y;
        hadSprite = true;
         if(patternDataBG == 0 && patternDataSprite != 0){
            drawColor = &palette[readPPUMemory8(0x3f10 + paletteIndexSprite)];
            if(sprite.isSprite0)
                ppuStatus |= 0x40; //set sprite 0 hit flag
            spriteDrawn = true;
            break;
        }else if(patternDataSprite != 0 && !behindBG){
            drawColor = &palette[readPPUMemory8(0x3f10 + paletteIndexSprite)];
            if(sprite.isSprite0)
                ppuStatus |= 0x40; //set sprite 0 hit flag
            spriteDrawn = true;
            break;
        }
    }
#ifdef DRAW_TILE_BOUNDS
    if((!spriteDrawn) && (tileProgress == 0 || (scanline%8 == 0))){
        boundColor = *drawColor + sf::Color(20,20,20);
        drawColor = &boundColor;
    }
#endif
#ifdef DRAW_SPRITE_BOX
    if(hadSprite && (!spriteDrawn) && (spriteActive==8 || spriteActive == 1 || scanline == spriteY || scanline == (spriteY + 7))){
        boundColor = sf::Color(200,0,0);
        drawColor = &boundColor;
    }
#endif

    pixelSet(scanCycle - 1, scanline, *drawColor);
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

    tVramAddr = (tVramAddr & 0x73ff) | ((ppuCtrl&0x0003) << 10); //nameTable select

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
    if(settingXScroll){
        tVramAddr = (tVramAddr & 0xFFE0) | ((ppuScroll & 0x00F8) >> 3);
        fineXScroll = (ppuScroll & 0x07);
    }else{
        tVramAddr = (tVramAddr & 0x0C1F) | ((ppuScroll & 0x0007)<<12) | ((ppuScroll & 0x00F8) << 2);
    }
    settingXScroll = !settingXScroll;
    ppuStatus = (ppuStatus & 0xE0) | (ppuScroll & 0x1F);
}

void PPU::setPpuAddr(uint8_t ppuAddr) {
    if(settingXScroll){
        tVramAddr = (tVramAddr & 0x00FF) | ((ppuAddr & 0x3F) << 8);
    }else{
        tVramAddr = (tVramAddr & 0xFF00) | (ppuAddr);
        vramAddr = tVramAddr;
    }
    settingXScroll = !settingXScroll;
    loadBuffer = true;
    ppuStatus = (ppuStatus & 0xE0) | (ppuAddr & 0x1F);
}

uint8_t PPU::getPpuData(){
    if(loadBuffer && inRange(vramAddr, 0, 0x3eff)){
        loadBuffer = false;
        return readPPUMemory8(vramAddr);
    }
    loadBuffer = false;
    uint8_t out = readPPUMemory8(vramAddr);
    vramAddr += vramAddressIncrement;
    return out;
}

void PPU::setPpuData(uint8_t ppuData) {
    writePPUMemory8(vramAddr, ppuData);
    vramAddr += vramAddressIncrement;
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





