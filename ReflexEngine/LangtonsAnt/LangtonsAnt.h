// Includes
#include "../ReflexEngine/Engine.h"

using namespace Reflex::Core;

enum BlendValue
{
	Disable,
	Gamma,
	Alpha,
	NumBlendTypes,
};

enum GridType
{
	Square,
	Hexagon,
	Pixel,
	NumGridTypes,
};

class GameState : public State
{
public:
	GameState( StateManager& stateManager, Context context );

protected:
	void SetupGrid();
	void Reset( const bool resetGridCcolours = true );

	void Update( const float deltaTime ) final;
	void ProcessEvent( const sf::Event& event ) final;
	void Render() final;

	void UpdateAntsThread();
	void UpdateAnts( const float deltaTime );
	void UpdateCamera( const float deltaTime );
	void UpdateIncrementalColours( const unsigned startIdx = 1 );
	void ColourSmooth();

	void Recolour();
	void RandomiseParameters();
	sf::Vector2i FindGridIndex( const sf::Vector2f& pos );
	void SetTileColour( const sf::Vector2i& index, const sf::Color& colour, const BlendValue blend = Disable );
	struct StateInfo& GetInfo() { return customStates[currentStateInfo]; }
	sf::Vector2f LockToGrid( const sf::Vector2f& v );
	void RandomiseAngles( const bool gridAligned = false );

	void SaveCustomSetups();
	void ReadCustomSetups();

protected:
	// Data Members
	Reflex::Components::CameraHandle camera;
	const float cameraSpeed = 500.0f;
	const float zoomSpeed = 1.05f;
	const float margin = 0.0f;
	float size = 10.0f;
	sf::Vector2u gridSize;
	sf::Vector2f tileSize;
	sf::Vector2f gridOrigin;
	float timer = 0.5f;
	float updatesPerSec = 100;
	unsigned generation = 0;
	int stepTo = 0;

	std::vector< sf::Vertex > gridVertices;
	std::vector< int > gridStates;

	unsigned currentStateInfo = 0;
	std::vector< struct StateInfo > customStates;

	bool placingAnts = false;
	bool angleAnt = false;
	bool displayAnts = false;
	bool logToConsole = false;
	unsigned activeAnts = 0;
	std::vector< struct Ant > ants;
	sf::CircleShape antPlacerDisplay;
	sf::RectangleShape antPlacerDisplay2;

	std::unique_ptr< sf::Thread > processThread;
};

struct StateInfo
{
	StateInfo( const std::string& name ) : name( name ) { }
	void Append( const sf::Color& colour, const float angleRadians )
	{
		coloursAndAngles.push_back( std::pair( Reflex::ToImGuiColour4( colour ), angleRadians ) );
		++statesCount;
	}

	std::string name;

	bool incrementalRGB = false;
	bool incrementalAlpha = false;
	bool infiniteGrid = false;
	bool lockToGrid = false;

	int statesCount = 0;
	std::vector< std::pair< std::array< float, 4 >, float > > coloursAndAngles;

	int blendIdx = Disable;
	int gridTypeIdx = Square;

	std::vector< struct Ant > ants;
};

struct Ant
{
	Ant( const sf::Vector2f& pos, const float direction = PI, const sf::Color& colour = sf::Color::White )
		: startingPos( pos )
		, currentPos( pos )
		, startingDir( direction )
		, currentDir( direction )
		, display( 2.0f )
		, hue( colour )
	{ 
		display.setFillColor( sf::Color::Transparent );
		display.setOutlineColor( sf::Color::Red );
		display.setOutlineThickness( 1.0f );
		Reflex::CenterOrigin( display );
	}

	const sf::Vector2f startingPos;
	sf::Vector2f currentPos;
	const float startingDir = PI;
	float currentDir = PI;
	bool active = true;
	sf::CircleShape display;
	sf::Color hue;
};
