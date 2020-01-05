// Includes
#include "../ReflexEngine/Engine.h"
#include "../ReflexEngine/Camera.h"

using namespace Reflex::Core;

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

	void UpdateAnts( const float deltaTime );
	void UpdateCamera( const float deltaTime );
	void UpdateIncrementalColours( const unsigned startIdx = 1 );

	void Recolour();
	sf::Vector2i FindGridIndex( const sf::Vector2f& pos );

	void SaveCustomSetups();
	void ReadCustomSetups();

protected:
	// Data Members
	Reflex::Core::Camera camera;
	const float cameraSpeed = 500.0f;
	const float zoomSpeed = 1.1f;
	const float margin = 0.0f;
	sf::Vector2u gridSize;
	sf::Vector2f tileSize;
	sf::Vector2f gridOrigin;
	float timer = 0.5f;
	float updateTime = 0.1f;
	unsigned generation = 0;
	int stepTo = 0;

	enum BlendValue
	{
		Disable,
		Gamma,
		Alpha,
		NumBlends,
	};

	int blendIdx = Disable;

	void SetTileColour( const sf::Vector2i& index, const sf::Color& colour, const BlendValue blend = Disable );

	enum SpeedValue
	{
		VeryFast,
		Fast,
		Regular,
		Slow,
		Pause,
		NumSpeeds
	};

	int speedIdx = Regular;
	std::array< float, NumSpeeds > speedValues = { 0.001f, 0.01f, 0.08f, 0.3f, 0.0f };

	std::vector< sf::Vertex > gridVertices;
	std::vector< int > gridStates;

	unsigned currentStateInfo = 0;
	std::vector< struct StateInfo > customStates;

	bool placingAnts = false;
	bool displayAnts = false;
	unsigned activeAnts = 0;
	std::vector< struct Ant > ants;
	sf::CircleShape antPlacerDisplay;

	enum GridType
	{
		Square,
		Hexagon,
		Pixel,
		NumGridTypes,
	};

	int gridTypeIdx = Hexagon;

	bool incrementalRGB = false;
	bool incrementalAlpha = false;
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
	int statesCount = 0;
	std::vector< std::pair< std::array< float, 4 >, float > > coloursAndAngles;
};

struct Ant
{
	Ant( const sf::Vector2f& pos, const float direction = PI )
		: startingPos( pos )
		, currentPos( pos )
		, startingDir( direction )
		, currentDir( direction )
		, display( 10.0f )
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
};
