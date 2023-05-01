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
	struct tetriminoes
	{
		std::vector<std::string> blockStrings;
		std::vector<std::vector<olc::vf2d>> blockPositions;
		olc::Pixel material;
		olc::vf2d sizePerBlock;
		olc::vf2d pallettePos;
		olc::vf2d placedPos;
		tetriminoes(int type, olc::vf2d size)
		{
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
	std::vector<tetriminoes>tetrisTiles;
	void drawTetriminoes(float fElapsedTime)
	{
		
		for (auto i = tetrisTiles.begin(); i < tetrisTiles.end(); i++)
		{
			for (auto b	 = i->blockPositions.begin(); b < i->blockPositions.end(); b++)
			{
				for (auto c = b->begin(); c < b->end(); c++)
				{
					FillRectDecal(*c + i->pallettePos, i->sizePerBlock);
				}
			
			}
		}
	}
	void initTetriminoes()
	{
		for (int i = 0; i < 7; i++)
		{
			tetriminoes newtetristile(i, { 10,10 });
			tetrisTiles.emplace_back(newtetristile);
		}
		float x = 0;
		for (auto i = tetrisTiles.begin(); i < tetrisTiles.end(); i++)
		{
			i->pallettePos = { 100,100 * x + 50 };
			x++;
		}
		bool breaker = true;
	}
	bool OnUserCreate() override
	{
		initTetriminoes();
		return true;
	}
	bool OnUserUpdate(float fElapsedTime) override
	{
		drawTetriminoes(fElapsedTime);
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