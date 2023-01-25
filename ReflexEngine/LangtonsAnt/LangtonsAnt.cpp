// Includes
#include "LangtonsAnt.h"
#include "../ReflexEngine/CameraComponent.h"

// Entry point
int main()
{
	Engine engine( "Langton's Ant" );
	engine.RegisterState< GameState >( true );
	engine.Run();

	return 0;
}

GameState::GameState( StateManager& stateManager, Context context )
	: State( stateManager, context )
	, antPlacerDisplay( 10.0f )
{
	camera = GetWorld().CreateObject()->AddComponent< Reflex::Components::Camera >( Reflex::Vector2uToVector2f( context.window.getSize() ) / 2.0f, Reflex::Vector2uToVector2f( context.window.getSize() ), true );
	camera->EnableWASDPanning( sf::Vector2f( 300.0f, 300.0f ) );
	camera->EnableArrowKeyPanning();
	camera->EnableMouseZooming( 1.2f, true );

	antPlacerDisplay.setFillColor( sf::Color(255, 0, 0, 128 ) );
	antPlacerDisplay2.setFillColor( sf::Color(255, 0, 0, 128 ) );
	Reflex::CenterOrigin( antPlacerDisplay );

	ants.emplace_back( sf::Vector2f( context.window.getSize().x / 2.0f, context.window.getSize().y / 2.0f ), PI, sf::Color::White );

	ReadCustomSetups();
	SetupGrid();
	Reset( false );

	//processThread = std::make_unique<sf::Thread>( &GameState::UpdateAntsThread, this );
	//processThread->launch();
}

void GameState::SetupGrid()
{
	gridStates.clear();
	gridVertices.clear();

	tileSize.x = size;
	tileSize.y = size;

	if( GetInfo().gridTypeIdx == Pixel )
		tileSize = sf::Vector2f( 1.0f, 1.0f );
	else if( GetInfo().gridTypeIdx == Hexagon )
		tileSize.x = sqrtf( 3.0f ) * size / 2.0f;

	auto startColour = Reflex::ToColour( GetInfo().coloursAndAngles[0].first );

	const auto heightGap = tileSize.y * ( GetInfo().gridTypeIdx == Hexagon ? 0.75f : 1.0f );
	gridSize.x = ( unsigned )ceil( GetWindow().getSize().x / tileSize.x );
	gridSize.y = ( unsigned )ceil( GetWindow().getSize().y / heightGap );
	gridStates.resize( gridSize.x * gridSize.y );
	gridOrigin = sf::Vector2f( GetWindow().getSize().x / 2.0f - gridSize.x / 2.0f * tileSize.x, GetWindow().getSize().y / 2.0f - gridSize.y / 2.0f * heightGap );

	if( GetInfo().gridTypeIdx == Square || GetInfo().gridTypeIdx == Pixel )
	{
		const auto usePoints = ( tileSize.x <= 1.0f );
		gridVertices.reserve( gridSize.x * gridSize.y * ( usePoints ? 1 : 4 ) );
		const auto dimensions = tileSize / 2.0f;

		for( unsigned y = 0; y < gridSize.y; ++y )
		{
			for( unsigned x = 0; x < gridSize.x; ++x )
			{
				//const auto r = Reflex::RandomInt( 255 );
				//startColour = sf::Color( r, r, r, 255 );
				const auto pos = gridOrigin + sf::Vector2f( x * tileSize.x, y * heightGap );

				if( usePoints )
				{
					gridVertices.emplace_back( pos, startColour );
				}
				else
				{
					gridVertices.emplace_back( pos + sf::Vector2f( -dimensions.x, -dimensions.y ), startColour );
					gridVertices.emplace_back( pos + sf::Vector2f( -dimensions.x, dimensions.y ), startColour );
					gridVertices.emplace_back( pos + sf::Vector2f( dimensions.x, dimensions.y ), startColour );
					gridVertices.emplace_back( pos + sf::Vector2f( dimensions.x, -dimensions.y ), startColour );
				}
			}
		}
	}
	else if( GetInfo().gridTypeIdx == Hexagon )
	{
		gridVertices.reserve( gridSize.x * gridSize.y * 18 );
		const auto dimensions = sf::Vector2f( tileSize.x / 2.0f, tileSize.y / 4.0f );

		for( unsigned y = 0; y < gridSize.y; ++y )
		{
			for( unsigned x = 0; x < gridSize.x; ++x )
			{
				//const auto r = Reflex::RandomInt( 255 );
				//startColour = sf::Color( r, r, r, 120 );
				const auto pos = gridOrigin + sf::Vector2f( x * tileSize.x + ( y & 1 ? tileSize.x / 2.0f : 0.0f ), y * heightGap );
				const auto m = sf::Vertex( pos, startColour );
				const auto a = sf::Vertex( pos + sf::Vector2f( 0.0f, tileSize.y / 2.0f ), startColour );
				const auto b = sf::Vertex( pos + sf::Vector2f( dimensions.x, dimensions.y ) , startColour );
				const auto c = sf::Vertex( pos + sf::Vector2f( dimensions.x, -dimensions.y ), startColour );
				const auto d = sf::Vertex( pos + sf::Vector2f( 0.0f, -tileSize.y / 2.0f ), startColour );
				const auto e = sf::Vertex( pos + sf::Vector2f( -dimensions.x, -dimensions.y ), startColour );
				const auto f = sf::Vertex( pos + sf::Vector2f( -dimensions.x, dimensions.y ), startColour );
				const auto newHexagon = { m, a, b, m, b, c, m, c, d, m, d, e, m, e, f, m, f, a };
				gridVertices.insert( gridVertices.end(), newHexagon );
			}
		}
	}
}

void GameState::Reset( const bool resetGridCcolours )
{
	std::fill( gridStates.begin(), gridStates.end(), 0 );
	
	for( auto& ant : ants )
	{
		ant.active = true;
		ant.currentPos = ant.startingPos;
		ant.currentDir = ant.startingDir;
	}
	
	activeAnts = ( unsigned )ants.size();
	generation = 0;
	placingAnts = false;

	if( resetGridCcolours )
		for( unsigned y = 0; y < gridSize.y; ++y )
			for( unsigned x = 0; x < gridSize.x; ++x )
				SetTileColour( sf::Vector2i( x, y ), Reflex::ToColour( GetInfo().coloursAndAngles[0].first ) );

	timer = 0.0f;
}

void GameState::Update( const float deltaTime )
{
	UpdateAnts( deltaTime );
}

void GameState::UpdateAntsThread()
{
	sf::Clock clock;
	sf::Time accumlatedTime = sf::Time::Zero;

	while( true )
	{
		sf::Time deltaTime = clock.restart();
		sf::Clock processTime;

		UpdateAnts( deltaTime.asSeconds() );

		{
			//sf::Mutex mutex;
			//sf::Lock lock( mutex );
			gridVerticesRender = gridVertices;
		}

		const float t = 1.0f / updatesPerSec;
		const float diff = processTime.getElapsedTime().asSeconds() - t;
		if( diff >= 0.0f )
			sf::sleep( sf::seconds( diff ) );
	}
}

void GameState::UpdateAnts( const float deltaTime )
{
	if( updatesPerSec <= 0.0f || activeAnts == 0 )
		return;

	timer -= deltaTime;

	while( activeAnts > 0 && ( timer <= 0.0f || ( int )generation < stepTo ) )
	{
		if( timer <= 0.0f )
			timer +=  1.0f / updatesPerSec;

		++generation;
		activeAnts = 0;

		for( auto& ant : ants )
		{
			if( !ant.active )
				continue;

			auto gridIndex = FindGridIndex( ant.currentPos );

			// Remove off screen ants
			if( gridIndex.x <= 0 || gridIndex.y <= 0 || gridIndex.x >= ( int )gridSize.x || gridIndex.y >= ( int )gridSize.y )
			{
				if( GetInfo().infiniteGrid )
				{
					if( gridIndex.x <= 0 ) ant.currentPos.x = ( gridSize.x - 1 ) * tileSize.x;
					if( gridIndex.y <= 0 ) ant.currentPos.y = ( gridSize.y - 1 ) * tileSize.y;
					if( gridIndex.x >= ( int )gridSize.x ) ant.currentPos.x = tileSize.x;
					if( gridIndex.y >= ( int )gridSize.y ) ant.currentPos.y = tileSize.y;
					gridIndex = FindGridIndex( ant.currentPos );
				}
				else
				{
					ant.active = false;
					continue;
				}
			}

			++activeAnts;

			// Rotate
			const auto idx = gridIndex.y * gridSize.x + gridIndex.x;
			assert( idx >= 0 && idx < gridStates.size() );
			gridStates[idx] %= GetInfo().statesCount;
			ant.currentDir = Reflex::Modf( ant.currentDir + GetInfo().coloursAndAngles[gridStates[idx]].second, PI2 );

			if( logToConsole )
				LOG_INFO( "Generation: " << generation << ", Ant: " << activeAnts << ", gridIndex: " << gridIndex.x << ", " <<
				gridIndex.y << ", Angle: " << ant.currentDir << ", Tile State: " << gridStates[idx] << ", Pos: " << ant.currentPos.x << ", " << ant.currentPos.y );

			// Flip tile
			const auto count = ( idx + 1 ) * ( tileSize.x <= 1.0f ? 1 : GetInfo().gridTypeIdx == Hexagon ? 18 : 4 );
			assert( idx >= 0 && count - 1 < gridVertices.size() );
			gridStates[idx] = ( gridStates[idx] + 1 ) % GetInfo().statesCount;

			auto newColour = Reflex::ToColour( GetInfo().coloursAndAngles[gridStates[idx]].first );
			
			if( ant.hue != sf::Color::White )
				newColour = Reflex::BlendColourGammaCorrection( newColour, ant.hue );

			SetTileColour( gridIndex, newColour, ( BlendValue )GetInfo().blendIdx );

			// Move
			ant.currentPos.x += std::cosf( ant.currentDir ) * tileSize.x;
			ant.currentPos.y += std::sinf( ant.currentDir ) * tileSize.y * ( GetInfo().gridTypeIdx == Hexagon ? 0.75f : 1.0f );;

			if( GetInfo().lockToGrid )
			{
				ant.currentPos.x = Reflex::Round( ant.currentPos.x );
				ant.currentPos.y = Reflex::Round( ant.currentPos.y );
			}
		}
	}
}

sf::Vector2i GameState::FindGridIndex( const sf::Vector2f& pos )
{
	if( GetInfo().gridTypeIdx == Square || GetInfo().gridTypeIdx == Pixel )
		return sf::Vector2i( int( ( pos.x - gridOrigin.x ) / tileSize.x + 0.5f ), int( ( pos.y - gridOrigin.y ) / tileSize.y + 0.5f ) );

	// Hexagon
	const auto halfSizeX = tileSize.x / 2.0f;
	const auto offset = ( sqrtf( 3.0f ) * halfSizeX ) / 2.0f;
	const auto heightGap = tileSize.y * ( GetInfo().gridTypeIdx == Hexagon ? 0.75f : 1.0f );
	const auto x = pos.x - gridOrigin.x;
	const auto y = pos.y - gridOrigin.y - ( size - heightGap ) + offset;

	auto row = Reflex::RoundToInt( y / heightGap );
	const auto rowIsOdd = row & 1;
	auto column = Reflex::RoundToInt( ( rowIsOdd ? x - halfSizeX : x ) / tileSize.x );

	const auto c = halfSizeX / 2.0f;
	const auto relY = y - row * ( heightGap - 1 ) + c;
	const auto relX = Reflex::Modf( rowIsOdd ? x : x + halfSizeX, tileSize.x );

	const auto m = c / halfSizeX;
	if( relX < halfSizeX && relY < ( -m * relX ) + c )
	{
		row--;
		if( !rowIsOdd )
			column--;
	}
	else if( relX >= halfSizeX && relY < ( m * relX ) - c )
	{
		row--;
		if( rowIsOdd )
			column++;
	}

	return sf::Vector2i( column, row );
}

void GameState::ProcessEvent( const sf::Event& event )
{
	if( ( angleAnt || placingAnts ) && event.type == sf::Event::MouseButtonPressed )
	{
		if( event.mouseButton.button == sf::Mouse::Button::Right )
		{
			if( angleAnt )
				angleAnt = false;
			else
				placingAnts = false;
		}
		else if( event.mouseButton.button == sf::Mouse::Button::Left )
		{
			if( angleAnt )
			{
				const auto mousePosition = GetMousePosition();
				const auto rotation = Reflex::RotationFromVector( mousePosition - antPlacerDisplay.getPosition() );
				ants.emplace_back( LockToGrid( antPlacerDisplay.getPosition() ), rotation );
				activeAnts++;
				angleAnt = false;
			}
			else
			{
				angleAnt = true;
			}
		}
	}

	if( event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Space )
	{
		if( pauseUpdatesPerSec == 0.0f )
		{
			pauseUpdatesPerSec = updatesPerSec;
			updatesPerSec = 0.0f;
		}
		else
		{
			updatesPerSec = pauseUpdatesPerSec;
			pauseUpdatesPerSec = 0.0f;
		}
	}
}

void GameState::Render()
{
	//GetWindow().draw( gridVerticesRender.data(), gridVerticesRender.size(), GetInfo().gridTypeIdx == Square ? sf::Quads : ( tileSize.x <= 1.0f ? sf::Points : sf::Triangles ) );
	GetWindow().draw( gridVertices.data(), gridVertices.size(), GetInfo().gridTypeIdx == Square ? sf::Quads : ( tileSize.x <= 1.0f ? sf::Points : sf::Triangles ) );
	const auto mousePos = GetMousePosition();

	if( placingAnts )
	{
		if( !angleAnt )
		{
			antPlacerDisplay.setPosition( mousePos );
		}
		else
		{
			antPlacerDisplay2.setPosition( antPlacerDisplay.getPosition() );
			antPlacerDisplay2.setSize( sf::Vector2f( std::min( 150.0f, Reflex::GetDistance( antPlacerDisplay.getPosition(), mousePos ) ), 2.0f ) );
			antPlacerDisplay2.setRotation( TODEGREES( Reflex::RotationFromVector( mousePos - antPlacerDisplay.getPosition() ) ) );
			GetWindow().draw( antPlacerDisplay2 );
		}

		GetWindow().draw( antPlacerDisplay );
	}

	if( displayAnts || placingAnts )
	{
		for( auto& ant : ants )
		{
			ant.display.setPosition( ant.currentPos );
			GetWindow().draw( ant.display );
		}
	}

	//static sf::Vector2i highlight( -999, 0 );
	//const auto newHighlight = FindGridIndex( mousePos );
	//
	//if( newHighlight != highlight && newHighlight.x >= 0 && newHighlight.y >= 0 && newHighlight.x < gridSize.x && newHighlight.y < gridSize.y )
	//{
	//	static sf::Color oldColour;
	//
	//	if( highlight.x != -999 )
	//		SetTileColour( highlight, oldColour );
	//
	//	highlight = newHighlight;
	//	const auto idx = ( highlight.y * gridSize.x + highlight.x ) * 18;
	//	oldColour = gridVertices[idx].color;
	//
	//	SetTileColour( newHighlight, sf::Color::Magenta );
	//}

	// Draw settings / display UI
	ImGui::SetNextWindowPos( sf::Vector2( 5.0f, 0.17f * GetWindow().getSize().y ), ImGuiCond_::ImGuiCond_Once );
	ImGui::Begin( "Langton's Ant", nullptr, ImGuiWindowFlags_AlwaysAutoResize );
	ImGui::Text( ( "Generation " + std::to_string( generation ) ).c_str() );
	ImGui::Separator();
	ImGui::Checkbox( "Console Logging", &logToConsole );

	ImGui::NewLine();

	{
		const auto prev = GetInfo().gridTypeIdx;
		ImGui::Text( "Grid Type" );
		ImGui::RadioButton( "Square", &GetInfo().gridTypeIdx, Square ); ImGui::SameLine();
		ImGui::RadioButton( "Hexagon", &GetInfo().gridTypeIdx, Hexagon ); ImGui::SameLine();
		ImGui::RadioButton( "Pixel", &GetInfo().gridTypeIdx, Pixel );

		if( ImGui::InputFloat( "Grid Size", &size, 1.0f, 5.0f, 1 ) || prev != GetInfo().gridTypeIdx )
		{
			SetupGrid();
			Reset();
		}
	}

	// Manual speed adjustment with slider
	ImGui::SliderFloat( "Updates per second", &updatesPerSec, 0, 100000, "%.0f", 2.0f );

	const auto g = floorf( log10f( ( float )stepTo ) );
	const auto baseRate = std::max( 100, ( int )powf( 10.0f, g ) );
	ImGui::InputInt( "Set Generation", &stepTo,
		stepTo == baseRate ? baseRate / 10 : baseRate,	// Decrement
		baseRate, 0,									// Decrement fast
		baseRate,										// Increment
		baseRate  * 10 );								// Increment fast
	stepTo = std::max( 0, stepTo );

	// Number of states for this sim
	if( ImGui::SliderInt( "State Count", &GetInfo().statesCount, 2, 50 ) )
	{
		while( customStates[currentStateInfo].coloursAndAngles.size() < (unsigned )GetInfo().statesCount )
		{
			 GetInfo().Append( Reflex::RandomColour(), Reflex::RandomBool() ? -PIDIV2 : PIDIV2 );
			 GetInfo().statesCount--;
		}

		GetInfo().coloursAndAngles.resize( GetInfo().statesCount );
	}

	ImGui::NewLine();

	// Settings for reach state (the colour of the state + angle to rotate when the ant steps on this tile)
	if( ImGui::CollapsingHeader( "States Data", true ) )
	{
		bool colourChange = false;

		for( int i = 0; i < GetInfo().statesCount; ++i )
		{
			if( ImGui::TreeNode( ( "State " + std::to_string( i ) ).c_str() ) )
			{
				if( ImGui::ColorEdit4( "State Colour", GetInfo().coloursAndAngles[i].first._Elems ) )
				{
					colourChange = true;
					UpdateIncrementalColours( i );
				}

				ImGui::SliderAngle( "State Rotation", &GetInfo().coloursAndAngles[i].second, -180.0f, 180.0f );
				ImGui::TreePop();
			}
		}

		if( colourChange )
			Recolour();
	}

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
				ants.clear();
				for( auto& ant : GetInfo().ants )
					ants.emplace_back( ant.startingPos, ant.startingDir );
				SetupGrid();
				Reset();
			}

			if( isSelected )
				ImGui::SetItemDefaultFocus();
		}

		ImGui::EndCombo();
	}

	ImGui::NewLine();

	static const char* btnText = "Place Ants";
	if( ImGui::Button( btnText ) )
	{
		placingAnts = !placingAnts;
		btnText = placingAnts ? "Placing Ants" : "Place Ants";
	}

	ImGui::SameLine();
	if( ImGui::Button( "Remove Ants" ) )
		ants.clear();

	ImGui::Checkbox( "Display Ants", &displayAnts );

	ImGui::NewLine();

	ImGui::Text( "Colour Blending" );
	ImGui::RadioButton( "Disabled", &GetInfo().blendIdx, Disable ); ImGui::SameLine();
	ImGui::RadioButton( "Gamma", &GetInfo().blendIdx, Gamma ); ImGui::SameLine();
	ImGui::RadioButton( "Alpha", &GetInfo().blendIdx, Alpha );

	if( ImGui::Checkbox( "Incremental RGB", &GetInfo().incrementalRGB ) )
		UpdateIncrementalColours();

	ImGui::SameLine();
	if( ImGui::Checkbox( "Incremental Alpha", &GetInfo().incrementalAlpha ) )
		UpdateIncrementalColours();

	ImGui::Checkbox( "Wrap Grid", &GetInfo().infiniteGrid );
	ImGui::SameLine();
	ImGui::Checkbox( "Lock To Grid", &GetInfo().lockToGrid );

	//if( ImGui::Button( "Smooth Colours" ) )
	//	ColourSmooth();

	ImGui::NewLine();
	if( ImGui::Button( "Random Angles" ) )
		RandomiseAngles();

	ImGui::SameLine();
	if( ImGui::Button( "Random Angles Aligned" ) )
		RandomiseAngles( true );

	if( ImGui::Button( "Random Colours" ) )
	{
		for( auto& info : GetInfo().coloursAndAngles )
			info.first = Reflex::ToImGuiColour4( Reflex::RandomColour( GetInfo().incrementalAlpha ) );

		if( GetInfo().incrementalRGB )
		{
			GetInfo().coloursAndAngles[1].first = Reflex::ToImGuiColour4( Reflex::RandomColour( GetInfo().incrementalAlpha, 0, 50 ) );
			Reflex::RandomElement( GetInfo().coloursAndAngles[1].first ) = Reflex::RandomInt( Reflex::RandomInt( 200 ) ) / 255.0f;
		}

		GetInfo().coloursAndAngles[0].first = Reflex::ToImGuiColour4( sf::Color::Black );
		UpdateIncrementalColours( 0 );
	}

	ImGui::SameLine();
	if( ImGui::Button( "Random Ant Colours" ) )
		for( auto& ant : ants )
			ant.hue = Reflex::RandomColour( false, 150, 255 );

	ImGui::NewLine();

	// Button for resetting generations & the simulation display
	if( ImGui::Button( "Restart" ) )
		Reset();

	ImGui::SameLine();
	if( ImGui::Button( "Random Simulation" ) )
		RandomiseParameters();


	ImGui::NewLine();

	/// Save new custom ant simulation setup (saves to JSON file)
	static char buffer[256];
	sprintf_s( buffer, IM_ARRAYSIZE( buffer ), GetInfo().name.c_str() );
	if( ImGui::InputText( "Name", buffer, IM_ARRAYSIZE( buffer ) ) )
		GetInfo().name = buffer;

	if( ImGui::Button( "Save New" ) )
	{
		auto copiedCustomState = customStates[currentStateInfo];
		copiedCustomState.name = buffer;

		copiedCustomState.ants.clear();
		for( auto& ant : ants )
			copiedCustomState.ants.emplace_back( ant.startingPos, ant.startingDir );

		ReadCustomSetups();
		customStates.push_back( std::move( copiedCustomState ) );
		SaveCustomSetups();
		currentItem = customStates.back().name.c_str();
		currentStateInfo = ( unsigned )customStates.size() - 1;
	}

	ImGui::SameLine();
	if( ImGui::Button( "Save Existing" ) )
	{
		auto copiedCustomState = customStates[currentStateInfo];

		copiedCustomState.ants.clear();
		for( auto& ant : ants )
			copiedCustomState.ants.emplace_back( ant.startingPos, ant.startingDir );

		SaveCustomSetups();
		currentItem = customStates[0].name.c_str();
	}

	ImGui::NewLine();
	static std::string key;

	ImGui::InputText( "Key", &key );
	if( ImGui::Button( "Generate Key" ) )
		key = GenerateSharingKey();

	ImGui::SameLine();
	if( ImGui::Button( "Load Key" ) )
	{
		try
		{
			LoadFromSharingKey( key );
		}
		catch( std::exception e )
		{
			LOG_WARN( "Failed to load key" );
		}
	}

	ImGui::End();
}

void GameState::UpdateIncrementalColours( const unsigned startIdx )
{
	if( !GetInfo().incrementalAlpha && !GetInfo().incrementalRGB )
		return;

	const auto colours = GetInfo().coloursAndAngles;
	sf::Vector3f baseC( colours[startIdx].first[0], colours[startIdx].first[1], colours[startIdx].first[2] );
	sf::Vector3f newC = baseC;
	const auto startAlpha = 0.1f;
	float alpha = startAlpha;

	for( unsigned i = startIdx; i < GetInfo().coloursAndAngles.size(); ++i )
	{
		if( GetInfo().incrementalRGB )
		{
			 GetInfo().coloursAndAngles[i].first[0] = std::min( 1.0f, newC.x );
			 GetInfo().coloursAndAngles[i].first[1] = std::min( 1.0f, newC.y );
			 GetInfo().coloursAndAngles[i].first[2] = std::min( 1.0f, newC.z );
			newC += ( sf::Vector3f( 1.0f, 1.0f, 1.0f ) - baseC ) / ( float )GetInfo().statesCount;
		}
		else if( GetInfo().incrementalAlpha )
		{
			GetInfo().coloursAndAngles[i].first[3] = std::min( 1.0f, alpha );
			alpha += ( 1.0f - startAlpha ) / ( float )GetInfo().statesCount;
		}
	}

	Recolour();
}

void GameState::ColourSmooth()
{

}

void GameState::Recolour()
{
	for( unsigned y = 0; y < gridSize.y; ++y )
		for( unsigned x = 0; x < gridSize.x; ++x )
			SetTileColour( sf::Vector2i( x, y ), Reflex::ToColour( GetInfo().coloursAndAngles[gridStates[y * gridSize.x + x] % GetInfo().statesCount].first ) );
}

sf::Vector2f GameState::LockToGrid( const sf::Vector2f& v )
{
	return sf::Vector2f( Reflex::Round( v.x / tileSize.x ) * tileSize.x, Reflex::Round( v.y / tileSize.y ) * tileSize.y );
}

void GameState::RandomiseParameters()
{
	ants.clear();

	const auto centre = sf::Vector2f( GetWindow().getSize().x / 2.0f, GetWindow().getSize().y / 2.0f );
	const auto antMode = Reflex::RandomInt( 5 );
	if( antMode == 0 )
	{
		ants.emplace_back( centre, Reflex::RandomAngle() - PI );
	}
	else
	{
		const auto distMaxX = Reflex::RandomFloat( 4.0f, GetWindow().getSize().x / Reflex::RandomFloat( 2.0f, 5.0f ) ) / 2.0f;
		const auto distMaxY = Reflex::RandomFloat( 4.0f, GetWindow().getSize().y / Reflex::RandomFloat( 2.0f, 5.0f ) ) / 2.0f;
		const auto minX = Reflex::RandomBool() ? 0.0f : Reflex::RandomFloat( 0.0f, distMaxX / Reflex::RandomFloat( 1.1f, 2.0f ) );
		const auto minY = Reflex::RandomBool() ? 0.0f : Reflex::RandomFloat( 0.0f, distMaxY / Reflex::RandomFloat( 1.1f, 2.0f ) );
		const auto count = 1 + ( antMode == 2 || antMode == 3 ? Reflex::RandomInt( 4 ) : ( Reflex::RandomInt( 4 ) == 0 ? 0 : Reflex::RandomInt( 20 ) ) );
		const auto randomCircleDivs = Reflex::RandomInt( 1, 6 );

		for( int i = 0; i < count; ++i )
		{
			const auto randomX = Reflex::RandomFloat( -distMaxX, distMaxX );
			const auto randomY = Reflex::RandomFloat( -distMaxY, distMaxY );
			const auto offset = sf::Vector2f( randomX + Reflex::Sign( randomX ) * minX, randomY + Reflex::Sign( randomY ) * minY );
			const auto pos = LockToGrid( centre + offset );
			const auto angle = Reflex::RandomAngle() - PI;
			ants.emplace_back( pos, angle );

			// Y symmetry 
			if( antMode == 2 )
				ants.emplace_back( LockToGrid( sf::Vector2f( GetWindow().getSize().x / 2.0f - ( pos.x - GetWindow().getSize().x / 2.0f ), pos.y ) ), angle - PI );
			// X symmetry
			else if( antMode == 3 )
				ants.emplace_back( LockToGrid( sf::Vector2f( pos.x, GetWindow().getSize().y / 2.0f - ( pos.y - GetWindow().getSize().y / 2.0f ) ) ), angle - PI );
			// Arbitrary symmetry
			else if( antMode == 4 )
			{
				for( int j = 0; j < randomCircleDivs; ++j )
				{
					const auto rotation = ( j + 1.0f ) * PI2 / ( randomCircleDivs + 1.0f );
					ants.emplace_back( LockToGrid( centre + Reflex::RotateVector( offset, TODEGREES( rotation ) ) ), angle + rotation );
				}
			}
		}

		if( Reflex::RandomBool() )
		{
			std::vector< sf::Color > colours;
			colours.resize( std::max( 2U, ( unsigned )std::sqrt( Reflex::RandomInt( ( int )ants.size() * ( int )ants.size() ) ) ) );

			if( Reflex::RandomBool() )
			{
				for( auto& c : colours )
					c = Reflex::RandomColour( false, 0, 255 );
			}
			else
			{
				for( unsigned i = 0; i < colours.size(); ++i )
				{
					if( i == 0 )
						colours[i] = Reflex::RandomColour( false, 75, 180 );
					else
					{
						colours[i].r = Reflex::Clamp( colours[i - 1].r + Reflex::RandomInt( 20 ), 0, 255 );
						colours[i].g = Reflex::Clamp( colours[i - 1].g + Reflex::RandomInt( 20 ), 0, 255 );
						colours[i].b = Reflex::Clamp( colours[i - 1].b + Reflex::RandomInt( 20 ), 0, 255 );
					}
				}
			}

			for( auto& ant : ants )
				ant.hue = Reflex::RandomElement( colours );
		}
	}

	const auto statesCount = 2 + ( Reflex::RandomBool() ? Reflex::RandomInt( 50 ) : Reflex::RandomInt( 500 ) );
	GetInfo().statesCount = 0;
	GetInfo().coloursAndAngles.clear();
	GetInfo().Append( sf::Color::Black, Reflex::RandomAngle() - PI );

	GetInfo().incrementalAlpha = statesCount > 4 && Reflex::RandomInt( 5 ) != 0;
	GetInfo().incrementalRGB = statesCount > 2 && Reflex::RandomBool();
	GetInfo().blendIdx = Reflex::RandomInt( NumBlendTypes );
	GetInfo().lockToGrid = Reflex::RandomInt( 4 ) == 0;
	GetInfo().infiniteGrid = Reflex::RandomBool();

	if( GetInfo().incrementalRGB )
	{
		GetInfo().Append( Reflex::RandomColour( GetInfo().incrementalAlpha, 0, Reflex::RandomInt( 1, 50 ) ), Reflex::RandomAngle() - PI );
		Reflex::RandomElement( GetInfo().coloursAndAngles[1].first ) = Reflex::RandomInt( 1 + Reflex::RandomInt( 200 ) ) / 255.0f;
	}

	const auto mode = Reflex::RandomInt( 6 );
	const auto colourMode = Reflex::RandomInt( 2 );
	auto previousAngle = Reflex::RandomAngle() - PI;
	auto previousColour = Reflex::RandomColour( GetInfo().blendIdx == Alpha );
	const auto randomColourIncrement = Reflex::RandomInt( 5, 50 );

	while( GetInfo().statesCount < statesCount )
	{
		auto angle = mode == 0 ? Reflex::RandomBool() : ( mode == 1 && Reflex::RandomInt( 5 ) == 0 ) ? previousAngle : Reflex::RandomAngle() - PI;
		auto colour = Reflex::RandomColour( GetInfo().blendIdx == Alpha );
		
		if( colourMode == 0 )
			colour = sf::Color( 
				Reflex::Clamp( ( int )previousColour.r + Reflex::RandomInt( 0, randomColourIncrement ) - randomColourIncrement / 2, 0, 255 ), 
				Reflex::Clamp( ( int )previousColour.g + Reflex::RandomInt( 0, randomColourIncrement ) - randomColourIncrement / 2, 0, 255 ),
				Reflex::Clamp( ( int )previousColour.b + Reflex::RandomInt( 0, randomColourIncrement ) - randomColourIncrement / 2, 0, 255 ), previousColour.a );

		if( mode == 3 )
			angle -= Reflex::Modf( angle, PI / Reflex::RandomInt( 1, 10 ) );

		GetInfo().Append( colour, angle );
		previousAngle = angle;
		previousColour = colour;
	}

	if( Reflex::RandomInt( 5 ) == 0 )
		RandomiseAngles( true );

	if( GetInfo().incrementalAlpha || GetInfo().incrementalRGB )
	{
		UpdateIncrementalColours();

		if( Reflex::RandomBool() && statesCount > 6 )
		{
			const auto count = 2 + Reflex::RandomInt( statesCount / Reflex::RandomInt( 3, statesCount / 2 ) );

			for( int i = 0; i < count; ++i )
			{
				const auto idx = 1 + ( statesCount / count ) * i;
				GetInfo().coloursAndAngles[idx].first = Reflex::ToImGuiColour4( Reflex::RandomColour( GetInfo().blendIdx == Alpha, 0, Reflex::RandomInt( 10, 50 ) ) );
				UpdateIncrementalColours( idx );
			}
		}
	}

	Reset();
}

void GameState::RandomiseAngles( const bool gridAligned )
{
	for( auto& state : GetInfo().coloursAndAngles )
		state.second = ( !gridAligned ? Reflex::RandomAngle() : ( GetInfo().gridTypeIdx == Hexagon ? ( PI2 / 6.0f ) * Reflex::RandomInt( 6 ) : ( PI2 / 4.0f ) * Reflex::RandomInt( 3 ) ) ) - PI;
}

std::string GameState::GenerateSharingKey() const
{
	std::stringstream ss;

	ss << GetInfo().name << " ";
	ss << GetInfo().blendIdx << " ";
	ss << GetInfo().gridTypeIdx << " ";
	ss << GetInfo().incrementalAlpha << " ";
	ss << GetInfo().incrementalRGB << " ";
	ss << GetInfo().infiniteGrid << " ";
	ss << GetInfo().coloursAndAngles.size() << " ";
	ss << ants.size() << " ";

	for( auto& info : GetInfo().coloursAndAngles )
	{
		ss << TODEGREES( info.second ) << " ";
		const auto colour = Reflex::ToColour( info.first );
		ss << colour.toInteger() << " ";
	}

	for( auto& info : ants )
	{
		ss << info.startingDir << " ";
		ss << info.startingPos.x / ( float )GetWindow().getSize().x << " ";
		ss << info.startingPos.y / ( float )GetWindow().getSize().y << " ";

		if( info.hue == sf::Color::White )
			ss << "W" << " ";
		else
			ss << info.hue.toInteger() << " ";
	}

	return ss.str();
}

void GameState::LoadFromSharingKey( const std::string& key )
{
	const auto split = Reflex::Split( key, ' ' );
	std::stringstream ss( key );

	ss >> GetInfo().name;
	ss >> GetInfo().blendIdx;
	ss >> GetInfo().gridTypeIdx;
	ss >> GetInfo().incrementalAlpha;
	ss >> GetInfo().incrementalRGB;
	ss >> GetInfo().infiniteGrid;
	int coloursCount, antsCount;
	ss >> coloursCount >> antsCount;
	GetInfo().ants.clear();
	GetInfo().statesCount = 0;
	GetInfo().coloursAndAngles.clear();
	
	for( int i = 0; i < coloursCount; ++i )
	{
		float angle;
		ss >> angle;
		sf::Uint32 colour;
		ss >> colour;
		GetInfo().Append( sf::Color( colour ), TORADIANS( angle ) );
	}
	
	for( int i = 0; i < antsCount; ++i )
	{
		float direction, posX, posY;
		ss >> direction >> posX >> posY;
		std::string str;
		ss >> str;
		sf::Uint32 colour;

		if( str == "W" )
			colour = sf::Color::White.toInteger();
		else
			ss >> colour;

		Ant ant( sf::Vector2f( posX * GetWindow().getSize().x, posY * GetWindow().getSize().y ), direction, sf::Color( colour ) );
		GetInfo().ants.push_back( ant );
	}

	ants.clear();
	for( auto& ant : GetInfo().ants )
		ants.emplace_back( ant.startingPos, ant.startingDir );

	SetupGrid();
	Reset( false );
}

void GameState::SaveCustomSetups() const
{
	Json::Value jsonOut;
	Json::Value setups( Json::arrayValue );

	for( unsigned i = 1; i < customStates.size(); ++i )
	{
		Json::Value item;
		item["Name"] = customStates[i].name;
		item["BlendIdx"] = customStates[i].blendIdx;
		item["GridTypeIdx"] = customStates[i].gridTypeIdx;
		item["IncrementalAlpha"] = customStates[i].incrementalAlpha;
		item["IncrementalRGB"] = customStates[i].incrementalRGB;
		item["InfiniteGrid"] = customStates[i].infiniteGrid;
		item["LockToGrid"] = customStates[i].lockToGrid;
		Json::Value coloursAndAngles( Json::arrayValue );
		Json::Value ants( Json::arrayValue );

		for( auto& info : customStates[i].coloursAndAngles )
		{
			Json::Value coloursAndAngle;
			coloursAndAngle["Angle"] = int( TODEGREES( info.second ) );
			Json::Value vec( Json::arrayValue );
			const auto colour = Reflex::ToColour( info.first );
			vec.append( Json::Value( ( int )colour.r ) );
			vec.append( Json::Value( ( int )colour.g ) );
			vec.append( Json::Value( ( int )colour.b ) );
			vec.append( Json::Value( ( int )colour.a ) );
			coloursAndAngle["Colour"] = vec;
			coloursAndAngles.append( coloursAndAngle );
		}

		for( auto& info : customStates[i].ants )
		{
			Json::Value ant;
			ant["Direction"] = info.startingDir;
			ant["PositionX"] = info.startingPos.x / ( float )GetWindow().getSize().x;
			ant["PositionY"] = info.startingPos.y / ( float )GetWindow().getSize().y;
			Json::Value vec( Json::arrayValue );
			vec.append( Json::Value( ( int )info.hue.r ) );
			vec.append( Json::Value( ( int )info.hue.g ) );
			vec.append( Json::Value( ( int )info.hue.b ) );
			vec.append( Json::Value( ( int )info.hue.a ) );
			ant["Hue"] = vec;
			ants.append( ant );
		}

		item["ColoursAndAngles"] = coloursAndAngles;
		item["Ants"] = ants;

		setups.append( item );
	}

	jsonOut["Setups"] = setups;
		
	Json::StreamWriterBuilder builder;
	builder["commentStyle"] = "None";
	builder["indentation"] = "   ";

	std::ofstream outputFileStream( "SimulationData.json" );
	builder.newStreamWriter()->write( jsonOut, &outputFileStream );
}

void GameState::ReadCustomSetups()
{
	customStates.clear();

	StateInfo custom( "Default" );
	custom.Append( sf::Color( 0, 0, 0, 255 ), PI / 2.0f );
	custom.Append( sf::Color::White, -PI / 2.0f );
	customStates.push_back( custom );

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
			newInfo.blendIdx = setup["BlendIdx"].asInt();
			newInfo.gridTypeIdx = setup["GridTypeIdx"].asInt();
			newInfo.incrementalAlpha = setup["IncrementalAlpha"].asBool();
			newInfo.incrementalRGB = setup["IncrementalRGB"].asBool();
			newInfo.infiniteGrid = setup["InfiniteGrid"].asBool();
			newInfo.lockToGrid = setup["LockToGrid"].asBool();
			const auto& coloursAndAngles = setup["ColoursAndAngles"];
			const auto& ants = setup["Ants"];

			for( unsigned j = 0; j < coloursAndAngles.size(); ++j )
			{
				const auto& colourAndAngle = coloursAndAngles[j];
				const auto& colour = colourAndAngle["Colour"];
				sf::Color c( colour[0].asInt(), colour[1].asInt(), colour[2].asInt(), colour[3].asInt() );
				newInfo.Append( c, TORADIANS( colourAndAngle["Angle"].asInt() ) );
			}

			for( unsigned j = 0; j < ants.size(); ++j )
			{
				const auto direction = ants[j]["Direction"].asFloat();
				const auto positionX = ants[j]["PositionX"].asFloat() * GetWindow().getSize().x;
				const auto positionY = ants[j]["PositionY"].asFloat() * GetWindow().getSize().y;
				const auto& colour = ants[j]["Hue"];
				sf::Color c( colour[0].asInt(), colour[1].asInt(), colour[2].asInt(), colour[3].asInt() );
				newInfo.ants.emplace_back( sf::Vector2f( positionX, positionY ), direction, c );
			}

			customStates.push_back( std::move( newInfo ) );
		}
	}
}

void GameState::SetTileColour( const sf::Vector2i& index, const sf::Color& colour, const BlendValue blend )
{
	if( GetInfo().gridTypeIdx == Square || GetInfo().gridTypeIdx == Pixel )
	{
		const auto numVerts = ( tileSize.x <= 1.0f ? 1U : 4U );
		const auto idx = ( index.y * gridSize.x + index.x ) * numVerts;
		auto newColour = colour;
		if( blend == Gamma )
			newColour = Reflex::BlendColourGammaCorrection( gridVertices[idx].color, colour );
		else if( blend == Alpha )
			newColour = Reflex::BlendColourDivideAlpha( gridVertices[idx].color, colour );

		for( unsigned i = 0; i < numVerts; ++i )
			gridVertices[idx + i].color = newColour;
	}
	else if( GetInfo().gridTypeIdx == Hexagon )
	{
		const auto idx = ( index.y * gridSize.x + index.x ) * 18;

		auto newColour = colour;
		if( blend == Gamma )
			newColour = Reflex::BlendColourGammaCorrection( gridVertices[idx].color, colour );
		else if( blend == Alpha )
			newColour = Reflex::BlendColourDivideAlpha( gridVertices[idx].color, colour );

		for( unsigned i = 0; i < 18; ++i )
			gridVertices[idx + i].color = newColour;
	}
}