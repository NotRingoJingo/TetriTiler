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

namespace fs = std::filesystem;


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
	struct Edge
	{
		uint16_t cornerA, side, cornerB;
	};
	struct SpriteTile
	{
		uint16_t tiletype;
		uint16_t tileNumber;
		Edge north, east, south, west;
	};
	struct Tile
	{
		bool flippedH = false;
		bool flippedV = false;
		uint16_t tiletype;
		uint16_t tileNumber = -1;
		Edge north, east, south, west;
		Rect rect;
		std::vector<SpriteTile> tilesAvailable;
	};
	std::vector<std::vector<SpriteTile>> AllBaseTiles;
	void SetEdge(SpriteTile& t, int edgeToSet, uint16_t a, uint16_t s, uint16_t b)
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
	void CompareTiles(Tile& a, Tile& b,int edgeToCompare)
	{
		if (b.tiletype < 0)
		{
			return;
		}
		for (uint16_t i = 0; i < a.tilesAvailable.size(); i++)
		{
			Edge bEdgeTranslated = a.tilesAvailable[i].north;
			Edge aEdgeTranslated = b.south;
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
			if (!EdgeComparison(aEdgeTranslated, bEdgeTranslated))
			{
				a.tilesAvailable.erase(a.tilesAvailable.begin() + 1);
				//a.tilesAvailable.erase(std::remove(a.tilesAvailable.begin(), a.tilesAvailable.end(), i), a.tilesAvailable.end());
				i--;
			}
		}
	}
	bool EdgeComparison(Edge a, Edge b)
	{
		if (a.cornerA == b.cornerA&& a.side == b.side && a.cornerB == b.cornerB) return true;
		return false;
	}
	bool BeginColapse(uint32_t gridX, uint32_t gridY, std::vector<std::vector<Tile>>& playboard, float fElapsedTime)
	{
		bool check = false;
		for (uint32_t y = 0; y < gridY; y++)
		{
			for (uint32_t x = 0; x < gridX; x++)
			{
				if (playboard[y][x].tiletype >= 0 && playboard[y][x].tileNumber == -1)
				{
					check = true;
				}
			}
		}
		if (!check) return check;
		CheckGridForCollapse(gridX, gridY, playboard);
		return check;
	}
	void CheckGridForCollapse(uint32_t gridX, uint32_t gridY, std::vector<std::vector<Tile>>& playboard)
	{
		std::vector<std::vector<Tile>> playboard_ = playboard;
		Tile* t = nullptr;// = playboard_[0][0];
		uint16_t lowestpossible = UINT16_MAX;
		for (uint32_t y = 0; y < gridY; y++)
		{
			for (uint32_t x = 0; x < gridX; x++)
			{
				if (playboard_[y][x].tiletype >= 0)
				{
					if (x > 0) CompareTiles(playboard_[y][x], playboard_[y][x - 1], 3);
					if (y > 0) CompareTiles(playboard_[y][x], playboard_[y-1][x], 0);
					if (x < gridX-1) CompareTiles(playboard_[y][x], playboard_[y][x + 1], 1);
					if (y < gridY-1) CompareTiles(playboard_[y][x], playboard_[y+1][x], 2);
					if (t == nullptr)t = &playboard_[y][x];
					if (playboard_[y][x].tilesAvailable.size() < lowestpossible)
					{
						lowestpossible = playboard_[y][x].tilesAvailable.size();
						t = &playboard_[y][x];
					}
				}
			}
		}
		//collapse t

		std::uniform_int_distribution<uint16_t> dist(0, t->tilesAvailable.size() - 1);

		t->tileNumber = t->tilesAvailable[dist(mtEngine)].tileNumber;
		t->east = AllBaseTiles[t->tiletype][t->tileNumber].east;
		t->north = AllBaseTiles[t->tiletype][t->tileNumber].north;
		t->west = AllBaseTiles[t->tiletype][t->tileNumber].west;
		t->south = AllBaseTiles[t->tiletype][t->tileNumber].south;

		//add puff animation and screen shake
		playboard = playboard_;
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
				spriteSheets.push_back(std::make_unique<olc::Sprite>("./assets/" + fileNames[i].filename().string()));
				//std::cout << fileNames[i].filename().string() << std::endl;
			}
		}
		for (int i = 0; i < spriteSheets.size(); i++)
		{
			vecS_Decals.push_back(std::make_unique<olc::Decal>(spriteSheets[i].get()));
			//std::cout << vecS_Decals[i].get()->sprite->height << std::endl;
		}
	}

	void SetBuiltInEdges()
	{
		uint16_t type = 0;
		uint16_t tileNum = 0;
		for (int i = 0; i < vecS_Decals.size(); i++)
		{
			std::vector<SpriteTile> st;
			for (int y = 0; y < 3; y++)
			{
				for (int x = 0; x < 3; x++)
				{
					st.push_back(SpriteTile{ type,tileNum });
					tileNum++;
				}
			}
			if (i + 1 % 7)
			{
				type += 1;
				tileNum = 0;
				AllBaseTiles.push_back(st);
			}
		}
		int spriteTileLoop = 0;
		int edgeType = 0;
		for (int i = 0; i < AllBaseTiles.size(); i++)
		{
			
			for (int j = 0; j < AllBaseTiles[i].size(); j++)
			{
				uint16_t eT = 0;
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
				switch (spriteTileLoop)
				{
				case 0:
					AllBaseTiles[i][j].north = { eT,eT,eT };
					AllBaseTiles[i][j].east = { AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype };
					AllBaseTiles[i][j].south = { AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype };
					AllBaseTiles[i][j].west = { eT,eT,eT };
					break;
				case 1:
					AllBaseTiles[i][j].north = { eT,eT,eT };
					AllBaseTiles[i][j].east = { AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype };
					AllBaseTiles[i][j].south = { AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype };
					AllBaseTiles[i][j].west = { AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype };
					break;
				case 2:
					AllBaseTiles[i][j].north = { eT,eT,eT };
					AllBaseTiles[i][j].east = { eT,eT,eT };
					AllBaseTiles[i][j].south = { AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype };
					AllBaseTiles[i][j].west = { AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype };
					break;
				case 3:
					AllBaseTiles[i][j].north = { AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype };
					AllBaseTiles[i][j].east = { AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype };
					AllBaseTiles[i][j].south = { AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype };
					AllBaseTiles[i][j].west = { eT,eT,eT };
					break;
				case 4:
					AllBaseTiles[i][j].north = { AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype };
					AllBaseTiles[i][j].east = { AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype };
					AllBaseTiles[i][j].south = { AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype };
					AllBaseTiles[i][j].west = { AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype };
					break;
				case 5:
					AllBaseTiles[i][j].north = { AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype };
					AllBaseTiles[i][j].east = { eT,eT,eT };
					AllBaseTiles[i][j].south = { AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype };
					AllBaseTiles[i][j].west = { AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype };
					break;
				case 6:
					AllBaseTiles[i][j].north = { AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype };
					AllBaseTiles[i][j].east = { AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype };
					AllBaseTiles[i][j].south = { eT,eT,eT };
					AllBaseTiles[i][j].west = { eT,eT,eT };
					break;
				case 7:
					AllBaseTiles[i][j].north = { AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype };
					AllBaseTiles[i][j].east = { AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype };
					AllBaseTiles[i][j].south = { eT,eT,eT };
					AllBaseTiles[i][j].west = { AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype };
					break;
				case 8:
					AllBaseTiles[i][j].north = { AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype };
					AllBaseTiles[i][j].east = { eT,eT,eT };
					AllBaseTiles[i][j].south = { eT,eT,eT };
					AllBaseTiles[i][j].west = { AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype,AllBaseTiles[i][j].tiletype };
					break;
				default:
					break;
				}
				spriteTileLoop++;
				if (spriteTileLoop >= 9)
				{
					spriteTileLoop = 0;
					edgeType++;
				}
			}
		}
	}

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
			for (auto i = blockPositions.begin(); i < blockPositions.end(); i++)
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
				blockStrings.push_back("0001");
				blockStrings.push_back("0001");
				blockStrings.push_back("0011");
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
						olc::vf2d newVf2d = olc::vf2d( size.x * x,size.y * y );
						blockPositions[y][x] = newVf2d;
					}
					else //give block position an invalid value
					{
						olc::vf2d negativePos = olc::vf2d{ -1.0,-1.0 };
						blockPositions[y][x] = negativePos;
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
	};
	struct PlayField
	{
		std::vector<std::vector<emptyCells>> playfieldCells;
		Rect playFieldRect;
		std::vector<tetriminoes>tetrisTiles;
		PlayField()
		{
			int x = 0;
			int y = 0;
			playfieldCells.resize(60);
			for (int i = 0; i < playfieldCells.size(); i++)
			{
				playfieldCells[i].resize(40);
			}
			for (auto i = playfieldCells.begin(); i < playfieldCells.end(); i++)
			{
				for (auto b = i->begin(); b < i->end(); b++)
				{
					b->cell.pos = olc::vf2d(0 + 10*x, 0 + 10*y);
					b->cell.size = olc::vf2d(10, 10);
					x++;
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

	void drawTetriminoes(float fElapsedTime)
	{
		
		for (auto i = pallette.pallettetetrisTiles.begin(); i < pallette.pallettetetrisTiles.end(); i++)
		{
			for (auto b	 = i->blockPositions.begin(); b < i->blockPositions.end(); b++)
			{
				for (auto c = b->begin(); c < b->end(); c++)
				{
					if (c->x != -1)//check if block position is valid
					{
						if (i->selected == false)
						{
							FillRectDecal(*c + i->pallettePos, i->sizePerBlock);
						}
						else
						{
							FillRectDecal(*c + i->pallettePos, i->sizePerBlock, olc::YELLOW);
						}
					}
				}
			
			}
		}
		for (auto i = playField.tetrisTiles.begin(); i < playField.tetrisTiles.end(); i++)
		{
			for (auto x = i->blockPositions.begin(); x <i->blockPositions.end(); x++)
			{
				for (auto c = x->begin(); c < x->end(); c++)
				{
					FillRectDecal(i->placedPos + *c, i->sizePerBlock);
				}
			}	
		}
		if (selectedTile.tileType!=8)
		{
			for (auto i = selectedTile.blockPositions.begin(); i < selectedTile.blockPositions.end(); i++)
			{
				
				for (auto b = i->begin(); b <i->end(); b++)
				{
					if (selectedTile.unplacable ==false)
					{
						FillRectDecal(olc::vf2d(GetMousePos().x + b->x, GetMousePos().y + b->y), selectedTile.sizePerBlock);
					}
					else
					{
						FillRectDecal(olc::vf2d(GetMousePos().x + b->x, GetMousePos().y + b->y), selectedTile.sizePerBlock, olc::RED);
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
				DrawRect(b->cell.pos, b->cell.size, olc::RED);
			}
		}
	}
	void drawPalletteField(float fElapsedTime)
	{
		FillRectDecal(pallette.palleteRect.pos, pallette.palleteRect.size, olc::Pixel(200, 200, 200, 200));
	}
	void drawScreen(float fElapsedTime)
	{
		drawPalletteField(fElapsedTime);
		drawPlayfield(fElapsedTime);
		drawTetriminoes(fElapsedTime);
	}
	
	void initTetriminoes()
	{
		for (int i = 0; i < 7; i++)
		{
			tetriminoes newtetristile(i, { 10,10 });
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
	void selectTile()
	{
		for (auto i = pallette.pallettetetrisTiles.begin(); i < pallette.pallettetetrisTiles.end(); i++)
		{
			for (auto b = i->blockPositions.begin(); b < i->blockPositions.end(); b++)
			{
				for (auto c = b->begin(); c < b->end(); c++)
				{
					if (CheckCollision(olc::vf2d(GetMousePos()), *c + i->pallettePos, { 50,50 }, i->sizePerBlock))
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
	olc::vf2d nearestCell()
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
		
		olc::vf2d nearest_vec = playField.playfieldCells[0][0].cell.pos; // Start with the first vec2
		float min_distance = distance(selectedTile.firstPos, nearest_vec);
		for (auto i = playField.playfieldCells.begin(); i < playField.playfieldCells.end(); i++)
		{
			for (auto c = i->begin(); c < i->end(); c++)
			{
				float d = distance(selectedTile.firstPos, c->cell.pos);
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
	{
		for (auto s = selectedTile.blockPositions.begin(); s < selectedTile.blockPositions.end(); s++)
		{
			for (auto t = s->begin(); t < s->end(); t++)
			{



				for (auto i = playField.tetrisTiles.begin(); i < playField.tetrisTiles.end(); i++)
				{
					if (CheckCollision(olc::vf2d(GetMousePos()) + *t, i->placedPos, { 50,50 }, i->sizePerBlock))
					{
						selectedTile.unplacable = true;
					}
					else
					{
						selectedTile.unplacable = false;
					}

							
				}
			}
		}
		for (auto i = selectedTile.blockPositions.begin(); i < selectedTile.blockPositions.end(); i++)
		{
			for (auto b = i->begin(); b < i->end(); b++)
			{
				if (CheckCollision(olc::vf2d(GetMousePos())+*b, pallette.palleteRect.pos, selectedTile.sizePerBlock, pallette.palleteRect.size))
				{
					selectedTile.unplacable = true;
				}
				else
				{
					selectedTile.unplacable = false;
				}
			}
		}
		if (selectedTile.unplacable == true)
		{
			bool breakthis = true;
		}
		if (GetKey(olc::SPACE).bReleased)
		{
 			bool breaker = true;
		}
		if (selectedTile.unplacable == false)
		{
			if (GetMouse(olc::Mouse::LEFT).bReleased)
			{
				olc::vf2d celltoplace = nearestCell();
				selectedTile.placedPos = olc::vf2d(celltoplace);
				playField.tetrisTiles.emplace_back(selectedTile);
			}


		}
			
		
	}
	void controls(float fElapsedTime)
	{
		
		placeTile();
		selectTile();
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
		initPallette();
		initTetriminoes();
		return true;
	}
	bool OnUserUpdate(float fElapsedTime) override
	{
		updateSelectedTile();
		controls(fElapsedTime);
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