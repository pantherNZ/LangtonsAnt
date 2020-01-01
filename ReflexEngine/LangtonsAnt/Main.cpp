// Includes
#include "../ReflexEngine/Engine.h"

using namespace Reflex::Core;

class GameState;

// Entry point
int main()
{
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

		ReadSetups();
		Reset();
	}

protected:
	void Render() final
	{
		GetWindow().draw( board.data(), board.size(), sf::Quads );

		// Draw settings / display UI
		ImGui::SetNextWindowPos( sf::Vector2( 5.0f, 500.0f ), ImGuiCond_::ImGuiCond_Once );
		ImGui::SetNextWindowSize( sf::Vector2( 400.0f, 400.0f ), ImGuiCond_::ImGuiCond_Once );
		ImGui::Begin( "Langton's Ant" );
		ImGui::Separator();
		ImGui::Text( ( "Generation " + std::to_string( generation ) ).c_str() );
		ImGui::Separator();

		// Handler for radio buttons for adjusting speed
		{
			const auto prev = speedIdx;
			ImGui::RadioButton( "Slow", &speedIdx, Slow ); ImGui::SameLine();
			ImGui::RadioButton( "Regular", &speedIdx, Regular ); ImGui::SameLine();
			ImGui::RadioButton( "Fast", &speedIdx, Fast ); ImGui::SameLine();
			ImGui::RadioButton( "Very Fast", &speedIdx, VeryFast ); ImGui::SameLine();
			ImGui::RadioButton( "Step 100k", &speedIdx, Immediate );

			if( prev != speedIdx )
			{
				updateTime = speedValues[speedIdx];

				if( speedIdx == Immediate )
					stepTo += 100000;
			}
		}

		// Manual speed adjustment with slider
		ImGui::SliderFloat( "Update Time", &updateTime, 0.0f, 0.5f );

		// Number of states for this sim
		if( ImGui::SliderInt( "State Count", &customStates[currentStateInfo].statesCount, 2, 20 ) )
		{
			setupType = Custom;
			customStates[currentStateInfo].coloursAndAngles.resize( customStates[currentStateInfo].statesCount );
		}

		ImGui::Separator();

		// Settings for reach state (the colour of the state + angle to rotate when the ant steps on this tile)
		if( ImGui::CollapsingHeader( "States Info", true ) )
		{
			bool colourChange = false;

			for( int i = 0; i < customStates[currentStateInfo].statesCount; ++i )
			{
				if( ImGui::TreeNode( ( "State " + std::to_string( i ) ).c_str() ) )
				{
					if( ImGui::ColorEdit3( "State Colour", customStates[currentStateInfo].coloursAndAngles[i].first._Elems ) )
					{
						colourChange = true;
						setupType = Custom;
					}

					if( ImGui::SliderAngle( "State Rotation", &customStates[currentStateInfo].coloursAndAngles[i].second ) )
						setupType = Custom;

					ImGui::TreePop();
				}
			}

			if( colourChange )
				for( unsigned y = 0; y < height; ++y )
					for( unsigned x = 0; x < width; ++x )
						SetTileColour( sf::Vector2i( x, y ), Reflex::ToSFColour( customStates[currentStateInfo].coloursAndAngles[states[y * width + x]].first ) );
		}

		ImGui::Separator();

		// UI combo box display for predefined ant simulation setups (read from JSON file)
		static const char* currentItem = customStates[0].name.c_str();
		if( ImGui::BeginCombo( "Predefined Setups", currentItem ) )
		{
			for( unsigned i = 0; i < customStates.size(); ++i )
			{
				const bool isSelected = currentItem && std::strcmp( currentItem, customStates[i].name.c_str() ) == 0;
				if( ImGui::Selectable( customStates[i].name.c_str(), isSelected ) )
				{
					currentItem = customStates[i].name.c_str();
					currentStateInfo = i;
					Reset();
				}
				if( isSelected )
					ImGui::SetItemDefaultFocus();
			}

			ImGui::EndCombo();
		}

		ImGui::Separator();

		static bool showSaveSetup = false;

		// Save new custom ant simulation setup (saves to JSON file)
		if( ImGui::Button( "Save Setup" ) )
			showSaveSetup = !showSaveSetup;

		if( showSaveSetup )
		{
			ImGui::BeginChild( "Save Setup" );
			static char buffer[256] = "New Sim";
			ImGui::InputText( "Name", buffer, IM_ARRAYSIZE( buffer ) );

			if( ImGui::Button( "Cancel" ) ) 
				showSaveSetup = false;

			ImGui::SameLine();
			if( ImGui::Button( "Save" ) ) 
			{
				auto copiedCustomState = customStates[currentStateInfo];
				copiedCustomState.name = buffer;
				ReadSetups();
				customStates.push_back( std::move( copiedCustomState ) );
				SaveSetups();
				currentItem = customStates.back().name.c_str();
				showSaveSetup = false;
			}

			ImGui::EndChild();
		}
		else
		{
			// Button for resetting generations & the simulation display
			ImGui::SameLine();
			if( ImGui::Button( "Reset Simulation" ) )
				Reset();
		}

		ImGui::End();
	}

	void Update( const float deltaTime ) final
	{
		if( updateTime <= 0.0f || ants.empty() )
			return;

		timer -= deltaTime;

		while( !ants.empty() && ( timer <= 0.0f || ( speedIdx == Immediate && generation < stepTo ) ) )
		{
			timer += updateTime;
			++generation;

			for( int i = ants.size() - 1; i >= 0; --i )
			{
				// Rotate
				auto& ant = ants[i];
				const auto idx = ant.index.y * width + ant.index.x;
				assert( idx >= 0 && idx < states.size() );
				states[idx] %= customStates[currentStateInfo].statesCount;
				ant.direction = Reflex::Modf( ant.direction + customStates[currentStateInfo].coloursAndAngles[states[idx]].second, PI2 );

				// Flip tile
				assert( idx >= 0 && idx * 4 + 3 < board.size() );
				states[idx] = ( states[idx] + 1 ) % customStates[currentStateInfo].statesCount;
				SetTileColour( ant.index, Reflex::ToSFColour( customStates[currentStateInfo].coloursAndAngles[states[idx]].first ) );

				// Move
				ant.index.x += Reflex::RoundToInt( std::cosf( ant.direction ) );
				ant.index.y += Reflex::RoundToInt( std::sinf( ant.direction ) );

				// Remove off screen ants
				if( ant.index.x < 0 || ant.index.y < 0 || ant.index.x >= ( int )width || ant.index.y >= ( int )height )
					ants.erase( ants.begin() + i );
			}
		}

		if( speedIdx == Immediate )
		{
			speedIdx = Fast;
			timer = 0.0f;
		}
	}

	void Reset()
	{
		std::fill( states.begin(), states.end(), 0 );
		ants.clear();
		ants.emplace_back( sf::Vector2i( width / 2, height / 2 ) );
		generation = 0;
		stepTo = 0;
		updateTime = speedValues[speedIdx];

		for( unsigned y = 0; y < height; ++y )
			for( unsigned x = 0; x < width; ++x )
				SetTileColour( sf::Vector2i( x, y ), Reflex::ToSFColour( customStates[currentStateInfo].coloursAndAngles[0].first ) );
	}

	void SetTileColour( const sf::Vector2i& index, const sf::Color& colour )
	{
		const auto idx = index.y * width + index.x;
		board[idx * 4].color = colour;
		board[idx * 4 + 1].color = colour;
		board[idx * 4 + 2].color = colour;
		board[idx * 4 + 3].color = colour;
	}

	void SaveSetups()
	{
		Json::Value jsonOut;
		Json::Value setups( Json::arrayValue );

		for( unsigned i = 2; i < customStates.size(); ++i )
		{
			Json::Value item;
			item["Name"] = customStates[i].name;
			Json::Value coloursAndAngles( Json::arrayValue );

			for( auto& info : customStates[i].coloursAndAngles )
			{
				Json::Value coloursAndAngle;
				coloursAndAngle["Angle"] = int( TODEGREES( info.second ) );
				Json::Value vec( Json::arrayValue );
				const auto colour = Reflex::ToSFColour( info.first );
				vec.append( Json::Value( ( int )colour.r ) );
				vec.append( Json::Value( ( int )colour.g ) );
				vec.append( Json::Value( ( int )colour.b ) );
				vec.append( Json::Value( ( int )colour.a ) );
				coloursAndAngle["Colour"] = vec;
				coloursAndAngles.append( coloursAndAngle );
			}

			item["ColoursAndAngles"] = coloursAndAngles;
			setups.append( item );
		}

		jsonOut["Setups"] = setups;
		
		Json::StreamWriterBuilder builder;
		builder["commentStyle"] = "None";
		builder["indentation"] = "   ";

		std::ofstream outputFileStream( "SimulationData.json" );
		builder.newStreamWriter()->write( jsonOut, &outputFileStream );
	}

	void ReadSetups()
	{
		customStates.clear();

		StateInfo custom( "Custom" );
		custom.Append( sf::Color::Black, PI / 2.0f );
		custom.Append( sf::Color::White, -PI / 2.0f );
		customStates.push_back( custom );

		StateInfo rlr( "RLR" );
		rlr.Append( sf::Color::Black, PI / 2.0f );
		rlr.Append( sf::Color::Magenta, -PI / 2.0f );
		rlr.Append( sf::Color::Green, PI / 2.0f );
		customStates.push_back( std::move( rlr ) );

		std::ifstream input( "SimulationData.json" );

		if( !input.fail() )
		{
			Json::CharReaderBuilder reader;
			Json::Value obj;
			std::string errs;

			if( !Json::parseFromStream( reader, input, &obj, &errs ) )
			{
				throw std::runtime_error( "SimulationData.json is malformed and cannot be read!" );
				return;
			}

			const auto& setups = obj["Setups"];

			for( unsigned i = 0; i < setups.size(); ++i )
			{
				const auto& setup = setups[i];
				StateInfo newInfo( setup["Name"].asString() );
				const auto& coloursAndAngles = setup["ColoursAndAngles"];

				for( unsigned j = 0; j < coloursAndAngles.size(); ++j )
				{
					const auto& colourAndAngle = coloursAndAngles[j];
					const auto& colour = colourAndAngle["Colour"];
					sf::Color c( colour[0].asInt(), colour[1].asInt(), colour[2].asInt(), colour[3].asInt() );
					newInfo.Append( c, TORADIANS( colourAndAngle["Angle"].asInt() ) );
				}

				customStates.push_back( std::move( newInfo ) );
			}
		}
	}

protected:
	const float margin = 0.0f;
	const float size = 5.0f;
	const unsigned width = 100;
	const unsigned height = 50;
	float timer = 0.5f;
	float updateTime = 0.1f;
	unsigned generation = 0;
	unsigned stepTo = 0;

	enum SpeedValue
	{
		Immediate,
		VeryFast,
		Fast,
		Regular,
		Slow,
		NumSpeeds
	};

	enum Setup
	{
		Custom,
		RLR, 
		LLRR,
		LRRRRRRLLR,
		LLRRRLRLRLLR,
		NumSetups
	};

	int setupType = Custom;
	int speedIdx = Fast;
	std::array< float, NumSpeeds > speedValues = { 0.1f, 0.001f, 0.01f, 0.1f, 0.3f };

	std::vector< sf::Vertex > board;
	std::vector< int > states;

	struct StateInfo
	{
		StateInfo( const std::string& name ) : name( name ) { }
		void Append( const sf::Color& colour, const float angleRadians ) { coloursAndAngles.push_back( std::pair( Reflex::ToImGuiColour3( colour ), angleRadians ) ); ++statesCount; }

		std::string name;
		int statesCount = 0;
		std::vector< std::pair< std::array< float, 3 >, float > > coloursAndAngles;
	};

	unsigned currentStateInfo = 0;
	std::vector< StateInfo > customStates;

	struct Ant
	{
		Ant( const sf::Vector2i& idx, const float startAng = PI ) : index( idx ), direction( startAng ) { }
		sf::Vector2i index;
		float direction = PI;
	};

	std::vector< Ant > ants;
};