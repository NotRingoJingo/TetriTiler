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

#define M_PI 3.14159265358979323846
class Engine : public olc::PixelGameEngine
{
public:
	Engine()
	{
		sAppName = "TetriTiler";
	}
private:
	enum shapes
	{
		i = 0,
		o = 1,
		t = 2,
		j =3,
		l = 4,
		s = 5,
		z =6,
	    
	};
	struct Rect
	{
		olc::vf2d pos;
		olc::vf2d size;
	};
	
	struct tetriminoes
	{
		std::vector<std::string> blockStrings;
		std::vector<std::vector<olc::vf2d>> blockPositions;
		olc::Pixel material;
		olc::vf2d sizePerBlock;
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
			case i:
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
					x++;
				}
				x = 0;
				y++;
			}
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
					if (i->selected == false)
					{
						FillRectDecal(*c + i->pallettePos, i->sizePerBlock);
					}
					else
					{
						FillRectDecal(*c + i->pallettePos, i->sizePerBlock,olc::YELLOW);
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
	void placeTile()
	{
		for (auto i = playField.tetrisTiles.begin(); i < playField.tetrisTiles.end(); i++)
		{
			for (auto b = i->blockPositions.begin(); b < i->blockPositions.end(); b++)
			{
				for (auto c = b->begin(); c < b->end(); c++)
				{
					if (CheckCollision(olc::vf2d(GetMousePos()), *c + i->placedPos, { 50,50 }, i->sizePerBlock))
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
		for (auto i = playField.tetrisTiles.begin(); i < playField.tetrisTiles.end(); i++)
		{
			if (selectedTile.unplacable == false)
			{
				if (GetMouse(olc::Mouse::LEFT).bReleased)
				{
					
					playField.tetrisTiles.emplace_back(selectedTile);
				}


			}
		}
	}
	void controls(float fElapsedTime)
	{
		selectTile();
		placeTile();
	}
	void updateSelectedTile()
	{
		for (auto i = selectedTile.blockPositions.begin(); i < selectedTile.blockPositions.end(); i++)
		{
			for (auto b = i->begin(); b < i->end(); b++)
			{
				*b = *b + olc::vf2d(GetMousePos());
			}
		}
	}
	bool OnUserCreate() override
	{
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