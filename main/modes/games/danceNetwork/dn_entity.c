#include "dn_entity.h"
#include "dn_utility.h"
#include "dn_random.h"
#include "shapes.h"

void dn_setData(dn_entity_t* self, void* data, dn_dataType_t dataType)
{
    if (self->data != NULL)
    {
        heap_caps_free(self->data);
        self->data = NULL;
    }
    self->data     = data;
    self->dataType = dataType;
}

void dn_updateBoard(dn_entity_t* self)
{
    dn_boardData_t* boardData = (dn_boardData_t*)self->data;

    // perform hooke's law on neighboring tiles
    for (int y = 0; y < DN_BOARD_SIZE; y++)
    {
        for (int x = 0; x < DN_BOARD_SIZE; x++)
        {
            // Get the current tile
            dn_tileData_t* tileData = &boardData->tiles[y][x];
            int8_t dampen           = 3;
            if (x == boardData->impactPos.x && y == boardData->impactPos.y)
            {
                // the selected tile approaches a particular offset
                tileData->yVel += (((int16_t)(((TFT_HEIGHT >> 2) << DN_DECIMAL_BITS) - tileData->yOffset)) / 3);
            }
            else
            {
                // all unselected tiles approach neighboring tiles
                if (y > boardData->impactPos.y)
                {
                    tileData->yVel += (((int16_t)(boardData->tiles[y - 1][x].yOffset - tileData->yOffset)) / 1);
                    dampen += y - boardData->impactPos.y;
                }
                if (y < boardData->impactPos.y)
                {
                    tileData->yVel += (((int16_t)(boardData->tiles[y + 1][x].yOffset - tileData->yOffset)) / 1);
                    dampen += boardData->impactPos.y - y;
                }
                if (x > boardData->impactPos.x)
                {
                    tileData->yVel += (((int16_t)(boardData->tiles[y][x - 1].yOffset - tileData->yOffset)) / 1);
                    dampen += x - boardData->impactPos.x;
                }
                if (x < boardData->impactPos.x)
                {
                    tileData->yVel += (((int16_t)(boardData->tiles[y][x + 1].yOffset - tileData->yOffset)) / 1);
                    dampen += boardData->impactPos.x - x;
                }
            }

            tileData->yVel /= dampen;

            // Update position with smaller time step
            uint16_t newYOffset = tileData->yOffset + tileData->yVel * (self->gameData->elapsedUs >> 14);
            // If the the yOffset would wrap around
            if (((tileData->yOffset & 0x8000) && !(newYOffset & 0x8000) && tileData->yVel > 0)
                || (!(tileData->yOffset & 0x8000) && (newYOffset & 0x8000) && tileData->yVel < 0))
            {
                // print a message
                ESP_LOGI("Dance Network", "Tile %d,%d yOffset hit the limit", x, y);
                // Set yVel to 0
                tileData->yVel = 0;
            }
            else
            {
                tileData->yOffset = newYOffset;
            }
        }
    }
}

bool dn_isTileSelectabe(dn_entity_t* board, dn_boardPos_t pos)
{
    dn_boardData_t* bData = (dn_boardData_t*)board->data;
    switch (board->gameData->phase)
    {
        case DN_P1_PICK_MOVE_OR_GAIN_REROLL_PHASE:
        case DN_P1_MOVE_PHASE:
        {
            if (bData->tiles[pos.y][pos.x].unit)
            {
                dn_entity_t* unit = bData->tiles[pos.y][pos.x].unit;
                if (unit == bData->p1Units[0] || unit == bData->p1Units[1] || unit == bData->p1Units[2]
                    || unit == bData->p1Units[3] || unit == bData->p1Units[4])
                {
                    return true;
                }
            }
        }
        default:
        {
            break;
        }
    }
    return false;
}

void dn_drawBoard(dn_entity_t* self)
{
    dn_boardData_t* boardData = (dn_boardData_t*)self->data;
    // Draw the tiles
    for (int y = 0; y < DN_BOARD_SIZE; y++)
    {
        for (int x = 0; x < DN_BOARD_SIZE; x++)
        {
            int drawX = ((self->pos.x - self->gameData->camera.pos.x) >> DN_DECIMAL_BITS)
                        + (x - y) * self->gameData->assets[DN_GROUND_TILE_ASSET].originX;
            int drawY = ((self->pos.y - self->gameData->camera.pos.y) >> DN_DECIMAL_BITS)
                        + (x + y) * self->gameData->assets[DN_GROUND_TILE_ASSET].originY
                        - (boardData->tiles[y][x].yOffset >> DN_DECIMAL_BITS);

            if (dn_isTileSelectabe(self, (dn_boardPos_t){.x = x, .y = y}))
            {
                drawWsgPaletteSimple(&self->gameData->assets[DN_GROUND_TILE_ASSET].frames[0],
                                     drawX - self->gameData->assets[DN_GROUND_TILE_ASSET].originX,
                                     drawY - self->gameData->assets[DN_GROUND_TILE_ASSET].originY,
                                     &self->gameData->entityManager
                                          .palettes[DN_RED_FLOOR_PALETTE
                                                    + (((y * ((self->gameData->generalTimer >> 10) % 10) + x + 2)
                                                        + (self->gameData->generalTimer >> 6))
                                                       % 6)]);
            }
            else
            {
                drawWsgSimple(&self->gameData->assets[DN_GROUND_TILE_ASSET].frames[0],
                              drawX - self->gameData->assets[DN_GROUND_TILE_ASSET].originX,
                              drawY - self->gameData->assets[DN_GROUND_TILE_ASSET].originY);
            }
            if (boardData->tiles[y][x].selector != NULL)
            {
                // Draw the back part of the selector
                dn_drawTileSelectorBackHalf(boardData->tiles[y][x].selector, drawX, drawY);
            }
            if (boardData->tiles[y][x].unit != NULL)
            {
                // Draw the unit on the tile
                dn_entity_t* unit = boardData->tiles[y][x].unit;
                if ((unit->assetIndex == DN_KING_ASSET || unit->assetIndex == DN_PAWN_ASSET)
                    && (unit == boardData->p1Units[0] || unit == boardData->p1Units[1] || unit == boardData->p1Units[2]
                        || unit == boardData->p1Units[3] || unit == boardData->p1Units[4]))
                {
                    drawWsgPaletteSimple(&self->gameData->assets[unit->assetIndex].frames[0],
                                         drawX - self->gameData->assets[unit->assetIndex].originX,
                                         drawY - self->gameData->assets[unit->assetIndex].originY,
                                         &self->gameData->entityManager.palettes[DN_WHITE_CHESS_PALETTE]);
                }
                else
                {
                    drawWsgSimple(&self->gameData->assets[unit->assetIndex].frames[0],
                                  drawX - self->gameData->assets[unit->assetIndex].originX,
                                  drawY - self->gameData->assets[unit->assetIndex].originY);
                }
            }
            if (boardData->tiles[y][x].selector != NULL)
            {
                // Draw the front part of the selector
                dn_drawTileSelectorFrontHalf(boardData->tiles[y][x].selector, drawX, drawY);
            }
        }
    }
    // Uncomment to visualize center of screen.
    // drawCircleFilled(TFT_WIDTH >> 1, TFT_HEIGHT >> 1, 2, c000);
}

void dn_updateCurtain(dn_entity_t* self)
{
    dn_curtainData_t* curtainData = (dn_curtainData_t*)self->data;
    curtainData->separation += (self->gameData->elapsedUs >> 13);

    dn_entity_t* board = (dn_entity_t*)self->gameData->entityManager.board;
    if (curtainData->separation > 100 && !board->updateFunction)
    {
        dn_boardData_t* boardData                                                = (dn_boardData_t*)board->data;
        boardData->tiles[boardData->impactPos.y][boardData->impactPos.x].yOffset = (TFT_HEIGHT >> 2) << DN_DECIMAL_BITS;
        board->updateFunction                                                    = dn_updateBoard;
    }
    if (curtainData->separation > (TFT_WIDTH >> 1))
    {
        self->destroyFlag = true;
    }
}
void dn_drawCurtain(dn_entity_t* self)
{
    dn_curtainData_t* curtainData = (dn_curtainData_t*)self->data;
    // Draw the curtain asset
    for (int x = 0; x < 4; x++)
    {
        for (int y = 0; y < 12; y++)
        {
            drawWsgSimple(&self->gameData->assets[DN_CURTAIN_ASSET].frames[0],
                          ((curtainData->separation > 0) * -curtainData->separation)
                              + x * self->gameData->assets[DN_CURTAIN_ASSET].frames[0].w,
                          y * self->gameData->assets[DN_CURTAIN_ASSET].frames[0].h);
            drawWsgSimple(&self->gameData->assets[DN_CURTAIN_ASSET].frames[0],
                          (TFT_WIDTH >> 1) + ((curtainData->separation > 0) * curtainData->separation)
                              + x * self->gameData->assets[DN_CURTAIN_ASSET].frames[0].w,
                          y * self->gameData->assets[DN_CURTAIN_ASSET].frames[0].h);
        }
    }
    char text[9] = "Player 1";
    // get the text width
    uint16_t tWidth = textWidth(&self->gameData->font_ibm, text);
    int16_t x       = (TFT_WIDTH >> 2) - (tWidth >> 1);
    int16_t y       = 29;
    // Draw the intro text
    if (curtainData->separation > -700 && curtainData->separation < -50)
    {
        drawText(&self->gameData->font_ibm, c001, text, x, y);
        y++;
        x++;
        drawText(&self->gameData->font_ibm, c001, text, x, y);
        y++;
        x--;
        drawText(&self->gameData->font_ibm, c001, text, x, y);
        y--;
        x--;
        drawText(&self->gameData->font_ibm, c001, text, x, y);
        x++;
        drawShinyText(&self->gameData->font_ibm, c245, c355, c555, text, x, y);
        // drawText(&self->gameData->font_ibm, c555, text, (TFT_WIDTH >> 2) - (tWidth >> 1), 30);
        // drawText(&self->gameData->font_ibm, c555, text, (TFT_WIDTH >> 2) - (tWidth >> 1), 29);
        // drawText(&self->gameData->font_ibm, c101, text, (TFT_WIDTH >> 2) - (tWidth >> 1), 31);
        // drawText(&self->gameData->font_ibm, c525, text, (TFT_WIDTH >> 2) - (tWidth >> 1), 30);
        switch (self->gameData->characterSets[0])
        {
            case DN_ALPHA_SET:
                drawWsgSimple(&self->gameData->assets[DN_ALPHA_ORTHO_ASSET].frames[0],
                              (TFT_WIDTH >> 2) - (self->gameData->assets[DN_ALPHA_ORTHO_ASSET].frames[0].w >> 1), 50);
                break;
            case DN_CHESS_SET:
                drawWsgPaletteSimple(&self->gameData->assets[DN_CHESS_ORTHO_ASSET].frames[0],
                                     (TFT_WIDTH >> 2) - (self->gameData->assets[DN_CHESS_ORTHO_ASSET].frames[0].w >> 1),
                                     50, &self->gameData->entityManager.palettes[DN_WHITE_CHESS_PALETTE]);
                break;
            default:
                break;
        }
    }
    if (curtainData->separation > -600 && curtainData->separation < -50)
    {
        strcpy(text, "VS");
        tWidth = textWidth(&self->gameData->font_righteous, text);
        drawText(&self->gameData->font_righteous, c530, text, (TFT_WIDTH >> 1) - (tWidth >> 1), 60);
        drawText(&self->gameData->outline_righteous, c550, text, (TFT_WIDTH >> 1) - (tWidth >> 1), 60);
    }
    if (curtainData->separation > -500 && curtainData->separation < -50)
    {
        strcpy(text, "Player 2");
        tWidth = textWidth(&self->gameData->font_ibm, text);
        x      = (TFT_WIDTH >> 1) + (TFT_WIDTH >> 2) - (tWidth >> 1);
        y      = 29;

        drawText(&self->gameData->font_ibm, c001, text, x, y);
        y++;
        x++;
        drawText(&self->gameData->font_ibm, c001, text, x, y);
        y++;
        x--;
        drawText(&self->gameData->font_ibm, c001, text, x, y);
        y--;
        x--;
        drawText(&self->gameData->font_ibm, c001, text, x, y);
        x++;
        drawShinyText(&self->gameData->font_ibm, c245, c355, c555, text, x, y);
        // drawText(&self->gameData->font_ibm, c555, text, (TFT_WIDTH >> 1) + (TFT_WIDTH >> 2) - (tWidth >> 1), 30);
        // drawText(&self->gameData->font_ibm, c555, text, (TFT_WIDTH >> 1) + (TFT_WIDTH >> 2) - (tWidth >> 1), 29);
        // drawText(&self->gameData->font_ibm, c101, text, (TFT_WIDTH >> 1) + (TFT_WIDTH >> 2) - (tWidth >> 1), 31);
        // drawText(&self->gameData->font_ibm, c525, text, (TFT_WIDTH >> 1) + (TFT_WIDTH >> 2) - (tWidth >> 1), 30);
        switch (self->gameData->characterSets[1])
        {
            case DN_ALPHA_SET:
                drawWsgSimple(&self->gameData->assets[DN_ALPHA_ORTHO_ASSET].frames[0],
                              (TFT_WIDTH >> 1) + (TFT_WIDTH >> 2)
                                  - (self->gameData->assets[DN_ALPHA_ORTHO_ASSET].frames[0].w >> 1),
                              50);
                break;
            case DN_CHESS_SET:
                drawWsgSimple(&self->gameData->assets[DN_CHESS_ORTHO_ASSET].frames[0],
                              (TFT_WIDTH >> 1) + (TFT_WIDTH >> 2)
                                  - (self->gameData->assets[DN_CHESS_ORTHO_ASSET].frames[0].w >> 1),
                              50);
                break;
            default:
                break;
        }
    }
}

void dn_drawAlbums(dn_entity_t* self)
{
    char text[10]   = "Player 1";
    uint16_t tWidth = textWidth(&self->gameData->font_ibm, text);
    drawShinyText(&self->gameData->font_ibm, c245, c355, c555, text,
                  ((self->pos.x - self->gameData->camera.pos.x) >> DN_DECIMAL_BITS) - (tWidth >> 1) - 80,
                  ((self->pos.y - self->gameData->camera.pos.y) >> DN_DECIMAL_BITS));

    // strcpy(text, "Creative");
    // tWidth = textWidth(&self->gameData->font_ibm, text);
    // drawShinyText(&self->gameData->font_ibm, c425, c535, c555, text,
    //     ((self->pos.x - self->gameData->camera.pos.x) >> DN_DECIMAL_BITS) - (tWidth >> 1),
    //     ((self->pos.y - self->gameData->camera.pos.y) >> DN_DECIMAL_BITS) - 6);
    // strcpy(text, "Commons");
    // tWidth = textWidth(&self->gameData->font_ibm, text);
    // drawShinyText(&self->gameData->font_ibm, c425, c535, c555, text,
    //     ((self->pos.x - self->gameData->camera.pos.x) >> DN_DECIMAL_BITS) - (tWidth >> 1),
    //     ((self->pos.y - self->gameData->camera.pos.y) >> DN_DECIMAL_BITS) + 5);

    strcpy(text, "Player 2");
    tWidth = textWidth(&self->gameData->font_ibm, text);
    drawShinyText(&self->gameData->font_ibm, c245, c355, c555, text,
                  ((self->pos.x - self->gameData->camera.pos.x) >> DN_DECIMAL_BITS) - (tWidth >> 1) + 80,
                  ((self->pos.y - self->gameData->camera.pos.y) >> DN_DECIMAL_BITS));
}

vec_t dn_colorToTrackCoords(paletteColor_t color)
{
    switch (color)
    {
        case c255:
        case c155:
        {
            return (vec_t){-1, 2};
        }
        case c300:
        case c200:
        {
            return (vec_t){0, 2};
        }
        case c301:
        case c201:
        {
            return (vec_t){1, 2};
        }
        case c302:
        case c202:
        {
            return (vec_t){-2, 1};
        }
        case c303:
        {
            return (vec_t){-1, 1};
        }
        case c304:
        {
            return (vec_t){0, 1};
        }
        case c305:
        {
            return (vec_t){1, 1};
        }
        case c310:
        {
            return (vec_t){2, 1};
        }
        case c311:
        case c111:
        {
            return (vec_t){-2, 0};
        }
        case c312:
        {
            return (vec_t){-1, 0};
        }
        case c313:
        {
            return (vec_t){1, 0};
        }
        case c314:
        {
            return (vec_t){2, 0};
        }
        case c315:
        {
            return (vec_t){-1, -1};
        }
        case c320:
        {
            return (vec_t){0, -1};
        }
        case c321:
        {
            return (vec_t){1, -1};
        }
        case c322:
        {
            return (vec_t){0, -2};
        }
        default:
        {
            return (vec_t){0, 0};
        }
    }
}

dn_twoColors_t dn_trackCoordsToColor(vec_t trackCoords)
{
    switch (trackCoords.y)
    {
        case 2: // forward 2
        {
            switch (trackCoords.x)
            {
                case -1: // left 1
                {
                    return (dn_twoColors_t){(paletteColor_t){c255}, (paletteColor_t){c155}};
                }
                case 0: // left 0
                {
                    return (dn_twoColors_t){(paletteColor_t){c300}, (paletteColor_t){c200}};
                }
                case 1: // right 1
                {
                    return (dn_twoColors_t){(paletteColor_t){c301}, (paletteColor_t){c201}};
                }
                default:
                {
                    break;
                }
            }
            break;
        }
        case 1: // forward 1
        {
            switch (trackCoords.x)
            {
                case -2: // left 2
                {
                    return (dn_twoColors_t){(paletteColor_t){c302}, (paletteColor_t){c202}};
                }
                case -1: // left 1
                {
                    return (dn_twoColors_t){(paletteColor_t){c303}, (paletteColor_t){c303}};
                }
                case 0: // left 0
                {
                    return (dn_twoColors_t){(paletteColor_t){c304}, (paletteColor_t){c304}};
                }
                case 1: // right 1
                {
                    return (dn_twoColors_t){(paletteColor_t){c305}, (paletteColor_t){c305}};
                }
                case 2: // right 2
                {
                    return (dn_twoColors_t){(paletteColor_t){c310}, (paletteColor_t){c310}};
                }
                default:
                {
                    break;
                }
            }
            break;
        }
        case 0: // forward 0
        {
            switch (trackCoords.x)
            {
                case -2: // left 2
                {
                    return (dn_twoColors_t){(paletteColor_t){c311}, (paletteColor_t){c111}};
                }
                case -1: // left 1
                {
                    return (dn_twoColors_t){(paletteColor_t){c312}, (paletteColor_t){c312}};
                }
                case 1: // right 1
                {
                    return (dn_twoColors_t){(paletteColor_t){c313}, (paletteColor_t){c313}};
                }
                case 2: // right 2
                {
                    return (dn_twoColors_t){(paletteColor_t){c314}, (paletteColor_t){c314}};
                }
                default:
                {
                    break;
                }
            }
            break;
        }
        case -1: // backward 1
        {
            switch (trackCoords.x)
            {
                case -1: // left 1
                {
                    return (dn_twoColors_t){(paletteColor_t){c315}, (paletteColor_t){c315}};
                }
                case 0: // left 0
                {
                    return (dn_twoColors_t){(paletteColor_t){c320}, (paletteColor_t){c320}};
                }
                case 1: // right 1
                {
                    return (dn_twoColors_t){(paletteColor_t){c321}, (paletteColor_t){c321}};
                }
                default:
                {
                    break;
                }
            }
            break;
        }
        case -2: // backward 1
        {
            switch (trackCoords.x)
            {
                case 0: // left 0
                {
                    return (dn_twoColors_t){(paletteColor_t){c322}, (paletteColor_t){c322}};
                }
                default:
                {
                    break;
                }
            }
            break;
        }
        default:
        {
            break;
        }
    }
    // out of range
    return (dn_twoColors_t){(paletteColor_t){c000}, (paletteColor_t){c000}};
}

void dn_addTrackToAlbum(dn_entity_t* album, vec_t trackCoords, dn_track_t track)
{
    dn_albumData_t* aData   = (dn_albumData_t*)album->data;
    dn_twoColors_t colors   = dn_trackCoordsToColor(trackCoords);
    paletteColor_t onColor  = c510;
    paletteColor_t offColor = c200;
    switch (track)
    {
        case DN_BLUE_TRACK:
        {
            onColor  = c105;
            offColor = c103;
            break;
        }
        default:
        {
            break;
        }
    }
    wsgPaletteSet(&aData->screenOnPalette, colors.unlit, offColor);
    wsgPaletteSet(&aData->screenOnPalette, colors.lit, onColor);
}

void dn_updateAlbum(dn_entity_t* self)
{
    dn_albumData_t* aData = (dn_albumData_t*)self->data;
    if (!aData->screenIsOn)
    {
        aData->timer -= self->gameData->elapsedUs;
        if (aData->timer <= 0)
        {
            aData->screenIsOn = true;
            aData->timer      = 0;
        }
    }
}

void dn_drawAlbum(dn_entity_t* self)
{
    dn_albumData_t* aData = (dn_albumData_t*)self->data;
    int32_t x             = ((self->pos.x - self->gameData->camera.pos.x) >> DN_DECIMAL_BITS)
                - self->gameData->assets[DN_ALBUM_ASSET].originX;
    int32_t y = ((self->pos.y - self->gameData->camera.pos.y) >> DN_DECIMAL_BITS)
                - self->gameData->assets[DN_ALBUM_ASSET].originY;
    drawWsgPalette(&self->gameData->assets[DN_ALBUM_ASSET].frames[0], x, y,
                   aData->screenIsOn ? &aData->screenOnPalette : &aData->screenOffPalette, false, false, aData->rot);
    if (aData->cornerLightOn || (aData->cornerLightBlinking && (self->gameData->generalTimer & 0b111111) > 15))
    {
        if (aData->rot == 180)
        {
            x += 5;
            y += 54;
        }
        else
        {
            x += 53;
            y += 4;
        }
        drawWsgSimple(&self->gameData->assets[DN_STATUS_LIGHT_ASSET].frames[0], x, y);
    }
}

void dn_updateCharacterSelect(dn_entity_t* self)
{
    dn_characterSelectData_t* cData = (dn_characterSelectData_t*)self->data;
    if (self->gameData->btnDownState & PB_A)
    {
        // select marker
        self->gameData->characterSets[0] = cData->selectCharacterIdx;
        // save to NVS
        writeNvs32(dnCharacterKey, self->gameData->characterSets[0]);

        dn_setCharacterSetPalette(&self->gameData->entityManager, self->gameData->characterSets[0]);
    }
    if (self->gameData->btnDownState & PB_B)
    {
        // free assets
        dn_freeAsset(&self->gameData->assets[DN_ALPHA_DOWN_ASSET]);
        dn_freeAsset(&self->gameData->assets[DN_ALPHA_UP_ASSET]);
        dn_freeAsset(&self->gameData->assets[DN_BUCKET_HAT_DOWN_ASSET]);
        dn_freeAsset(&self->gameData->assets[DN_BUCKET_HAT_UP_ASSET]);
        dn_freeAsset(&self->gameData->assets[DN_KING_ASSET]);
        dn_freeAsset(&self->gameData->assets[DN_PAWN_ASSET]);
        dn_freeAsset(&self->gameData->assets[DN_GROUND_TILE_ASSET]);
        self->destroyFlag = true;
        dn_ShowUi(UI_MENU);
        return;
    }
    else if (self->gameData->btnState & PB_LEFT)
    {
        if (cData->xSelectScrollOffset == 0)
        {
            // scroll left
            if (cData->selectCharacterIdx == 0)
            {
                cData->selectCharacterIdx = DN_NUM_CHARACTERS - 1;
            }
            else
            {
                cData->selectCharacterIdx--;
            }
            cData->xSelectScrollOffset -= self->gameData->assets[DN_GROUND_TILE_ASSET].frames[0].w * 5;
        }
    }
    else if (self->gameData->btnState & PB_RIGHT)
    {
        if (cData->xSelectScrollOffset == 0)
        {
            // scroll right
            cData->selectCharacterIdx = (cData->selectCharacterIdx + 1) % DN_NUM_CHARACTERS;
            // increment the offset to scroll
            cData->xSelectScrollOffset += self->gameData->assets[DN_GROUND_TILE_ASSET].frames[0].w * 5;
        }
    }

    // Scroll the offset if it's not centered yet
    cData->xSelectScrollTimer += self->gameData->elapsedUs;
    while (cData->xSelectScrollTimer >= 3000)
    {
        cData->xSelectScrollTimer -= 3000;
        if (cData->xSelectScrollOffset > 0)
        {
            cData->xSelectScrollOffset--;
        }
        else if (cData->xSelectScrollOffset < 0)
        {
            cData->xSelectScrollOffset++;
        }
    }
}
void dn_drawCharacterSelect(dn_entity_t* self)
{
    dn_characterSelectData_t* cData = (dn_characterSelectData_t*)self->data;

    // Draw the background, a blank menu
    drawMenuMega(self->gameData->bgMenu, self->gameData->menuRenderer, self->gameData->elapsedUs);

    // Set up variables for drawing
    int16_t yOff = MANIA_TITLE_HEIGHT + 20;
    int16_t xOff
        = ((TFT_WIDTH - self->gameData->assets[DN_GROUND_TILE_ASSET].frames[0].w) >> 1) + cData->xSelectScrollOffset;
    int8_t pIdx = cData->selectCharacterIdx;

    // 'Rewind' characters until they're off screen
    while (xOff > 0)
    {
        xOff -= self->gameData->assets[DN_GROUND_TILE_ASSET].frames[0].w * 5;
        pIdx--;
    }

    // Don't use a negative index!
    if (pIdx < 0)
    {
        pIdx = DN_NUM_CHARACTERS - 1;
    }
    // pIdx = dn_wrap(pIdx, DN_NUM_CHARACTERS - 1);

    // Draw floor tiles
    for (int16_t y = 0; y < 9; y++)
    {
        // Draw tiles until you're off screen
        while (xOff < TFT_WIDTH + ((self->gameData->assets[DN_GROUND_TILE_ASSET].frames[0].w * 5) >> 1))
        {
            for (int16_t x = -2; x < 3; x++)
            {
                int16_t drawX = xOff + x * self->gameData->assets[DN_GROUND_TILE_ASSET].frames[0].w
                                + ((self->gameData->assets[DN_GROUND_TILE_ASSET].frames[0].w >> 1) * (y % 2));
                int16_t drawY = yOff + y * (self->gameData->assets[DN_GROUND_TILE_ASSET].frames[0].h >> 1);
                if (drawX >= -self->gameData->assets[DN_GROUND_TILE_ASSET].frames[0].w && drawX <= TFT_WIDTH)
                {
                    // If this is the active maker, draw swapped pallete
                    if (pIdx == self->gameData->characterSets[0] && cData->selectDiamondShape[y * 5 + x + 2])
                    {
                        drawWsgPaletteSimple(
                            &self->gameData->assets[DN_GROUND_TILE_ASSET].frames[0], drawX, drawY,
                            &self->gameData->entityManager
                                 .palettes[DN_RED_FLOOR_PALETTE
                                           + (((y * ((self->gameData->generalTimer >> 10) % 10) + x + 2)
                                               + (self->gameData->generalTimer >> 6))
                                              % 6)]);
                    }
                    else
                    {
                        drawWsgSimple(&self->gameData->assets[DN_GROUND_TILE_ASSET].frames[0], drawX, drawY);
                    }
                }
            }
            // Increment X offset
            xOff += self->gameData->assets[DN_GROUND_TILE_ASSET].frames[0].w * 5;
            // Increment marker index
            pIdx = (pIdx + 1) % DN_NUM_CHARACTERS;
        }
        // reset values
        xOff = ((TFT_WIDTH - self->gameData->assets[DN_GROUND_TILE_ASSET].frames[0].w) >> 1)
               + cData->xSelectScrollOffset;
        pIdx = cData->selectCharacterIdx;

        // 'Rewind' characters until they're off screen
        while (xOff > 0)
        {
            xOff -= self->gameData->assets[DN_GROUND_TILE_ASSET].frames[0].w * 5;
            pIdx--;
        }
        // Don't use a negative index!
        while (pIdx < 0)
        {
            pIdx += DN_NUM_CHARACTERS;
        }
    }

    xOff += self->gameData->assets[DN_GROUND_TILE_ASSET].originX;
    yOff += self->gameData->assets[DN_GROUND_TILE_ASSET].originY;

    // Draw characters until you're off screen (sort of)
    while (xOff < TFT_WIDTH + ((self->gameData->assets[DN_GROUND_TILE_ASSET].frames[0].w * 5) >> 1))
    {
        dn_assetIdx_t kingDown = 0;
        dn_assetIdx_t kingUp   = 0;
        dn_assetIdx_t pawnDown = 0;
        dn_assetIdx_t pawnUp   = 0;

        switch (pIdx)
        {
            case DN_ALPHA_SET:
            {
                kingDown = DN_ALPHA_DOWN_ASSET;
                kingUp   = DN_ALPHA_UP_ASSET;
                pawnDown = DN_BUCKET_HAT_DOWN_ASSET;
                pawnUp   = DN_BUCKET_HAT_UP_ASSET;
                break;
            }
            case DN_CHESS_SET:
            {
                kingDown = DN_KING_ASSET;
                kingUp   = DN_KING_ASSET;
                pawnDown = DN_PAWN_ASSET;
                pawnUp   = DN_PAWN_ASSET;
                break;
            }
            default:
            {
                break;
            }
        }
        for (int8_t i = 0; i < 5; i++)
        {
            if (i == 2) // king is the middle piece
            {
                drawWsgSimple(&self->gameData->assets[kingDown].frames[0],
                              xOff + (self->gameData->assets[DN_GROUND_TILE_ASSET].frames[0].w >> 1) * i
                                  - self->gameData->assets[kingDown].originX,
                              yOff + (self->gameData->assets[DN_GROUND_TILE_ASSET].frames[0].h >> 1) * i
                                  - self->gameData->assets[kingDown].originY);
                if (kingUp == DN_KING_ASSET)
                {
                    drawWsgPaletteSimple(
                        &self->gameData->assets[kingUp].frames[0],
                        xOff - (self->gameData->assets[DN_GROUND_TILE_ASSET].frames[0].w >> 1) * (4 - i)
                            - self->gameData->assets[kingUp].originX,
                        yOff + (self->gameData->assets[DN_GROUND_TILE_ASSET].frames[0].h >> 1) * (4 + i)
                            - self->gameData->assets[kingUp].originY,
                        &self->gameData->entityManager.palettes[DN_WHITE_CHESS_PALETTE]);
                }
                else
                {
                    drawWsgSimple(&self->gameData->assets[kingUp].frames[0],
                                  xOff - (self->gameData->assets[DN_GROUND_TILE_ASSET].frames[0].w >> 1) * (4 - i)
                                      - self->gameData->assets[kingUp].originX,
                                  yOff + (self->gameData->assets[DN_GROUND_TILE_ASSET].frames[0].h >> 1) * (4 + i)
                                      - self->gameData->assets[kingUp].originY);
                }
            }
            else
            {
                drawWsgSimple(&self->gameData->assets[pawnDown].frames[0],
                              xOff + (self->gameData->assets[DN_GROUND_TILE_ASSET].frames[0].w >> 1) * i
                                  - self->gameData->assets[pawnDown].originX,
                              yOff + (self->gameData->assets[DN_GROUND_TILE_ASSET].frames[0].h >> 1) * i
                                  - self->gameData->assets[pawnDown].originY);
                if (pawnUp == DN_PAWN_ASSET)
                {
                    drawWsgPaletteSimple(
                        &self->gameData->assets[pawnUp].frames[0],
                        xOff - (self->gameData->assets[DN_GROUND_TILE_ASSET].frames[0].w >> 1) * (4 - i)
                            - self->gameData->assets[pawnUp].originX,
                        yOff + (self->gameData->assets[DN_GROUND_TILE_ASSET].frames[0].h >> 1) * (4 + i)
                            - self->gameData->assets[pawnUp].originY,
                        &self->gameData->entityManager.palettes[DN_WHITE_CHESS_PALETTE]);
                }
                else
                {
                    drawWsgSimple(&self->gameData->assets[pawnUp].frames[0],
                                  xOff - (self->gameData->assets[DN_GROUND_TILE_ASSET].frames[0].w >> 1) * (4 - i)
                                      - self->gameData->assets[pawnUp].originX,
                                  yOff + (self->gameData->assets[DN_GROUND_TILE_ASSET].frames[0].h >> 1) * (4 + i)
                                      - self->gameData->assets[pawnUp].originY);
                }
            }
        }

        // Increment X offset
        xOff += self->gameData->assets[DN_GROUND_TILE_ASSET].frames[0].w * 5;
        // Increment marker index
        pIdx = (pIdx + 1) % DN_NUM_CHARACTERS;
    }

    // Draw arrows to indicate this can be scrolled
    // Blink the arrows

    if ((self->gameData->generalTimer % 256) > 128)
    {
        // Draw arrows to indicate this can be scrolled
        drawText(&self->gameData->font_righteous, c000, "<", 3, 41);
        drawText(&self->gameData->font_righteous, c550, "<", 3, 38);

        drawText(&self->gameData->font_righteous, c000, ">", TFT_WIDTH - 20, 41);
        drawText(&self->gameData->font_righteous, c550, ">", TFT_WIDTH - 20, 38);
    }
}

void dn_updateTileSelector(dn_entity_t* self)
{
    dn_tileSelectorData_t* tData = (dn_tileSelectorData_t*)self->data;
    dn_boardData_t* bData        = (dn_boardData_t*)self->gameData->entityManager.board->data;
    // move with WASD
    bData->tiles[tData->pos.y][tData->pos.x].selector = NULL;
    if (self->gameData->btnDownState & PB_LEFT)
    {
        tData->pos.x--;
    }
    if (self->gameData->btnDownState & PB_UP)
    {
        tData->pos.y--;
    }
    if (self->gameData->btnDownState & PB_RIGHT)
    {
        tData->pos.x++;
    }
    if (self->gameData->btnDownState & PB_DOWN)
    {
        tData->pos.y++;
    }

    tData->pos.x = CLAMP(tData->pos.x, 0, 4);
    tData->pos.y = CLAMP(tData->pos.y, 0, 4);

    bData->tiles[tData->pos.y][tData->pos.x].selector = self;

    // lines move up at varying rates
    for (int line = 0; line < NUM_SELECTOR_LINES; line++)
    {
        tData->lineYs[line] += line % 7;
        if (dn_randomInt(0, 600) < tData->lineYs[line])
        {
            tData->lineYs[line] = 0;
        }
    }
}

void dn_drawTileSelectorBackHalf(dn_entity_t* self, int16_t x, int16_t y)
{
    dn_tileSelectorData_t* tData = (dn_tileSelectorData_t*)self->data;
    for (int line = 0; line < NUM_SELECTOR_LINES; line++)
    {
        drawLineFast(x - 23, y - (tData->lineYs[line] >> 3), x, y - 11 - (tData->lineYs[line] >> 3),
                     tData->colors[line % 3]);
        drawLineFast(x, y - 11 - (tData->lineYs[line] >> 3), x + 23, y - (tData->lineYs[line] >> 3),
                     tData->colors[line % 3]);
    }
}

void dn_drawTileSelectorFrontHalf(dn_entity_t* self, int16_t x, int16_t y)
{
    dn_tileSelectorData_t* tData = (dn_tileSelectorData_t*)self->data;
    for (int line = 0; line < NUM_SELECTOR_LINES; line++)
    {
        drawLineFast(x - 23, y - (tData->lineYs[line] >> 3), x, y + 11 - (tData->lineYs[line] >> 3),
                     tData->colors[line % 3]);
        drawLineFast(x, y + 11 - (tData->lineYs[line] >> 3), x + 23, y - (tData->lineYs[line] >> 3),
                     tData->colors[line % 3]);
    }
}

void dn_drawPlayerTurn(dn_entity_t* self)
{
    paletteColor_t col = c055;
    switch (self->gameData->phase)
    {
        case DN_P2_PICK_MOVE_OR_GAIN_REROLL_PHASE:
        case DN_P2_MOVE_PHASE:
        case DN_P2_UPGRADE_PHASE:
        case DN_P2_SWAP_PHASE:
        {
            col = c550;
            break;
        }
        default:
        {
            break;
        }
    }
    drawCircleQuadrants(41, 41, 41, false, false, true, false, col);
    drawCircleQuadrants(TFT_WIDTH - 42, 41, 41, false, false, false, true, col);
    drawCircleQuadrants(41, TFT_HEIGHT - 42, 41, false, true, false, false, col);
    drawCircleQuadrants(TFT_WIDTH - 42, TFT_HEIGHT - 42, 41, true, false, false, false, col);
    drawRect(0, 0, TFT_WIDTH - 0, TFT_HEIGHT - 0, col);

    drawCircleQuadrants(42, 42, 41, false, false, true, false, col);
    drawCircleQuadrants(TFT_WIDTH - 43, 42, 41, false, false, false, true, col);
    drawCircleQuadrants(41, TFT_HEIGHT - 43, 41, false, true, false, false, col);
    drawCircleQuadrants(TFT_WIDTH - 43, TFT_HEIGHT - 43, 41, true, false, false, false, col);
    drawRect(1, 1, TFT_WIDTH - 1, TFT_HEIGHT - 1, col);
}