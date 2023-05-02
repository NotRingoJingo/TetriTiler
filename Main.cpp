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

#define M_PI 3.14159265358979323846
class Engine : public olc::PixelGameEngine
{
public:
	Engine()
	{
		sAppName = "TetriTiler";
	}
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
	struct Tile
	{
		bool flippedH = false;
		bool flippedV = false;
		uint16_t tiletype;
		uint16_t tileNumber;
		Edge north, east, south, west;
		Rect rect;
		std::vector<uint16_t> tilesAvailable;
	};
	void SetEdge(Tile& t, int edgeToSet, uint16_t a, uint16_t s, uint16_t b)
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
			//tile is next to empty grid handle this logic
		}
		Edge bEdgeTranslated = a.north;
		Edge aEdgeTranslated = b.south;
		switch (edgeToCompare)
		{
		case 0:
			bEdgeTranslated = b.south;
			if (b.flippedV) bEdgeTranslated = b.north;
			if (b.flippedH) bEdgeTranslated = InvertEdge(bEdgeTranslated);
			aEdgeTranslated = a.north;
			if (a.flippedV) bEdgeTranslated = a.south;
			if (a.flippedH) bEdgeTranslated = InvertEdge(aEdgeTranslated);
			break;
		case 1:
			bEdgeTranslated = b.west;
			if (b.flippedH) bEdgeTranslated = b.east;
			if (b.flippedV) bEdgeTranslated = InvertEdge(bEdgeTranslated);
			aEdgeTranslated = a.east;
			if (a.flippedH) bEdgeTranslated = a.west;
			if (a.flippedV) bEdgeTranslated = InvertEdge(aEdgeTranslated);
			break;
		case 2:
			bEdgeTranslated = b.north;
			if (b.flippedV) bEdgeTranslated = b.south;
			if (b.flippedH) bEdgeTranslated = InvertEdge(bEdgeTranslated);
			aEdgeTranslated = a.south;
			if (a.flippedV) bEdgeTranslated = a.north;
			if (a.flippedH) bEdgeTranslated = InvertEdge(aEdgeTranslated);
			break;
		case 3:
			bEdgeTranslated = b.east;
			if (b.flippedH) bEdgeTranslated = b.west;
			if (b.flippedV) bEdgeTranslated = InvertEdge(bEdgeTranslated);
			aEdgeTranslated = a.west;
			if (a.flippedH) bEdgeTranslated = a.east;
			if (a.flippedV) bEdgeTranslated = InvertEdge(aEdgeTranslated);
			break;
		}
		if (!EdgeComparison(aEdgeTranslated, bEdgeTranslated))
		{
			a.tilesAvailable.erase(std::remove(a.tilesAvailable.begin(), a.tilesAvailable.end(), b.tileNumber), a.tilesAvailable.end());
		}
	}
	bool EdgeComparison(Edge a, Edge b)
	{
		if (a.cornerA == b.cornerA&& a.side == b.side && a.cornerB == b.cornerB) return true;
		return false;
	}

private:

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