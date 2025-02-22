#ifndef _PATHFINDING_BIGBUG_H_
#define _PATHFINDING_BIGBUG_H_

//==============================================================================
// Includes
//==============================================================================
#include "swadge2024.h"
#include "typedef_bigbug.h"
#include "tilemap_bigbug.h"

//==============================================================================
// Structs
//==============================================================================

//==============================================================================
// Prototypes
//==============================================================================

bool isPerimeterNode(const bb_midgroundTileInfo_t* tile);
void getNeighbors(const bb_midgroundTileInfo_t* tile, list_t* neighbors, bb_tilemap_t* tilemap);
bool contains(const list_t* nodeList, const bb_midgroundTileInfo_t* tile);
bool pathfindToPerimeter(bb_midgroundTileInfo_t* start, bb_tilemap_t* tilemap);

#endif