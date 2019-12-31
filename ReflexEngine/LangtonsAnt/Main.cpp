// Includes
#include "../ReflexEngine/Engine.h"
#include "../ReflexEngine/Utility.h"

using namespace Reflex::Core;

class GameState;

// Entry point
int main()
{
	srand( (unsigned )time( 0 ) );

	Engine engine( "Langton's Ant" );
	engine.RegisterState< GameState >( true );
	engine.Run();

	return 0;
}

class GameState : public State
{
public:
	GameState( StateManager& stateManager, Context context )
		: State( stateManager, context )
		, width( int( context.window.getSize().x / size + 0.5f ) )
		, height( int( context.window.getSize().y / size + 0.5f ) )
	{
		board.reserve( width * height * 4 );
		states.resize( width * height );
		const auto origin = sf::Vector2f( GetWindow().getSize().x / 2.0f - width / 2.0f * size, GetWindow().getSize().y / 2.0f - height / 2.0f * size );

		for( unsigned y = 0; y < height; ++y )
		{
			for( unsigned x = 0; x < width; ++x )
			{
				const auto pos = origin + sf::Vector2f( x * size, y * size );
				const auto dimensions = sf::Vector2f( size - margin, size - margin ) / 2.0f;

				sf::Vertex topLeft, topRight, botLeft, botRight;
				topLeft.position = pos + sf::Vector2f( -dimensions.x, -dimensions.y );
				topRight.position = pos + sf::Vector2f( dimensions.x, -dimensions.y );
				botLeft.position = pos + sf::Vector2f( -dimensions.x, dimensions.y );
				botRight.position = pos + sf::Vector2f( dimensions.x, dimensions.y );

				topLeft.color = sf::Color::Black;
				topRight.color = sf::Color::Black;
				botLeft.color = sf::Color::Black;
				botRight.color = sf::Color::Black;

				board.push_back( topLeft );
				board.push_back( botLeft );
				board.push_back( botRight );
				board.push_back( topRight );
			}
		}

		ants.emplace_back( sf::Vector2i( width / 2, height / 2 ) );
	}

protected:
	void Render() final
	{
		GetWindow().draw( board.data(), board.size(), sf::Quads );
	}

	void Update( const float deltaTime ) final
	{
		timer -= deltaTime;

		//while( timer <= 0.0f)
		{
			timer += 0.00f;

			for( int i = ants.size() - 1; i >= 0; --i )
			{
				// Rotate
				auto& ant = ants[i];
				const auto idx = ant.index.y * width + ant.index.x;
				assert( idx >= 0 && idx < states.size() );
				const auto active = states[idx];
				ant.direction = Ant::Direction( Reflex::Mod( ( ant.direction + ( active ? 1 : -1 ) ), Ant::NumDirections ) );

				// Flip tile
				assert( idx >= 0 && idx * 4 + 3 < board.size() );
				states[idx] = !active;
				board[idx * 4].color = active ? sf::Color::White : sf::Color::Black;
				board[idx * 4 + 1].color = active ? sf::Color::White : sf::Color::Black;
				board[idx * 4 + 2].color = active ? sf::Color::White : sf::Color::Black;
				board[idx * 4 + 3].color = active ? sf::Color::White : sf::Color::Black;

				// Move
				if( ( ant.direction & 1 ) == 0 )
					ant.index.y += ant.direction == Ant::Up ? -1 : 1;
				else
					ant.index.x += ant.direction == Ant::Right ? -1 : 1;

				if( ant.index.x < 0 || ant.index.y < 0 || ant.index.x >= ( int )width || ant.index.y >= ( int )height )
					ants.erase( ants.begin() + i );
			}
		}
	}

	void ProcessEvent( const sf::Event& event ) final
	{
		
	}

protected:
	const float margin = 0.0f;
	const float size = 5.0f;
	const unsigned width = 100;
	const unsigned height = 50;
	float timer = 0.5f;
	std::vector< sf::Vertex > board;
	std::vector< bool > states;

	struct Ant
	{
		Ant( const sf::Vector2i& idx ) : index( idx ) { }

		enum Direction
		{
			Up,
			Right,
			Down,
			Left,
			NumDirections,
		};

		sf::Vector2i index;
		Direction direction = Right;
	};

	std::vector< Ant > ants;
};