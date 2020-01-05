// Includes
#include "LangtonsAnt.h"

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
	, size( 100.0f )
	, camera( Reflex::Vector2iToVector2f( context.window.getPosition() ), Reflex::Vector2uToVector2f( context.window.getSize() ), context )
{
	antPlacerDisplay.setFillColor( sf::Color(255, 0, 0, 128 ) );
	Reflex::CenterOrigin( antPlacerDisplay );

	ants.emplace_back( sf::Vector2f( context.window.getSize().x / 2.0f, context.window.getSize().y / 2.0f ), PI );

	ReadCustomSetups();
	SetupGrid();
	Reset( false );
}

void GameState::SetupGrid()
{
	gridStates.clear();
	gridVertices.clear();

	tileSize.x = size;
	tileSize.y = size;

	if( gridTypeIdx == Pixel )
		tileSize = sf::Vector2f( 1.0f, 1.0f );
	else if( gridTypeIdx == Hexagon )
		tileSize.x = sqrtf( 3.0f ) * size / 2.0f;
		
	auto startColour = Reflex::ToColour( customStates[currentStateInfo].coloursAndAngles[0].first );

	const auto heightGap = tileSize.y * ( gridTypeIdx == Hexagon ? 0.75f : 1.0f );
	gridSize.x = ( unsigned )ceil( GetWindow().getSize().x / tileSize.x );
	gridSize.y = ( unsigned )ceil( GetWindow().getSize().y / heightGap );
	gridStates.resize( gridSize.x * gridSize.y );
	gridOrigin = sf::Vector2f( GetWindow().getSize().x / 2.0f - gridSize.x / 2.0f * tileSize.x, GetWindow().getSize().y / 2.0f - gridSize.y / 2.0f * heightGap );

	if( gridTypeIdx == Square || gridTypeIdx == Pixel )
	{
		const auto usePoints = ( tileSize.x <= 1.0f );
		gridVertices.reserve( gridSize.x * gridSize.y * ( usePoints ? 1 : 4 ) );
		const auto dimensions = tileSize / 2.0f;

		for( unsigned y = 0; y < gridSize.y; ++y )
		{
			for( unsigned x = 0; x < gridSize.x; ++x )
			{
				const auto r = Reflex::RandomInt( 255 );
				startColour = sf::Color( r, r, r, 255 );
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
	else if( gridTypeIdx == Hexagon )
	{
		gridVertices.reserve( gridSize.x * gridSize.y * 18 );
		const auto dimensions = sf::Vector2f( tileSize.x / 2.0f, tileSize.y / 4.0f );

		for( unsigned y = 0; y < gridSize.y; ++y )
		{
			for( unsigned x = 0; x < gridSize.x; ++x )
			{
				const auto r = Reflex::RandomInt( 255 );
				startColour = sf::Color( r, r, r, 120 );
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
	
	activeAnts = ants.size();
	generation = 0;
	updateTime = speedValues[speedIdx];
	placingAnts = false;

	if( resetGridCcolours )
		for( unsigned y = 0; y < gridSize.y; ++y )
			for( unsigned x = 0; x < gridSize.x; ++x )
				SetTileColour( sf::Vector2i( x, y ), Reflex::ToColour( customStates[currentStateInfo].coloursAndAngles[0].first ) );

	timer = 0.0f;
}

void GameState::Update( const float deltaTime )
{
	//UpdateAnts( deltaTime );
	UpdateCamera( deltaTime );
}

void GameState::UpdateAnts( const float deltaTime )
{
	if( updateTime <= 0.0f || activeAnts == 0 )
		return;

	timer -= deltaTime;

	while( activeAnts > 0 && ( timer <= 0.0f || ( int )generation < stepTo ) )
	{
		if( timer <= 0.0f )
			timer += updateTime;

		++generation;
		activeAnts = 0;

		for( auto& ant : ants )
		{
			if( !ant.active )
				continue;

			const auto gridIndex = FindGridIndex( ant.currentPos );

			// Remove off screen ants
			if( gridIndex.x <= 0 || gridIndex.y <= 0 || gridIndex.x >= ( int )gridSize.x || gridIndex.y >= ( int )gridSize.y )
			{
				ant.active = false;
				continue;
			}

			++activeAnts;

			// Rotate
			const auto idx = gridIndex.y * gridSize.x + gridIndex.x;
			assert( idx >= 0 && idx < gridStates.size() );
			gridStates[idx] %= customStates[currentStateInfo].statesCount;
			ant.currentDir = Reflex::Modf( ant.currentDir + customStates[currentStateInfo].coloursAndAngles[gridStates[idx]].second, PI2 );

			// Flip tile
			assert( idx >= 0 && ( tileSize.x <= 1.0f ? idx : idx * 4 + 3 ) < gridVertices.size() );
			gridStates[idx] = ( gridStates[idx] + 1 ) % customStates[currentStateInfo].statesCount;
			SetTileColour( gridIndex, Reflex::ToColour( customStates[currentStateInfo].coloursAndAngles[gridStates[idx]].first ), ( GameState::BlendValue )blendIdx );

			// Move
			ant.currentPos.x += std::cosf( ant.currentDir ) * tileSize.x;
			ant.currentPos.y += std::sinf( ant.currentDir ) * tileSize.y;
		}
	}
}

void GameState::UpdateCamera( const float deltaTime )
{
	const auto vertical = Reflex::Sign( (int )sf::Keyboard::isKeyPressed( sf::Keyboard::Down ) ) - Reflex::Sign( ( int )sf::Keyboard::isKeyPressed( sf::Keyboard::Up ) );
	const auto horizontal = Reflex::Sign( ( int )sf::Keyboard::isKeyPressed( sf::Keyboard::Right ) ) - Reflex::Sign( ( int )sf::Keyboard::isKeyPressed( sf::Keyboard::Left ) );

	if( vertical || horizontal )
		camera.GetView().move( deltaTime * sf::Vector2f( ( float )horizontal, ( float )vertical ) * cameraSpeed );
}

sf::Vector2i GameState::FindGridIndex( const sf::Vector2f& pos )
{
	if( gridTypeIdx == Square || gridTypeIdx == Pixel )
		return sf::Vector2i( Reflex::RoundToInt( ( pos.x - gridOrigin.x ) / tileSize.x ), Reflex::RoundToInt( ( pos.y - gridOrigin.y ) / tileSize.y ) );

	// Hexagon
	const auto halfSizeX = tileSize.x / 2.0f;
	const auto offset = ( sqrtf( 3.0f ) * halfSizeX ) / 2.0f;
	const auto heightGap = tileSize.y * ( gridTypeIdx == Hexagon ? 0.75f : 1.0f );
	const auto x = pos.x - gridOrigin.x;
	const auto y = pos.y - gridOrigin.y - ( size - heightGap ) + offset;
	auto row = Reflex::RoundToInt( y / heightGap );

	int column;
	const auto rowIsOdd = row & 1;

	if( rowIsOdd )
		column = Reflex::RoundToInt( ( x - halfSizeX ) / tileSize.x );
	else
		column = Reflex::RoundToInt( x / tileSize.x );

	const auto c = halfSizeX / 2.0f;
	const auto relY = y - row * ( heightGap - 1 ) + c;
	float relX;
	
	if( rowIsOdd )
		relX = Reflex::Modf( x, tileSize.x );
	else
		relX = Reflex::Modf( x + halfSizeX, tileSize.x );

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
	if( placingAnts && event.type == sf::Event::MouseButtonPressed )
	{
		if( event.mouseButton.button == sf::Mouse::Button::Right )
		{
			placingAnts = false;
		}
		else if( event.mouseButton.button == sf::Mouse::Button::Left )
		{
			const auto mousePosition = GetWindow().mapPixelToCoords( sf::Mouse::getPosition( GetWindow() ) );
			ants.emplace_back( sf::Vector2f( Reflex::RoundToInt( mousePosition.x / tileSize.x ) * tileSize.x, Reflex::RoundToInt( mousePosition.y / tileSize.y ) * tileSize.y ), PI );
		}
	}

	if( event.type == sf::Event::MouseWheelScrolled )
	{
		static float cameraZoom = 1.0f;
		cameraZoom += event.mouseWheelScroll.delta * zoomSpeed;

		if( event.mouseWheelScroll.delta < 0 )
		{
			camera.GetView().zoom( zoomSpeed );
			cameraZoom *= zoomSpeed;
		}
		else
		{
			camera.GetView().zoom( 1.0f / zoomSpeed );
			cameraZoom *= ( 1.0f / zoomSpeed );
		}
	}
}

void GameState::Render()
{
	//GetWindow().setView( camera.GetView() );

	GetWindow().draw( gridVertices.data(), gridVertices.size(), gridTypeIdx == Square ? sf::Quads : ( tileSize.x <= 1.0f ? sf::Points : sf::Triangles ) );
	const auto mousePos = Reflex::Vector2iToVector2f( sf::Mouse::getPosition( GetWindow() ) );

	if( placingAnts )
	{
		antPlacerDisplay.setPosition( mousePos );
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

	static sf::Vector2i highlight( -999, 0 );
	const auto newHighlight = FindGridIndex( mousePos );

	if( newHighlight != highlight && newHighlight.x >= 0 && newHighlight.y >= 0 && newHighlight.x < gridSize.x && newHighlight.y < gridSize.y )
	{
		static sf::Color oldColour;

		if( highlight.x != -999 )
			SetTileColour( highlight, oldColour );

		highlight = newHighlight;
		const auto idx = ( highlight.y * gridSize.x + highlight.x ) * 18;
		oldColour = gridVertices[idx].color;

		SetTileColour( newHighlight, sf::Color::Magenta );
	}

	// Draw settings / display UI
	ImGui::SetNextWindowPos( sf::Vector2( 5.0f, 0.17f * GetWindow().getSize().y ), ImGuiCond_::ImGuiCond_Once );
	ImGui::SetNextWindowSize( sf::Vector2( 400.0f, 400.0f ), ImGuiCond_::ImGuiCond_Once );
	ImGui::Begin( "Langton's Ant", nullptr, ImGuiWindowFlags_AlwaysAutoResize );
	ImGui::Text( ( "Generation " + std::to_string( generation ) ).c_str() );
	ImGui::Separator();
	ImGui::NewLine();

	{
		const auto prev = gridTypeIdx;
		ImGui::Text( "Grid Type" );
		ImGui::RadioButton( "Square", &gridTypeIdx, Square ); ImGui::SameLine();
		ImGui::RadioButton( "Hexagon", &gridTypeIdx, Hexagon ); ImGui::SameLine();
		ImGui::RadioButton( "Pixel", &gridTypeIdx, Pixel );

		if( ImGui::InputFloat( "Grid Size", &size, 1.0f, 5.0f, 1 ) || prev != gridTypeIdx )
		{
			SetupGrid();
			Reset();
		}
	}

	ImGui::NewLine();

	// Handler for radio buttons for adjusting speed
	{
		const auto prev = speedIdx;
		ImGui::Text( "Update Speed" );
		ImGui::RadioButton( "Pause", &speedIdx, Pause ); ImGui::SameLine();
		ImGui::RadioButton( "Slow", &speedIdx, Slow ); ImGui::SameLine();
		ImGui::RadioButton( "Regular", &speedIdx, Regular ); ImGui::SameLine();
		ImGui::RadioButton( "Fast", &speedIdx, Fast ); ImGui::SameLine();
		ImGui::RadioButton( "Very Fast", &speedIdx, VeryFast );

		if( prev != speedIdx )
			updateTime = speedValues[speedIdx];

		const auto g = floorf( log10f( ( float )stepTo ) );
		const auto baseRate = std::max( 100, ( int )powf( 10.0f, g ) );
		ImGui::InputInt( "Set Generation", &stepTo, 
			baseRate,										// Decrement
			baseRate, 0,									// Decrement fast
			baseRate,										// Increment
			std::max( 100, ( int )powf( 10.0f, g + 1 ) ) );	// Increment fast
		stepTo = std::max( 0, stepTo );
	}

	// Manual speed adjustment with slider
	ImGui::SliderFloat( "Update Time", &updateTime, 0.0f, 0.5f );

	// Number of states for this sim
	if( ImGui::SliderInt( "State Count", &customStates[currentStateInfo].statesCount, 2, 20 ) )
	{
		while( customStates[currentStateInfo].coloursAndAngles.size() < (unsigned )customStates[currentStateInfo].statesCount )
		{
			customStates[currentStateInfo].Append( Reflex::RandomColour(), Reflex::RandomBool() ? -PIDIV2 : PIDIV2 );
			customStates[currentStateInfo].statesCount--;
		}

		customStates[currentStateInfo].coloursAndAngles.resize( customStates[currentStateInfo].statesCount );
	}

	ImGui::NewLine();

	// Settings for reach state (the colour of the state + angle to rotate when the ant steps on this tile)
	if( ImGui::CollapsingHeader( "States Data", true ) )
	{
		bool colourChange = false;

		for( int i = 0; i < customStates[currentStateInfo].statesCount; ++i )
		{
			if( ImGui::TreeNode( ( "State " + std::to_string( i ) ).c_str() ) )
			{
				if( ImGui::ColorEdit4( "State Colour", customStates[currentStateInfo].coloursAndAngles[i].first._Elems ) )
				{
					colourChange = true;
					UpdateIncrementalColours( i );
				}

				ImGui::SliderAngle( "State Rotation", &customStates[currentStateInfo].coloursAndAngles[i].second );
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

	ImGui::Checkbox( "Display Ants", &displayAnts );

	ImGui::NewLine();

	ImGui::Text( "Colour Blending" );
	ImGui::RadioButton( "Disabled", &blendIdx, Disable ); ImGui::SameLine();
	ImGui::RadioButton( "Gamma", &blendIdx, Gamma ); ImGui::SameLine();
	ImGui::RadioButton( "Alpha", &blendIdx, Alpha );

	if( ImGui::Checkbox( "Incremental RGB", &incrementalRGB ) )
		UpdateIncrementalColours();

	ImGui::SameLine();
	if( ImGui::Checkbox( "Incremental Alpha", &incrementalAlpha ) )
		UpdateIncrementalColours();

	ImGui::NewLine();

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
			ReadCustomSetups();
			customStates.push_back( std::move( copiedCustomState ) );
			SaveCustomSetups();
			currentItem = customStates[0].name.c_str();
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

void GameState::UpdateIncrementalColours( const unsigned startIdx )
{
	if( !incrementalAlpha && !incrementalRGB )
		return;

	const auto colours = customStates[currentStateInfo].coloursAndAngles;
	sf::Vector3f baseC( colours[startIdx].first[0], colours[startIdx].first[1], colours[startIdx].first[2] );
	sf::Vector3f newC = baseC;
	const auto startAlpha = 0.1f;
	float alpha = startAlpha;

	for( unsigned i = startIdx; i < customStates[currentStateInfo].coloursAndAngles.size(); ++i )
	{
		if( incrementalRGB )
		{
			customStates[currentStateInfo].coloursAndAngles[i].first[0] = std::min( 1.0f, newC.x );
			customStates[currentStateInfo].coloursAndAngles[i].first[1] = std::min( 1.0f, newC.y );
			customStates[currentStateInfo].coloursAndAngles[i].first[2] = std::min( 1.0f, newC.z );
			newC += ( sf::Vector3f( 1.0f, 1.0f, 1.0f ) - baseC ) / ( float )customStates[currentStateInfo].statesCount;
		}
		else if( incrementalAlpha )
		{
			customStates[currentStateInfo].coloursAndAngles[i].first[3] = std::min( 1.0f, alpha );
			alpha += ( 1.0f - startAlpha ) / ( float )customStates[currentStateInfo].statesCount;
		}
	}

	Recolour();
}

void GameState::Recolour()
{
	for( unsigned y = 0; y < gridSize.y; ++y )
		for( unsigned x = 0; x < gridSize.x; ++x )
			SetTileColour( sf::Vector2i( x, y ), Reflex::ToColour( customStates[currentStateInfo].coloursAndAngles[gridStates[y * gridSize.x + x] % customStates[currentStateInfo].statesCount].first ) );
}

void GameState::SaveCustomSetups()
{
	Json::Value jsonOut;
	Json::Value setups( Json::arrayValue );

	for( unsigned i = 1; i < customStates.size(); ++i )
	{
		Json::Value item;
		item["Name"] = customStates[i].name;
		Json::Value coloursAndAngles( Json::arrayValue );

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

void GameState::SetTileColour( const sf::Vector2i& index, const sf::Color& colour, const BlendValue blend )
{
	if( gridTypeIdx == Square || gridTypeIdx == Pixel )
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
	else if( gridTypeIdx == Hexagon )
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