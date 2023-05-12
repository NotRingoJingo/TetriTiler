#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
//#define OLC_PGEX_SOUND
//#include "olcPGEX_Sound.h"
#define OLC_PGE_GAMEPAD
#include "olcPGEX_Gamepad.h"
//# define AUDIO_LISTENER_IMPLEMENTATION
//# include "olcPGEX_AudioListener.h"
//# define AUDIO_SOURCE_IMPLEMENTATION
//# include "olcPGEX_AudioSource.h"
# include <string>
# include <sstream>
# include <random>
# include <iostream>
# include <filesystem>
# include <png.h>
namespace fs = std::filesystem;
std::unique_ptr<olc::Platform> thisPlatform(olc::platform.get());

#define M_PI 3.14159265358979323846
class Engine : public olc::PixelGameEngine
{
public:
	
	Engine()
	{
		sAppName = "TetriTiler";
	}
	std::mt19937 mtEngine;
	std::vector<std::unique_ptr<olc::Sprite>> spriteSheets;
	std::vector<std::unique_ptr<olc::Decal>> vecS_Decals;
	uint16_t m_cellTileGridSize = 3;
	uint16_t m_resolution = 16;
	const float c_collapseTileTimer = .1f;
	float m_collapseTileTimer = c_collapseTileTimer;
	bool fastCollapse = false;
	const float c_fastCollapseTileTimer = .001f;
	bool m_BeginCollapse = false;
	int fuckupcount = 0;
	struct Rect
	{
		olc::vf2d pos;
		olc::vf2d size;
	};
	struct SpriteSheet
	{
		std::unique_ptr<olc::Sprite> sheet;
		std::unique_ptr<olc::Decal> sheetDecal;
		uint32_t resolution;
		uint16_t tilecount;
		uint16_t tiletype;
	};
	std::vector<SpriteSheet> SpriteSheets_Vec;
	struct Edge
	{
		int16_t cornerA = -1, side = -1 , cornerB = -1;
	};
	struct SpriteTile
	{
		int16_t tiletype;
		uint16_t tileNumber;
		Edge north, east, south, west;
		olc::vf2d sourcpos = { 0,16 };
		uint16_t SheetNumber = 0;
	};
	struct Tile
	{
		bool flippedH = false;
		bool flippedV = false;
		int16_t tiletype = -1;
		uint16_t tileNumber = 0;
		Edge north, east, south, west;
		Rect rect;
		SpriteTile tileToDraw;
		bool collapsed = false;
		std::vector<SpriteTile> tilesAvailable;
	};
	std::vector<std::vector<SpriteTile>> AllBaseTiles;
	void SetEdge(SpriteTile& t, int edgeToSet, int16_t a, int16_t s, int16_t b)
	{
		switch (edgeToSet)
		{
		case 0:
			t.north = { a,s,b };
			break;
		case 1:
			t.east = { a,s,b };
			break;
		case 2:
			t.south = { a,s,b };
			break;
		case 3:
			t.west = { a,s,b };
			break;
		}
	}
	Edge InvertEdge(Edge e)
	{
		return { e.cornerB,e.side,e.cornerA };
	}
	void RemoveEmptyTiles(Tile& tile)
	{
		for (int i = 0; i < tile.tilesAvailable.size(); i++)
		{
			/*if (CompareAllEdges(tile, Edge{ -2,-2,-2 }))
			{
				tile.tilesAvailable.erase(tile.tilesAvailable.begin() + i);
				i--;
			}*/
			if (CompareAllEdges(tile, Edge{ -1,-1,-1 }))
			{
				tile.tilesAvailable.erase(tile.tilesAvailable.begin() + i);
				i--;
			}
		}
	}
	bool CompareAllEdges(Tile t,Edge eToCompare)
	{
		if (EdgeComparison(t.north, eToCompare) && EdgeComparison(t.east, eToCompare) && EdgeComparison(t.south, eToCompare) && EdgeComparison(t.west, eToCompare)) return true;
		return false;
	}
	void CompareTiles(Tile& a, Tile& b,int edgeToCompare)
	{
		for (uint16_t i = 0; i < a.tilesAvailable.size(); i++)
		{
			Edge aEdgeTranslated = a.tilesAvailable[i].north;
			Edge bEdgeTranslated = b.south;
			switch (edgeToCompare)
			{
			case 0:
				bEdgeTranslated = b.south;
				if (b.flippedV) bEdgeTranslated = b.north;
				if (b.flippedH) bEdgeTranslated = InvertEdge(bEdgeTranslated);
				aEdgeTranslated = a.tilesAvailable[i].north;
				if (a.flippedV) aEdgeTranslated = a.tilesAvailable[i].south;
				if (a.flippedH) aEdgeTranslated = InvertEdge(aEdgeTranslated);
				break;
			case 1:
				bEdgeTranslated = b.west;
				if (b.flippedH) bEdgeTranslated = b.east;
				if (b.flippedV) bEdgeTranslated = InvertEdge(bEdgeTranslated);
				aEdgeTranslated = a.tilesAvailable[i].east;
				if (a.flippedH) aEdgeTranslated = a.tilesAvailable[i].west;
				if (a.flippedV) aEdgeTranslated = InvertEdge(aEdgeTranslated);
				break;
			case 2:
				bEdgeTranslated = b.north;
				if (b.flippedV) bEdgeTranslated = b.south;
				if (b.flippedH) bEdgeTranslated = InvertEdge(bEdgeTranslated);
				aEdgeTranslated = a.tilesAvailable[i].south;
				if (a.flippedV) aEdgeTranslated = a.tilesAvailable[i].north;
				if (a.flippedH) aEdgeTranslated = InvertEdge(aEdgeTranslated);
				break;
			case 3:
				bEdgeTranslated = b.east;
				if (b.flippedH) bEdgeTranslated = b.west;
				if (b.flippedV) bEdgeTranslated = InvertEdge(bEdgeTranslated);
				aEdgeTranslated = a.tilesAvailable[i].west;
				if (a.flippedH) aEdgeTranslated = a.tilesAvailable[i].east;
				if (a.flippedV) aEdgeTranslated = InvertEdge(aEdgeTranslated);
				break;
			}
			if (b.tiletype >= 0 && bEdgeTranslated.cornerA == -1 && bEdgeTranslated.cornerB == -1 && bEdgeTranslated.side == -1&&!b.collapsed)
			{
				//bEdgeTranslated = { b.tiletype,b.tiletype,b.tiletype };
				if (!EdgeComparisonBeforeCollapse(aEdgeTranslated, bEdgeTranslated, b.tiletype))
				{
					a.tilesAvailable.erase(a.tilesAvailable.begin() + i);
					//a.tilesAvailable.erase(std::remove(a.tilesAvailable.begin(), a.tilesAvailable.end(), i), a.tilesAvailable.end());
					i--;
				}
			}
			else if (!EdgeComparison(aEdgeTranslated, bEdgeTranslated))
			{
				a.tilesAvailable.erase(a.tilesAvailable.begin() + i);
				//a.tilesAvailable.erase(std::remove(a.tilesAvailable.begin(), a.tilesAvailable.end(), i), a.tilesAvailable.end());
				i--;
			}
		}
		if (a.tilesAvailable.size()<1)
		{
			a.tilesAvailable.push_back(AllBaseTiles[a.tiletype][12]);
			fuckupcount++;
			std::cout << fuckupcount << std::endl;
		}
	}
	bool EdgeComparisonBeforeCollapse(Edge a, Edge b,int16_t typeofTile)
	{
		if (a.cornerA == b.cornerA || a.cornerA == typeofTile)
		{
			if (a.side == b.side || a.side == typeofTile)
			{
				if (a.cornerB == b.cornerB || a.cornerB == typeofTile)
				{
					return true;
				}
			}
		}
		return false;
	}
	bool EdgeComparison(Edge a, Edge b)
	{
		if (a.cornerA == b.cornerA&& a.side == b.side && a.cornerB == b.cornerB) return true;
		return false;
	}
	void InitializeSpritesDecals()
	{
		std::string path = "./assets";
		std::vector<std::filesystem::path> fileNames;
		std::vector<std::string> extensionTypes = { ".png",".jpg",".bmp" };
		for (const auto& fileToLoad : fs::directory_iterator(path))
		{
			fileNames.push_back(fileToLoad.path());
		}
		for (int i = 0; i < fileNames.size(); i++)
		{
			//fileStringNames.push_back(fileNames[i].filename().string());
			bool tokenPass = false;
			for (int j = 0; j < extensionTypes.size(); j++)
			{
				if (fileNames[i].extension().string() == extensionTypes[j])
				{
					tokenPass = true;
					break;
				}
			}
			if (tokenPass)
			{
				SpriteSheet s = { std::make_unique<olc::Sprite>("./assets/" + fileNames[i].filename().string()) };
				SpriteSheets_Vec.push_back(std::move(s));

				//spriteSheets.push_back(std::make_unique<olc::Sprite>("./assets/" + fileNames[i].filename().string()));
				//std::cout << fileNames[i].filename().string() << std::endl;
			}
		}
		for (int i = 0; i < SpriteSheets_Vec.size(); i++)
		{
			SpriteSheets_Vec[i].sheetDecal = std::make_unique<olc::Decal>(SpriteSheets_Vec[i].sheet.get());
			SpriteSheets_Vec[i].tiletype = i;
			SpriteSheets_Vec[i].tilecount = 9;
			SpriteSheets_Vec[i].resolution = m_resolution;
			//vecS_Decals.push_back();
			//std::cout << vecS_Decals[i].get()->sprite->height << std::endl;
		}
	}

	void SetBuiltInEdges()
	{
		int16_t type = 0;
		uint16_t tileNum = 0;
		uint16_t div = 6;
		std::vector<SpriteTile> st;
		for (int i = 0; i < SpriteSheets_Vec.size(); i++)
		{
			//TODO add all cells
			for (int y = 0; y < 5; y++)
			{
				for (int x = 0; x < 11; x++)
				{
					SpriteTile sp;
					sp.tileNumber = tileNum;
					sp.tiletype = type;
					sp.sourcpos = {(float) x * m_resolution,(float)y * m_resolution };
					sp.SheetNumber = i;
					st.push_back(sp);
					tileNum++;
				}
			}
			if (i==div)
			{
				type += 1;
				if (type == 7) type = -1;
				tileNum = 0;
				div += 7;
				AllBaseTiles.push_back(st);
				st.clear();
			}
		}
		int spriteTileLoop = 0;
		for (int i = 0; i < AllBaseTiles.size(); i++)
		{
			
		int edgeType = 0;
			for (int j = 0; j < AllBaseTiles[i].size(); j++)
			{
				int16_t eT = 0;
				switch (edgeType)
				{
				case 0:
					eT = 5;
					break;
				case 1:
					eT = 3;
					break;
				case 2:
					eT = 2;
					break;
				case 3:
					eT = 6;
					break;
				case 4:
					eT = 4;
					break;
				case 5:
					eT = 1;
					break;
				case 6:
					eT = 0;
					break;
				case 7:
					eT = -1;
					break;
				default:
					break;
				}
				//re number the case switch to inculde all cells and update the edges
				//add the sprites to the sprite sheets and include them in this
				switch (spriteTileLoop)
				{
				case 0:
					AllBaseTiles[i][j].north = { eT,eT,eT };
					AllBaseTiles[i][j].east = { eT,AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype };
					AllBaseTiles[i][j].south = { eT,AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype };
					AllBaseTiles[i][j].west = { eT,eT,eT };
					break;
				case 1:
					AllBaseTiles[i][j].north = { eT,eT,eT };
					AllBaseTiles[i][j].east = { eT,AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype };
					AllBaseTiles[i][j].south = { AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype };
					AllBaseTiles[i][j].west = { eT,AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype };
					break;
				case 2:
					AllBaseTiles[i][j].north = { eT,eT,eT };
					AllBaseTiles[i][j].east = { eT,eT,eT };
					AllBaseTiles[i][j].south = { AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype,eT };
					AllBaseTiles[i][j].west = { eT,AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype };
					break;
				case 3:
					AllBaseTiles[i][j].north = { eT,eT,eT };
					AllBaseTiles[i][j].east = { eT,eT,eT };
					AllBaseTiles[i][j].south = { eT,AllBaseTiles[i][j].tiletype,eT };
					AllBaseTiles[i][j].west = { eT,eT,eT };
					break;
				case 4:
					AllBaseTiles[i][j].north = { eT,eT,eT };
					AllBaseTiles[i][j].east = { eT,AllBaseTiles[i][j].tiletype,eT };
					AllBaseTiles[i][j].south = { eT,AllBaseTiles[i][j].tiletype,eT };
					AllBaseTiles[i][j].west = { eT,eT,eT };
					break;
				case 5:
					AllBaseTiles[i][j].north = { eT,eT,eT };
					AllBaseTiles[i][j].east = { eT,AllBaseTiles[i][j].tiletype,eT };
					AllBaseTiles[i][j].south = { AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype,eT };
					AllBaseTiles[i][j].west = { eT,AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype };
					break;
				case 6:
					AllBaseTiles[i][j].north = { eT,eT,eT };
					AllBaseTiles[i][j].east = { eT,AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype };
					AllBaseTiles[i][j].south = { eT,AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype };
					AllBaseTiles[i][j].west = { eT,AllBaseTiles[i][j].tiletype,eT };
					break;
				case 7:
					AllBaseTiles[i][j].north = { eT,eT,eT };
					AllBaseTiles[i][j].east = { eT,eT,eT };
					AllBaseTiles[i][j].south = { eT,AllBaseTiles[i][j].tiletype,eT };
					AllBaseTiles[i][j].west = { eT,AllBaseTiles[i][j].tiletype,eT };
					break;
				case 8:
					AllBaseTiles[i][j].north = { eT,eT,eT };
					AllBaseTiles[i][j].east = { eT,AllBaseTiles[i][j].tiletype,eT };
					AllBaseTiles[i][j].south = { eT,AllBaseTiles[i][j].tiletype,eT };
					AllBaseTiles[i][j].west = { eT,AllBaseTiles[i][j].tiletype,eT };
					break;
				case 9:
					AllBaseTiles[i][j].north = { AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype,eT };
					AllBaseTiles[i][j].east = { eT,AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype };
					AllBaseTiles[i][j].south = { eT,AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype };
					AllBaseTiles[i][j].west = { AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype,eT };
					break;
				case 10:
					AllBaseTiles[i][j].north = {-2,-2,-2 };
					AllBaseTiles[i][j].east = { -2,-2,-2 };
					AllBaseTiles[i][j].south = { -2,-2,-2 };
					AllBaseTiles[i][j].west = { -2,-2,-2 };
					break;
				case 11:
					AllBaseTiles[i][j].north = { eT,AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype };
					AllBaseTiles[i][j].east = { AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype };
					AllBaseTiles[i][j].south = { eT,AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype };
					AllBaseTiles[i][j].west = { eT,eT,eT };
					break;
				case 12:
					AllBaseTiles[i][j].north = { AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype };
					AllBaseTiles[i][j].east = { AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype };
					AllBaseTiles[i][j].south = { AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype };
					AllBaseTiles[i][j].west = { AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype };
					break;
				case 13:
					AllBaseTiles[i][j].north = { AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype,eT };
					AllBaseTiles[i][j].east = { eT,eT,eT };
					AllBaseTiles[i][j].south = { AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype,eT };
					AllBaseTiles[i][j].west = { AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype };
					break;
				case 14:
					AllBaseTiles[i][j].north = { eT,AllBaseTiles[i][j].tiletype,eT };
					AllBaseTiles[i][j].east = { eT,eT,eT };
					AllBaseTiles[i][j].south = { eT,AllBaseTiles[i][j].tiletype,eT };
					AllBaseTiles[i][j].west = { eT,eT,eT };
					break;
				case 15:
					AllBaseTiles[i][j].north = { eT,AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype };
					AllBaseTiles[i][j].east = { AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype,eT };
					AllBaseTiles[i][j].south = { eT,AllBaseTiles[i][j].tiletype,eT };
					AllBaseTiles[i][j].west = { eT,eT,eT };
					break;
				case 16:
					AllBaseTiles[i][j].north = { AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype };
					AllBaseTiles[i][j].east = { AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype,eT };
					AllBaseTiles[i][j].south = { AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype,eT };
					AllBaseTiles[i][j].west = { AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype };
					break;
				case 17:
					AllBaseTiles[i][j].north = { AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype };
					AllBaseTiles[i][j].east = { AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype };
					AllBaseTiles[i][j].south = { eT,AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype };
					AllBaseTiles[i][j].west = { AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype,eT };
					break;
				case 18:
					AllBaseTiles[i][j].north = { AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype,eT };
					AllBaseTiles[i][j].east = { eT,eT,eT };
					AllBaseTiles[i][j].south = { eT,AllBaseTiles[i][j].tiletype,eT };
					AllBaseTiles[i][j].west = { AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype,eT };
					break;
				case 19:
					AllBaseTiles[i][j].north = { AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype };
					AllBaseTiles[i][j].east = { AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype,eT };
					AllBaseTiles[i][j].south = { eT,AllBaseTiles[i][j].tiletype,eT };
					AllBaseTiles[i][j].west = { AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype,eT };
					break;
				case 20:
					AllBaseTiles[i][j].north = { eT,AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype };
					AllBaseTiles[i][j].east = {AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype,eT };
					AllBaseTiles[i][j].south = {AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype,eT};
					AllBaseTiles[i][j].west = { eT,AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype};
					break;
				case 21:
					AllBaseTiles[i][j].north = { -2,-2,-2 };
					AllBaseTiles[i][j].east = { -2,-2,-2 };
					AllBaseTiles[i][j].south = { -2,-2,-2 };
					AllBaseTiles[i][j].west = { -2,-2,-2 };
					break;
				case 22:
					AllBaseTiles[i][j].north = { eT,AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype };
					AllBaseTiles[i][j].east = { AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype,eT };
					AllBaseTiles[i][j].south = { eT,eT,eT };
					AllBaseTiles[i][j].west = { eT,eT,eT };
					break;
				case 23:
					AllBaseTiles[i][j].north = { AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype };
					AllBaseTiles[i][j].east = { AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype,eT };
					AllBaseTiles[i][j].south = { eT,eT,eT };
					AllBaseTiles[i][j].west = { AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype,eT };
					break;
				case 24:
					AllBaseTiles[i][j].north = { AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype,eT };
					AllBaseTiles[i][j].east = { eT,eT,eT };
					AllBaseTiles[i][j].south = { eT,eT,eT };
					AllBaseTiles[i][j].west = { AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype,eT };
					break;
				case 25:
					AllBaseTiles[i][j].north = { eT,AllBaseTiles[i][j].tiletype,eT };
					AllBaseTiles[i][j].east = { eT,eT,eT };
					AllBaseTiles[i][j].south = { eT,eT,eT };
					AllBaseTiles[i][j].west = { eT,eT,eT };
					break;
				case 26:
					AllBaseTiles[i][j].north = { eT,AllBaseTiles[i][j].tiletype,eT };
					AllBaseTiles[i][j].east = { eT,AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype };
					AllBaseTiles[i][j].south = { eT,AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype };
					AllBaseTiles[i][j].west = { eT,eT,eT };
					break;
				case 27:
					AllBaseTiles[i][j].north = { AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype,eT };
					AllBaseTiles[i][j].east = { eT,AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype };
					AllBaseTiles[i][j].south = { AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype };
					AllBaseTiles[i][j].west = { AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype };
					break;
				case 28:
					AllBaseTiles[i][j].north = { eT,AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype };
					AllBaseTiles[i][j].east = { eT,AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype };
					AllBaseTiles[i][j].south = { AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype };
					AllBaseTiles[i][j].west = { AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype };
					break;
				case 29:
					AllBaseTiles[i][j].north = { eT,AllBaseTiles[i][j].tiletype,eT };
					AllBaseTiles[i][j].east = { eT,eT,eT };
					AllBaseTiles[i][j].south = { AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype,eT };
					AllBaseTiles[i][j].west = { eT,AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype };
					break;
				case 30:
					AllBaseTiles[i][j].north = { eT,AllBaseTiles[i][j].tiletype,eT };
					AllBaseTiles[i][j].east = { eT,AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype };
					AllBaseTiles[i][j].south = { AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype };
					AllBaseTiles[i][j].west = { eT,AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype };
					break;
				case 31:
					AllBaseTiles[i][j].north = { eT,AllBaseTiles[i][j].tiletype,eT };
					AllBaseTiles[i][j].east = { eT,AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype };
					AllBaseTiles[i][j].south = { eT,AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype };
					AllBaseTiles[i][j].west = { eT,AllBaseTiles[i][j].tiletype,eT };
					break;
				case 32:
					AllBaseTiles[i][j].north = { eT,AllBaseTiles[i][j].tiletype,eT };
					AllBaseTiles[i][j].east = { eT,AllBaseTiles[i][j].tiletype,eT };
					AllBaseTiles[i][j].south = { AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype,eT };
					AllBaseTiles[i][j].west = { eT,AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype };
					break;
				case 33:
					AllBaseTiles[i][j].north = { eT,eT,eT };
					AllBaseTiles[i][j].east = { eT,AllBaseTiles[i][j].tiletype,eT };
					AllBaseTiles[i][j].south = { eT,eT,eT };
					AllBaseTiles[i][j].west = { eT,eT,eT };
					break;
				case 34:
					AllBaseTiles[i][j].north = { eT,eT,eT };
					AllBaseTiles[i][j].east = { eT,AllBaseTiles[i][j].tiletype,eT };
					AllBaseTiles[i][j].south = { eT,eT,eT };
					AllBaseTiles[i][j].west = { eT,AllBaseTiles[i][j].tiletype,eT };
					break;
				case 35:
					AllBaseTiles[i][j].north = { eT,eT,eT };
					AllBaseTiles[i][j].east = { eT,eT,eT };
					AllBaseTiles[i][j].south = { eT,eT,eT };
					AllBaseTiles[i][j].west = { eT,AllBaseTiles[i][j].tiletype,eT };
					break;
				case 36:
					AllBaseTiles[i][j].north = { eT,eT,eT };
					AllBaseTiles[i][j].east = { eT,eT,eT };
					AllBaseTiles[i][j].south = { eT,eT,eT };
					AllBaseTiles[i][j].west = { eT,eT,eT };
					break;
				case 37:
					AllBaseTiles[i][j].north = { eT,AllBaseTiles[i][j].tiletype,eT };
					AllBaseTiles[i][j].east = { eT,AllBaseTiles[i][j].tiletype,eT };
					AllBaseTiles[i][j].south = { eT,eT,eT };
					AllBaseTiles[i][j].west = { eT,eT,eT };
					break;
				case 38:
					AllBaseTiles[i][j].north = { AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype,eT };
					AllBaseTiles[i][j].east = { eT,AllBaseTiles[i][j].tiletype,eT };
					AllBaseTiles[i][j].south = { eT,eT,eT };
					AllBaseTiles[i][j].west = { AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype,eT };
					break;
				case 39:
					AllBaseTiles[i][j].north = { eT,AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype };
					AllBaseTiles[i][j].east = {AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype,eT };
					AllBaseTiles[i][j].south = { eT,eT,eT };
					AllBaseTiles[i][j].west = {   eT,AllBaseTiles[i][j].tiletype,eT};
					break;
				case 40:
					AllBaseTiles[i][j].north = { eT,AllBaseTiles[i][j].tiletype,eT };
					AllBaseTiles[i][j].east = { eT,eT,eT };
					AllBaseTiles[i][j].south = { eT,eT,eT };
					AllBaseTiles[i][j].west = { eT,AllBaseTiles[i][j].tiletype,eT };
					break;
				case 41:
					AllBaseTiles[i][j].north = { eT,AllBaseTiles[i][j].tiletype,eT };
					AllBaseTiles[i][j].east = { eT,AllBaseTiles[i][j].tiletype,eT };
					AllBaseTiles[i][j].south = { eT,eT,eT };
					AllBaseTiles[i][j].west = { eT,AllBaseTiles[i][j].tiletype,eT };
					break;
				case 42:
					AllBaseTiles[i][j].north = { eT,AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype };
					AllBaseTiles[i][j].east = { AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype,eT };
					AllBaseTiles[i][j].south = { eT,AllBaseTiles[i][j].tiletype,eT };
					AllBaseTiles[i][j].west = { eT,AllBaseTiles[i][j].tiletype,eT };
					break;
				case 43:
					AllBaseTiles[i][j].north = { AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype,eT };
					AllBaseTiles[i][j].east = { eT,AllBaseTiles[i][j].tiletype,eT };
					AllBaseTiles[i][j].south = { eT,AllBaseTiles[i][j].tiletype,eT };
					AllBaseTiles[i][j].west = { AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype,eT };
					break;
				case 44:
					AllBaseTiles[i][j].north = { -2,-2,-2 };
					AllBaseTiles[i][j].east = { -2,-2,-2 };
					AllBaseTiles[i][j].south = { -2,-2,-2 };
					AllBaseTiles[i][j].west = { -2,-2,-2 };
					break;
				case 45:
					AllBaseTiles[i][j].north = { -2,-2,-2 };
					AllBaseTiles[i][j].east = { -2,-2,-2 };
					AllBaseTiles[i][j].south = { -2,-2,-2 };
					AllBaseTiles[i][j].west = { -2,-2,-2 };
					break;
				case 46:
					AllBaseTiles[i][j].north = { -2,-2,-2 };
					AllBaseTiles[i][j].east = { -2,-2,-2 };
					AllBaseTiles[i][j].south = { -2,-2,-2 };
					AllBaseTiles[i][j].west = { -2,-2,-2 };
					break;
				case 47:
					AllBaseTiles[i][j].north = { -2,-2,-2 };
					AllBaseTiles[i][j].east = { -2,-2,-2 };
					AllBaseTiles[i][j].south = { -2,-2,-2 };
					AllBaseTiles[i][j].west = { -2,-2,-2 };
					break;
				case 48:
					AllBaseTiles[i][j].north = { eT,AllBaseTiles[i][j].tiletype,eT };
					AllBaseTiles[i][j].east = { eT,AllBaseTiles[i][j].tiletype,eT };
					AllBaseTiles[i][j].south = { eT,AllBaseTiles[i][j].tiletype,eT };
					AllBaseTiles[i][j].west = { eT,eT,eT };
					break;
				case 49:
					AllBaseTiles[i][j].north = { AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype,eT };
					AllBaseTiles[i][j].east = { eT,AllBaseTiles[i][j].tiletype,eT };
					AllBaseTiles[i][j].south = { AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype,eT };
					AllBaseTiles[i][j].west = { AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype };
					break;
				case 50:
					AllBaseTiles[i][j].north = { eT,AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype };
					AllBaseTiles[i][j].east = { AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype };
					AllBaseTiles[i][j].south = { eT,AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype };
					AllBaseTiles[i][j].west = { eT,AllBaseTiles[i][j].tiletype,eT };
					break;
				case 51:
					AllBaseTiles[i][j].north = { eT,AllBaseTiles[i][j].tiletype,eT };
					AllBaseTiles[i][j].east = { eT,eT,eT };
					AllBaseTiles[i][j].south = { eT,AllBaseTiles[i][j].tiletype,eT };
					AllBaseTiles[i][j].west = { eT,AllBaseTiles[i][j].tiletype,eT };
					break;
				case 52:
					AllBaseTiles[i][j].north = { eT,AllBaseTiles[i][j].tiletype,eT };
					AllBaseTiles[i][j].east = { eT,AllBaseTiles[i][j].tiletype,eT };
					AllBaseTiles[i][j].south = { eT,AllBaseTiles[i][j].tiletype,eT };
					AllBaseTiles[i][j].west = { eT,AllBaseTiles[i][j].tiletype,eT };
					break;
				case 53:
					AllBaseTiles[i][j].north = { -2,-2,-2 };
					AllBaseTiles[i][j].east = { -2,-2,-2 };
					AllBaseTiles[i][j].south = { -2,-2,-2 };
					AllBaseTiles[i][j].west = { -2,-2,-2 };
					break;
				case 54:
					AllBaseTiles[i][j].north = { -2,-2,-2 };
					AllBaseTiles[i][j].east = { -2,-2,-2 };
					AllBaseTiles[i][j].south = { -2,-2,-2 };
					AllBaseTiles[i][j].west = { -2,-2,-2 };
					break;
				default:
					break;
				}
				//update sprite loop to the new total
				spriteTileLoop++;
				if (spriteTileLoop >= 55)
				{
					spriteTileLoop = 0;
					edgeType++;
				}
			}
		}
	}
	void UpdatePlayfield(float fElapsedTime)
	{
		if (m_BeginCollapse)
		{
			m_collapseTileTimer -= fElapsedTime;
			if (m_collapseTileTimer <= 0)
			{
				if (fastCollapse) m_collapseTileTimer = c_fastCollapseTileTimer;
				else m_collapseTileTimer = c_collapseTileTimer;
				m_BeginCollapse = BeginColapse(playField, fElapsedTime);
			}
		}
		if (GetKey(olc::E).bReleased)
		{
			ExplodePlayfield(playField);
			fastCollapse = true;
		}
	
	}
	HWND pubHwnd = olc::pubHand;
private:
	enum shapes
	{
		I = 0,
		o = 1,
		t = 2,
		j = 3,
		l = 4,
		s = 5,
		z = 6,
	    
	};

	//struct Rect
	//{
	//	olc::vf2d pos;
	//	olc::vf2d size;
	//};
	
	struct tetriminoes
	{
		std::vector<std::string> blockStrings;
		std::vector<std::vector<olc::vf2d>> blockPositions;
		std::vector<std::vector<olc::vf2d>> drawBlockPositions;
		olc::Pixel material;
		olc::vf2d sizePerBlock;
		olc::vf2d firstPos = {-1,-1};
		olc::vf2d lastPos;
		olc::vf2d pallettePos;
		olc::vf2d placedPos;
		bool selected = false;
		bool unplacable = false;
		int tileType;
		//tetriminoes();
		tetriminoes()
		{
			tileType = 8;
		}
		tetriminoes(int type, olc::vf2d size)
		{
			tileType = type;

			blockStrings.clear();
			blockPositions.resize(4);
			drawBlockPositions.resize(4);
			for (auto i = blockPositions.begin(); i < blockPositions.end(); i++)
			{
				i->resize(4);
			}
			for (auto i = drawBlockPositions.begin(); i < drawBlockPositions.end(); i++)
			{
				i->resize(4);
			}
			sizePerBlock = size;
			float x = 0;
			float y = 0;
			switch (type)
			{
			case I:
				blockStrings.push_back("1111");
				blockStrings.push_back("0000");
				blockStrings.push_back("0000");
				blockStrings.push_back("0000");
				break;
			case o:
				blockStrings.push_back("1100");
				blockStrings.push_back("1100");
				blockStrings.push_back("0000");
				blockStrings.push_back("0000");
				break;
			case t:
				blockStrings.push_back("1110");
				blockStrings.push_back("0100");
				blockStrings.push_back("0000");
				blockStrings.push_back("0000");
				break;
			case j:
				blockStrings.push_back("0100");
				blockStrings.push_back("0100");
				blockStrings.push_back("1100");
				blockStrings.push_back("0000");
				break;
			case l:
				blockStrings.push_back("1000");
				blockStrings.push_back("1000");
				blockStrings.push_back("1100");
				blockStrings.push_back("0000");
				break;
			case s:
				blockStrings.push_back("0110");
				blockStrings.push_back("1100");
				blockStrings.push_back("0000");
				blockStrings.push_back("0000");
				break;
			case z:
				blockStrings.push_back("1100");
				blockStrings.push_back("0110");
				blockStrings.push_back("0000");
				blockStrings.push_back("0000");
				break;
			default:
				break;
			}
			for (auto i = blockStrings.begin(); i < blockStrings.end(); i++)
			{
				for (auto c	 = i->begin(); c <i->end(); c++)
				{
					if (*c == '1')
					{
						olc::vf2d newVf2d = olc::vf2d(size.x * x, size.y * y);
						olc::vf2d newDrawVf2d = olc::vf2d(10 * x, 10 * y);
						blockPositions[y][x] = newVf2d;
						drawBlockPositions[y][x] = newDrawVf2d;
					}
					else //give block position an invalid value
					{
						olc::vf2d negativePos = olc::vf2d{ -1.0,-1.0 };
						olc::vf2d negativeDrawPos = olc::vf2d{ -1.0,-1.0 };
						blockPositions[y][x] = negativePos;
						drawBlockPositions[y][x] = negativeDrawPos;
					}
					x++;
				}
				x = 0;
				y++;
			}
			
			for (int i = 0; i < blockPositions.size(); i++)
			{
				//std::remove_if(blockPositions[i].begin(), blockPositions[i].end(), toBeRemoved());
				blockPositions[i].erase(std::remove_if(blockPositions[i].begin(), blockPositions[i].end(), [](const olc::vf2d& v) {
					return v.x < 0 || v.y < 0;
					}), blockPositions[i].end());
				drawBlockPositions[i].erase(std::remove_if(drawBlockPositions[i].begin(), drawBlockPositions[i].end(), [](const olc::vf2d& v) {
					return v.x < 0 || v.y < 0;
					}), drawBlockPositions[i].end());
			}
			for (auto i = blockPositions.begin(); i < blockPositions.end(); i++)
			{
				for (auto b = i->begin(); b < i->end(); b++)
				{
					if (firstPos == olc::vf2d(-1,-1))
					{
						firstPos = *b;
						
					}
					lastPos = *b;
				}
			}
			bool breakthis = true;
	    }
		

	};
	struct Pallete
	{
		Rect palleteRect;
		std::vector<tetriminoes>pallettetetrisTiles;
	};
	struct emptyCells
	{
		Rect cell;
		bool empty = false;
		std::vector<std::vector<Tile>> tilesInCell; //nullptr;
		bool notempty = false;
	};
	struct PlayField
	{
		std::vector<std::vector<emptyCells>> playfieldCells;
		Rect playFieldRect;
		std::vector<tetriminoes>tetrisTiles;
		PlayField()
		{
		}
		PlayField(uint16_t resolution, uint16_t cellTileGridSize)
		{
			int x = 0;
			int y = 0;
			playfieldCells.resize(15);
			for (int i = 0; i < playfieldCells.size(); i++)
			{
				playfieldCells[i].resize(10);
			}
			for (auto i = playfieldCells.begin(); i < playfieldCells.end(); i++)
			{
				for (auto b = i->begin(); b < i->end(); b++)
				{
					b->cell.pos = olc::vf2d(0 + (resolution * cellTileGridSize)*x, 0 + (resolution * cellTileGridSize )*y);
					b->cell.size = olc::vf2d(resolution*cellTileGridSize, resolution * cellTileGridSize);
					for (uint16_t j = 0; j < cellTileGridSize; j++)
					{
						std::vector<Tile> tilesToAdd;
						for (uint16_t k = 0; k < cellTileGridSize; k++)
						{
							Tile t;
							t.tiletype = -1;
							t.east = { -1,-1,-1 };
							t.west = { -1,-1,-1 };
							t.north = { -1,-1,-1 };
							t.south = { -1,-1,-1 };
							olc::vf2d dPos, dSize;
							dPos = b->cell.pos + olc::vf2d{ float(k * resolution),float(j * resolution) };
							dSize.x = resolution;
							dSize.y = resolution;
							t.rect.pos = dPos;
							t.rect.size = dSize;

							tilesToAdd.push_back(t);
						}
						b->tilesInCell.push_back(tilesToAdd);
					}
					x++;
					playFieldRect.size.x = b->cell.pos.x + 10;
					playFieldRect.size.y = b->cell.pos.y + 10;
				}
				x = 0;
				y++;
			}
			
		}
	};
	Pallete pallette;
	PlayField playField;
	std::vector<tetriminoes>tetrisTiles;
	tetriminoes selectedTile;
	void drawTiles(float fElapsedTime)
	{
		for (int cellY = 0; cellY < playField.playfieldCells.size(); cellY++)
		{
			for (int cellX = 0; cellX < playField.playfieldCells[cellY].size(); cellX++)
			{
				for (int y = 0; y < playField.playfieldCells[cellY][cellX].tilesInCell.size(); y++)
				{
					for (int x = 0; x < playField.playfieldCells[cellY][cellX].tilesInCell[y].size(); x++)
					{
						if (playField.playfieldCells[cellY][cellX].tilesInCell[y][x].tiletype == -1)
						{
							DrawRect(playField.playfieldCells[cellY][cellX].tilesInCell[y][x].rect.pos, 
								playField.playfieldCells[cellY][cellX].tilesInCell[y][x].rect.size, olc::GREY);
						}
					}
				}
			}
		}
	}
	void drawTetriminoes(float fElapsedTime)
	{
		
		for (auto i = pallette.pallettetetrisTiles.begin(); i < pallette.pallettetetrisTiles.end(); i++)
		{
			for (auto b	 = i->drawBlockPositions.begin(); b < i->drawBlockPositions.end(); b++)
			{
				for (auto c = b->begin(); c < b->end(); c++)
				{
					if (c->x != -1)//check if block position is validi->sizePerBlocki->sizePerBlock
					{
						if (i->selected == false)
						{
							FillRectDecal(*c + i->pallettePos, { 10,10 });
						}
						else
						{
							FillRectDecal(*c + i->pallettePos, { 10,10 }, olc::YELLOW);
						}
					}
				}
			
			}
		}
		if (m_BeginCollapse)
		{
			for (auto i = playField.tetrisTiles.begin(); i < playField.tetrisTiles.end(); i++)
			{
				for (auto x = i->blockPositions.begin(); x < i->blockPositions.end(); x++)
				{
					for (auto c = x->begin(); c < x->end(); c++)
					{
						FillRectDecal(i->placedPos + *c, i->sizePerBlock);
					}
				}
			}
		}
		else
		{
			if (selectedTile.tileType != 8)
			{
				for (auto i = selectedTile.blockPositions.begin(); i < selectedTile.blockPositions.end(); i++)
				{

					for (auto b = i->begin(); b < i->end(); b++)
					{
						olc::Pixel col = olc::WHITE;
						if (selectedTile.unplacable)//
						{
							col = olc::RED;
							//FillRectDecal(olc::vf2d((GetMousePos().x + b->x)-selectedTile.sizePerBlock.x, (GetMousePos().y + b->y) - selectedTile.sizePerBlock.y), selectedTile.sizePerBlock);
						}
						//else
						//{
						//	//FillRectDecal(olc::vf2d(GetMousePos().x + b->x, GetMousePos().y + b->y), selectedTile.sizePerBlock, olc::RED); - selectedTile.sizePerBlock.x*2 - selectedTile.sizePerBlock.y*2
						//}
						FillRectDecal(olc::vf2d((GetMousePos().x + b->x), (GetMousePos().y + b->y)), selectedTile.sizePerBlock, col);
					}
				}
			}
		}
	}
	void drawPlayfield(float fElapsedTime)
	{
		for (auto i = playField.playfieldCells.begin(); i < playField.playfieldCells.end(); i++)
		{
			for (auto b = i->begin(); b < i->end(); b++)
			{
				if (b->notempty == false)
				{
					DrawRect(b->cell.pos, b->cell.size, olc::RED);
				}
				
			}
		}
	}
	void drawPalletteField(float fElapsedTime)
	{
		FillRectDecal(pallette.palleteRect.pos, pallette.palleteRect.size, olc::Pixel(200, 200, 200, 200));
	}

	void drawCollapsedTiles(float fElapsedTime)
	{
		for (int cellY = 0; cellY < playField.playfieldCells.size(); cellY++)
		{
			for (int cellX = 0; cellX < playField.playfieldCells[cellY].size(); cellX++)
			{
				for (int y = 0; y < playField.playfieldCells[cellY][cellX].tilesInCell.size(); y++)
				{
					for (int x = 0; x < playField.playfieldCells[cellY][cellX].tilesInCell[y].size(); x++)
					{
						if (!playField.playfieldCells[cellY][cellX].tilesInCell[y][x].collapsed)
						{
						}
						else
						{
							int16_t ttype = playField.playfieldCells[cellY][cellX].tilesInCell[y][x].tiletype;
							if (ttype == -1) ttype = 7;
							DrawPartialDecal(playField.playfieldCells[cellY][cellX].tilesInCell[y][x].rect.pos, playField.playfieldCells[cellY][cellX].tilesInCell[y][x].rect.size,
								SpriteSheets_Vec[playField.playfieldCells[cellY][cellX].tilesInCell[y][x].tileToDraw.SheetNumber].sheetDecal.get(),
								AllBaseTiles[ttype][playField.playfieldCells[cellY][cellX].tilesInCell[y][x].tileNumber].sourcpos,
								olc::vf2d{ (float)m_resolution,(float)m_resolution });
						}
					}
				}
			}
		}
	}

	void drawScreen(float fElapsedTime)
	{
		drawPalletteField(fElapsedTime);
		drawTiles(fElapsedTime);
		drawPlayfield(fElapsedTime);
		drawTetriminoes(fElapsedTime);
		drawCollapsedTiles(fElapsedTime);
	}
	
	void initTetriminoes()
	{
		for (int i = 0; i < 7; i++)
		{
			tetriminoes newtetristile(i, { float(m_resolution*m_cellTileGridSize),float(m_resolution * m_cellTileGridSize) });
			pallette.pallettetetrisTiles.emplace_back(newtetristile);
		}
		float x = 0;
		for (auto i = pallette.pallettetetrisTiles.begin(); i < pallette.pallettetetrisTiles.end(); i++)
		{
			i->pallettePos = { (pallette.palleteRect.pos.x+5) +100 *x,pallette.palleteRect.pos.y +20  };
			x++;
		}
		bool breaker = true;
	}
	void initPallette()
	{
		pallette.palleteRect.pos = olc::vf2d( 5,ScreenHeight() - 150 );
		pallette.palleteRect.size = olc::vf2d(ScreenWidth() - 5,140 );
	}
	void makePng() {
		
	// Get thewn handle to the window
	
		
		HWND hWnd = olc::platform->GetHwnd();// replace with your co
		//if (hWnd != nullptr)
		//{
		//	// Get the process ID of the process that owns the window
		//	DWORD processId;
		//	GetWindowThreadProcessId(hWnd, &processId);

		//	// Open the process with PROCESS_VM_READ permission
		//	HANDLE processHandle = OpenProcess(PROCESS_VM_READ, FALSE, processId);

		//	if (processHandle != nullptr)
		//	{
		//		// Allocate a buffer to hold the title of the window
		//		const size_t bufferSize = 256;
		//		LPWSTR buffer = new WCHAR[bufferSize];

		//		// Read the window title from the process memory
		//		if (ReadProcessMemory(processHandle, (LPCVOID)(hWnd + GWL_STYLE), buffer, bufferSize, nullptr))
		//		{
		//			// Display the window title
		//			std::cout << "Window title: " << buffer ;
		//		}
		//		else
		//		{
		//			std::cerr << "Failed to read process memory";
		//		}

		//		// Free the buffer
		//		delete[] buffer;

		//		// Close the process handle
		//		CloseHandle(processHandle);
		//	}
		//	else
		//	{
		//		std::cerr << "Failed to open process" ;
		//	}
		//}
		//else
		//{
		//	std::cerr << "Failed to find window";
		//}
		// Define the area to capture as a rectangle
		int x = playField.playFieldRect.pos.x; // replace with your code
		int y = playField.playFieldRect.pos.y; // replace with your code
		int w = playField.playFieldRect.size.x; // replace with your code
		int h = playField.playFieldRect.size.y; // replace with your code
		RECT rect = { x, y, x + w, y + h };

		// Capture the region to a memory bitmap
		BYTE* image_data = nullptr ;
		int width, height;
		if (capture_window_to_bitmap(hWnd, x, y,w, h, image_data, width, height)) {
			// Save the captured image to a PNG file
			save_Png("captured_image.png", image_data, width, height, x, y, w, h);
			//delete[] image_data;
		}
		
	}
	bool capture_window_to_bitmap(HWND hWnd, int x, int y, int w, int h, BYTE*& image_data, int& width, int& height) {
		// Get the window's device context

		HDC hdcWindow = GetDC(hWnd);

		// Get the dimensions of the window
		RECT rect;
		//GetClientRect(hWnd, &rect);
		int window_width = w;
		int window_height = h;

		// Create a memory device context and a memory bitmap
		HDC hdcMem = CreateCompatibleDC(hdcWindow);
		HBITMAP hBitmap = CreateCompatibleBitmap(hdcWindow, window_width, window_height);

		// Select the memory bitmap into the memory device context
		HBITMAP hBitmapOld = (HBITMAP)SelectObject(hdcMem, hBitmap);

		// Copy the contents of the window to the memory device context
		bool bsuccess =BitBlt(hdcMem, 0, 0, window_width, window_height, hdcWindow, 0, 0, SRCCOPY);

		// Get the dimensions of the captured region
		if (bsuccess)
		{


			width = w;
			height = h;

			// Allocate memory for the image data
			image_data = new BYTE[w * h * 4]; // RGBA
			std::cout << " " + *image_data;
			// Copy the image data from the bitmap to the image buffer
			BITMAPINFO bmi = { 0 };
			bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			bmi.bmiHeader.biWidth = window_width;
			bmi.bmiHeader.biHeight = -window_height; // negative height to indicate top-down bitmap
			bmi.bmiHeader.biPlanes = 1;
			bmi.bmiHeader.biBitCount = 32;
			//bmi.bmiHeader.biCompression = BI_RGB;
			HBITMAP hDIB = CreateDIBSection(hdcMem, &bmi, DIB_RGB_COLORS, (void**)&image_data, NULL, 0);
			if (hDIB!=NULL)
			{
				int iBytes = (((width * 32) + 31) / 32) * 4 * height;
				bsuccess = GetBitmapBits(hBitmap, iBytes, image_data);
				if (bsuccess)
				{
					
				}
			}
			//GetDIBits(hdcMem, hBitmap, y, h, image_data, &bmi, DIB_RGB_COLORS);

			// Deselect the memory bitmap and clean up
			SelectObject(hdcMem, hBitmapOld);
			DeleteObject(hBitmap);
			DeleteDC(hdcMem);
			ReleaseDC(hWnd, hdcWindow);
		}

		return bsuccess;
	}
	bool save_Png(const char* filename, BYTE* image_data, int width, int height, int x, int y, int w, int h) 
	{
		// Open the PNG file for writing
		FILE* fp;
		errno_t err = fopen_s(&fp, filename, "wb");
		if (err != 0 || !fp) {
			fprintf(stderr, "Error: could not open PNG file %s for writing\n", filename);
			return false;
		}
		/*FILE* fp = fopen(filename, "wb");
		if (!fp) {
			std::cerr << "Error: could not open PNG file for writing" << std::endl;
			return false;
		}*/
		png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
		if (!png_ptr) {
			std::cerr << "Error: could not initialize PNG write struct" << std::endl;
			fclose(fp);
			return false;
		}
		png_infop info_ptr = png_create_info_struct(png_ptr);
		if (!info_ptr) {
			std::cerr << "Error: could not initialize PNG info struct" << std::endl;
			png_destroy_write_struct(&png_ptr, nullptr);
			fclose(fp);
			return false;
		}
		if (setjmp(png_jmpbuf(png_ptr))) {
			std::cerr << "Error: an error occurred while writing the PNG file" << std::endl;
			png_destroy_write_struct(&png_ptr, &info_ptr);
			fclose(fp);
			return false;
		}
		png_init_io(png_ptr, fp);
		png_set_IHDR(png_ptr, info_ptr, w, h, 8, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
		png_byte** row_pointers = new png_byte * [h];
		for (int i = 0; i < h; ++i) {
			row_pointers[i] = &image_data[((y + i) * width + x) * 4]; // RGBA
		}
		// Write the PNG image data
		png_write_info(png_ptr, info_ptr);
		png_write_image(png_ptr, row_pointers);
		png_write_end(png_ptr, nullptr);

		// Clean up memory and close the PNG file
		delete[] row_pointers;
		png_destroy_write_struct(&png_ptr, &info_ptr);
		fclose(fp);

		return true;
	}
	
		
	

	bool CheckCollision(olc::vf2d objapos, olc::vf2d objbpos, olc::vf2d objaSize, olc::vf2d objbsize)

	{

		if (objapos.x < objbpos.x + objbsize.x && objapos.x + objaSize.x> objbpos.x) {
			if (objapos.y < objbpos.y + objbsize.y && objapos.y + objaSize.y > objbpos.y)
			{
				return true;
			}
		}
		return false;
	}

	bool BeginColapse(PlayField& playboard, float fElapsedTime)
	{
		for (uint32_t celly = 0; celly < playboard.playfieldCells.size(); celly++)
		{
			for (uint32_t cellx = 0; cellx < playboard.playfieldCells[celly].size(); cellx++)
			{
				for (uint32_t y = 0; y < playboard.playfieldCells[celly][cellx].tilesInCell.size(); y++)
				{
					for (uint32_t x = 0; x < playboard.playfieldCells[celly][cellx].tilesInCell[y].size(); x++)
					{
// && playboard.playfieldCells[celly][cellx].tilesInCell[y][x].tileNumber == -1
						if (playboard.playfieldCells[celly][cellx].tilesInCell[y][x].tiletype >= 0&& !playboard.playfieldCells[celly][cellx].tilesInCell[y][x].collapsed)
						{
							CheckGridForCollapse(playboard);
							return true;
						}
					}
				}
			}
		}
		return false;
	}

	void CheckGridForCollapse(PlayField& playboard)
	{
		//std::vector<std::vector<Tile>> playboard_ = playboard;
		Tile* t = nullptr;// = playboard_[0][0];
		uint16_t lowestpossible = UINT16_MAX;
		for (uint32_t celly = 0; celly < playboard.playfieldCells.size(); celly++)
		{
			for (uint32_t cellx = 0; cellx < playboard.playfieldCells[celly].size(); cellx++)
			{
				for (uint32_t y = 0; y < playboard.playfieldCells[celly][cellx].tilesInCell.size(); y++)
				{
					for (uint32_t x = 0; x < playboard.playfieldCells[celly][cellx].tilesInCell[y].size(); x++)
					{
						//TODO
						//pick the next tile and the surrounding tiles, compare after this loop so each tile does a comparison one time
						//remove this todo segment once done
						if (playboard.playfieldCells[celly][cellx].tilesInCell[y][x].tiletype < 0) continue;
						if (playboard.playfieldCells[celly][cellx].tilesInCell[y][x].collapsed) continue;
						if (x > 0) CompareTiles(playboard.playfieldCells[celly][cellx].tilesInCell[y][x], playboard.playfieldCells[celly][cellx].tilesInCell[y][x - 1], 3);
						else if(cellx>0) CompareTiles(playboard.playfieldCells[celly][cellx].tilesInCell[y][x], playboard.playfieldCells[celly][cellx-1].tilesInCell[y][m_cellTileGridSize-1], 3);
						if (y > 0) CompareTiles(playboard.playfieldCells[celly][cellx].tilesInCell[y][x], playboard.playfieldCells[celly][cellx].tilesInCell[y - 1][x], 0);
						else if (celly > 0) CompareTiles(playboard.playfieldCells[celly][cellx].tilesInCell[y][x], playboard.playfieldCells[celly-1][cellx].tilesInCell[m_cellTileGridSize-1][x], 0);
						if (x < m_cellTileGridSize - 1) CompareTiles(playboard.playfieldCells[celly][cellx].tilesInCell[y][x], playboard.playfieldCells[celly][cellx].tilesInCell[y][x + 1], 1);
						else if (cellx < playboard.playfieldCells[celly].size()-1) CompareTiles(playboard.playfieldCells[celly][cellx].tilesInCell[y][x], playboard.playfieldCells[celly][cellx + 1].tilesInCell[y][0], 1);
						if (y < m_cellTileGridSize - 1) CompareTiles(playboard.playfieldCells[celly][cellx].tilesInCell[y][x], playboard.playfieldCells[celly][cellx].tilesInCell[y + 1][x], 2);
						else if (celly < playboard.playfieldCells.size()-1) CompareTiles(playboard.playfieldCells[celly][cellx].tilesInCell[y][x], playboard.playfieldCells[celly+1][cellx].tilesInCell[0][x], 2);
						if (t == nullptr)t = &playboard.playfieldCells[celly][cellx].tilesInCell[y][x];
						if (playboard.playfieldCells[celly][cellx].tilesInCell[y][x].tilesAvailable.size() < lowestpossible)
						{
							lowestpossible = playboard.playfieldCells[celly][cellx].tilesInCell[y][x].tilesAvailable.size();
							t = &playboard.playfieldCells[celly][cellx].tilesInCell[y][x];
						}

					}
				}
			}
		}
		/*for (uint32_t y = 0; y < gridY; y++)
		{
			for (uint32_t x = 0; x < gridX; x++)
			{
				if (playboard_[y][x].tiletype >= 0)
				{
					
				}
			}
		}*/
		//collapse t
		if (t == nullptr) return;

		std::uniform_int_distribution<uint16_t> dist(0, t->tilesAvailable.size() - 1);
		uint16_t tA = 0;
		if (t->tilesAvailable.size() > 0) tA = dist(mtEngine);
		t->tileToDraw = t->tilesAvailable[tA];
		t->tileNumber = t->tilesAvailable[tA].tileNumber;
		t->tiletype = t->tilesAvailable[tA].tiletype;
		int16_t ttype = t->tiletype;
		if (ttype == -1) ttype = 7;
		t->east = AllBaseTiles[ttype][t->tileNumber].east;
		t->north = AllBaseTiles[ttype][t->tileNumber].north;
		t->west = AllBaseTiles[ttype][t->tileNumber].west;
		t->south = AllBaseTiles[ttype][t->tileNumber].south;
		t->collapsed = true;

		//add puff animation and screen shake
		//playboard = playboard_;
	}

	void selectTile()
	{
		if (m_BeginCollapse)return;
		for (auto i = pallette.pallettetetrisTiles.begin(); i < pallette.pallettetetrisTiles.end(); i++)
		{
			for (auto b = i->drawBlockPositions.begin(); b < i->drawBlockPositions.end(); b++)
			{
				for (auto c = b->begin(); c < b->end(); c++)
				{
					if (CheckCollision(olc::vf2d(GetMousePos()), *c + i->pallettePos, { 50,50 }, { 10,10 }))
					{
						i->selected = true;
					}
					else
					{
						i->selected = false;
					}
				}
			}
		}
		for (auto i = pallette.pallettetetrisTiles.begin(); i < pallette.pallettetetrisTiles.end(); i++)
		{
			if (i->selected == true)
			{
				if (GetMouse(olc::Mouse::LEFT).bReleased)
				{
					selectedTile = *i;
				}


			}
		}
	}
	olc::vf2d nearestCell(olc::vf2d posToCheck)
	{
		//need to have a start pos and end pos added to struct for finding cells and divide mouse pos by block size
		/*olc::vf2d maxXvec = { 0,0 };
		for (auto i = selectedTile.blockPositions.begin(); i < selectedTile.blockPositions.end(); i++)
		{
			for (auto b = i->begin(); b < i->end(); b++)
			{
				if (b->x > maxXvec.x)
				{
					maxXvec = *b;
				}
			}
		}*/
		
		olc::vf2d nearest_vec = playField.playfieldCells[0][0].cell.pos; // Start with the first vec2selectedTile.firstPosselectedTile.firstPos
		float min_distance = distance(posToCheck, nearest_vec);
		for (auto i = playField.playfieldCells.begin(); i < playField.playfieldCells.end(); i++)
		{
			for (auto c = i->begin(); c < i->end(); c++)
			{
				float d = distance(posToCheck, c->cell.pos);
				if (d < min_distance) {
					min_distance = d;
					nearest_vec = c->cell.pos;
				}
			}
		}
		return nearest_vec;
	}
	float distance(const olc::vf2d& a, const olc::vf2d& b) {
		return std::sqrt(std::pow(b.x - a.x, 2) + std::pow(b.y - a.y, 2));
	}

	void placeTile()
	{// { 50,50 }
		if (m_BeginCollapse)return;
		selectedTile.unplacable = false;
		for (auto s = selectedTile.blockPositions.begin(); s < selectedTile.blockPositions.end(); s++)
		{
			for (auto t = s->begin(); t < s->end(); t++)
			{
				for (auto i = playField.tetrisTiles.begin(); i < playField.tetrisTiles.end(); i++)
				{
					for (auto b = i->blockPositions.begin(); b < i->blockPositions.end(); b++)
					{
						for (auto j = b->begin(); j < b->end(); j++)
						{
							if (CheckCollision(olc::vf2d(nearestCell(GetMousePos())) + *t, *j + i->placedPos, i->sizePerBlock, i->sizePerBlock))
							{
								selectedTile.unplacable = true;
							}
						}
					}
				}
				olc::vf2d posToCheck = nearestCell(GetMousePos()) + *t;
				if (posToCheck.x + selectedTile.sizePerBlock.x > playField.playfieldCells[0].size() * m_cellTileGridSize * m_resolution || posToCheck.y + selectedTile.sizePerBlock.y > playField.playfieldCells.size() * m_cellTileGridSize * m_resolution || GetMousePos().x < 0 || GetMousePos().y < 0)
				{
					return true;
				}
			}
		}
		//for (auto i = selectedTile.blockPositions.begin(); i < selectedTile.blockPositions.end(); i++)
		//{
		//	for (auto b = i->begin(); b < i->end(); b++)
		//	{
		//		if (CheckCollision(olc::vf2d(GetMousePos())+*b, pallette.palleteRect.pos, selectedTile.sizePerBlock, pallette.palleteRect.size))
		//		{
		//			selectedTile.unplacable = true;
		//		}
		//	/*	else
		//		{
		//			selectedTile.unplacable = false;
		//		}*/
		//	}
		//}
		if (selectedTile.unplacable == true)
		{
			bool breakthis = true;
		}
		if (GetKey(olc::SPACE).bReleased)
		{
 			bool breaker = true;
		}
		if (GetMouse(olc::Mouse::LEFT).bReleased)
		{
			if (selectedTile.unplacable == false)
			{
				olc::vf2d celltoplace = nearestCell(GetMousePos());
				selectedTile.placedPos = olc::vf2d(celltoplace);
				playField.tetrisTiles.emplace_back(selectedTile);
				setCellTypes(playField);
				m_BeginCollapse = true;
			}
		}
	}

	void setCellTypes(PlayField& playfield_)
	{
		for (int tet = 0; tet < playfield_.tetrisTiles.size(); tet++)
		{
			for (int blockY = 0; blockY < playfield_.tetrisTiles[tet].blockPositions.size(); blockY++)
			{
				for (int blockX = 0; blockX < playfield_.tetrisTiles[tet].blockPositions[blockY].size(); blockX++)
				{
					for (int y = 0; y < playfield_.playfieldCells.size(); y++)
					{
						for (int x = 0; x < playfield_.playfieldCells[y].size(); x++)
						{
							if (CheckCollision(playfield_.tetrisTiles[tet].blockPositions[blockY][blockX]+ playfield_.tetrisTiles[tet].placedPos, playfield_.playfieldCells[y][x].cell.pos,
								playfield_.tetrisTiles[tet].sizePerBlock, playfield_.playfieldCells[y][x].cell.size))
							{
								for (int cy = 0; cy < playfield_.playfieldCells[y][x].tilesInCell.size(); cy++)
								{
									for (int cx = 0; cx < playfield_.playfieldCells[y][x].tilesInCell[cy].size(); cx++)
									{
										if (!playfield_.playfieldCells[y][x].tilesInCell[cy][cx].collapsed)
										{
											playfield_.playfieldCells[y][x].tilesInCell[cy][cx].tiletype = selectedTile.tileType;
											playfield_.playfieldCells[y][x].tilesInCell[cy][cx].tilesAvailable = AllBaseTiles[playfield_.playfieldCells[y][x].tilesInCell[cy][cx].tiletype];
											for (int i = 0; i < AllBaseTiles[7].size(); i++)
											{
												playfield_.playfieldCells[y][x].tilesInCell[cy][cx].tilesAvailable.push_back(AllBaseTiles[7][i]);
											}
											RemoveEmptyTiles(playfield_.playfieldCells[y][x].tilesInCell[cy][cx]);
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
	void ExplodePlayfield(PlayField& playfield_)
	{
		for (int tet = 0; tet < playfield_.tetrisTiles.size(); tet++)
		{
			for (int blockY = 0; blockY < playfield_.tetrisTiles[tet].blockPositions.size(); blockY++)
			{
				for (int blockX = 0; blockX < playfield_.tetrisTiles[tet].blockPositions[blockY].size(); blockX++)
				{
					for (int y = 0; y < playfield_.playfieldCells.size(); y++)
					{
						for (int x = 0; x < playfield_.playfieldCells[y].size(); x++)
						{
							if (!CheckCollision(playfield_.tetrisTiles[tet].blockPositions[blockY][blockX] + playfield_.tetrisTiles[tet].placedPos, playfield_.playfieldCells[y][x].cell.pos,
								playfield_.tetrisTiles[tet].sizePerBlock, playfield_.playfieldCells[y][x].cell.size))
							{
								std::uniform_int_distribution<uint16_t> dist(0, 6);
								int16_t tempType = dist(mtEngine);
								for (int cy = 0; cy < playfield_.playfieldCells[y][x].tilesInCell.size(); cy++)
								{
									for (int cx = 0; cx < playfield_.playfieldCells[y][x].tilesInCell[cy].size(); cx++)
									{
										if (!playfield_.playfieldCells[y][x].tilesInCell[cy][cx].collapsed)
										{

											playfield_.playfieldCells[y][x].tilesInCell[cy][cx].tiletype = tempType;
											playfield_.playfieldCells[y][x].tilesInCell[cy][cx].tilesAvailable = AllBaseTiles[playfield_.playfieldCells[y][x].tilesInCell[cy][cx].tiletype];
											for (int i = 0; i < AllBaseTiles[7].size(); i++)
											{
												playfield_.playfieldCells[y][x].tilesInCell[cy][cx].tilesAvailable.push_back(AllBaseTiles[7][i]);
											}
											RemoveEmptyTiles(playfield_.playfieldCells[y][x].tilesInCell[cy][cx]);
										}
									}
								}
							}
						}
					}
				}
			}
		}
		m_BeginCollapse = true;
	}
	void controls(float fElapsedTime)
	{
		
		selectTile();
		placeTile();
		keyBoardcontrols();
	}
	void updateMouse()
	{
		//if (GetMousePos().y < pallette.palleteRect.pos.y)
		//{// = false;
		//	ShowCursor(false);
		//}
		//else
		//{
		//	ShowCursor(true);
		//}
	}
	void updateSelectedTile()
	{
		for (auto i = selectedTile.blockPositions.begin(); i < selectedTile.blockPositions.end(); i++)
		{
			for (auto b = i->begin(); b < i->end(); b++)
			{
				*b + olc::vf2d(GetMousePos());
			}
		}
	}
	bool OnUserCreate() override
	{
		//std::random_device rd();
		mtEngine.seed(1794);
		InitializeSpritesDecals();
		SetBuiltInEdges();
		playField = PlayField(m_resolution, m_cellTileGridSize);
		initPallette();
		initTetriminoes();
		return true;
	}
	bool OnUserUpdate(float fElapsedTime) override
	{
		updateSelectedTile();
		controls(fElapsedTime);
		UpdatePlayfield(fElapsedTime);
		updateMouse();
		drawScreen(fElapsedTime);
		return true;
	}

};
int main()
{
	//srand(time(NULL));
//#ifdef _WIN32
//
//	ShowCursor(false);
//#endif // _WIN32

	Engine Game;
	if (Game.Construct(1200, 900, 1, 1))
	{
	

		Game.Start();
	}
	return 0;
}