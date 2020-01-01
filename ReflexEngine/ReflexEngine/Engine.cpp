// Includes
#include "Precompiled.h"
#include "Engine.h"

// Implementation
namespace Reflex::Core
{
	Engine::Engine( const std::string& windowName, const bool fullscreen )
		: m_updateInterval( sf::seconds( 1.0f / 60.f ) )
		, m_window()
		, m_textureManager()
		, m_fontManager()
		, m_stateManager( Context( m_window, m_textureManager, m_fontManager ) )
	{
		// 2560, 1377
		const auto modes = sf::VideoMode::getFullscreenModes();
		m_window.create( modes[0], windowName, fullscreen ? sf::Style::Fullscreen : sf::Style::Default );

		Setup();
	}

	Engine::Engine( const int screenWidth, const int screenHeight, const std::string& windowName )
		: m_updateInterval( sf::seconds( 1.0f / 60.f ) )
		, m_window()
		, m_textureManager()
		, m_fontManager()
		, m_stateManager( Context( m_window, m_textureManager, m_fontManager ) )
	{
		// 2560, 1377
		sf::VideoMode mode;
		mode.width = screenWidth;
		mode.height = screenHeight;
		m_window.create( mode, windowName, sf::Style::Default );

		Setup();
	}
	
	void Engine::Setup()
	{
		srand( (unsigned )time( 0 ) );
		m_window.setPosition( sf::Vector2i( -6, 0 ) );
		ImGui::SFML::Init( m_window );
		Profiler::GetProfiler();
	}

	Engine::~Engine()
	{
		ImGui::SFML::Shutdown();
	}

	void Engine::Run()
	{
		m_totalTime.restart();

		try
		{
			sf::Clock clock;
			sf::Time accumlatedTime = sf::Time::Zero;

			while( m_window.isOpen() )
			{
				sf::Time deltaTime = clock.restart();
				Profiler::GetProfiler().FrameTick( deltaTime.asMicroseconds() );
				ProcessEvents();

				accumlatedTime += deltaTime;
				bool updated = false;

				while( accumlatedTime > m_updateInterval )
				{
					accumlatedTime -= m_updateInterval;
					ProcessEvents();
					Update( m_updateInterval.asSeconds() );
					updated = true;
				}

				ImGui::SFML::Update( m_window, deltaTime );
				UpdateStatistics( deltaTime.asSeconds() );
				Render();
			}

			Profiler::GetProfiler().OutputResults( "Performance_Results.txt" );
		}
		catch( std::exception& e )
		{
			LOG_CRIT( "*EXCEPTION: " << *e.what() );
			throw;
		}
	}

	void Engine::ProcessEvents()
	{
		sf::Event processEvent;

		while( m_window.pollEvent( processEvent ) )
		{
			ImGui::SFML::ProcessEvent( processEvent );
			m_stateManager.ProcessEvent( processEvent );

			switch( processEvent.type )
			{
			case sf::Event::KeyPressed:
				KeyboardInput( processEvent.key.code, true );
				break;

			case sf::Event::KeyReleased:
				KeyboardInput( processEvent.key.code, false );
				break;

			case sf::Event::Closed:
				m_window.close();
				break;
			default: break;
			}
		}
	}

	void Engine::KeyboardInput( const sf::Keyboard::Key key, const bool isPressed )
	{
		if( key == sf::Keyboard::Escape )
			m_window.close();
	}

	void Engine::Update( const float deltaTime )
	{
		m_stateManager.Update( deltaTime );
	}

	void Engine::Render()
	{
		m_window.clear( sf::Color::Black );
		m_stateManager.Render();
		m_window.setView( m_window.getDefaultView() );

		ImGui::SetNextWindowPos( sf::Vector2( 5.0f, 5.0f ), ImGuiCond_::ImGuiCond_Once );
		ImGui::SetNextWindowSize( sf::Vector2( 200.0f, 200.0f ), ImGuiCond_::ImGuiCond_Once );
		ImGui::Begin( "Engine Info" );
		ImGui::Text( m_statisticsText.toAnsiString().c_str() );

		ImGui::Checkbox( "Show ImGui Metrics", &m_showMetrics );
		ImGui::Checkbox( "Show ImGui Style Editor", &m_showStyleEditor );

		if( m_showMetrics )
			ImGui::ShowMetricsWindow();

		if( m_showStyleEditor ) 
		{ 
			ImGui::Begin( "Style Editor", &m_showStyleEditor );
			ImGui::ShowStyleEditor(); 
			ImGui::End(); 
		}

		ImGui::End();

		ImGui::SFML::Render( m_window );
		m_window.display();
	}

	void Engine::UpdateStatistics( const float deltaTime )
	{
		m_statisticsUpdateTime += sf::seconds( deltaTime );
		m_statisticsNumFrames += 1;

		if( m_statisticsUpdateTime >= sf::seconds( 1.0f ) )
		{
			const auto ms_per_frame = m_statisticsUpdateTime.asMilliseconds() / ( float )m_statisticsNumFrames;
			std::stringstream ss;
			ss << "FPS: " << std::to_string( m_statisticsNumFrames ) << "\nFrame Time: ";
			
			if( ms_per_frame > 0 )
				ss << std::fixed << std::setprecision( 2 ) << ms_per_frame << "ms";
			else
				ss << ( m_statisticsUpdateTime.asMicroseconds() / m_statisticsNumFrames ) << "us";

			ss << "\nDuration: " << ( int )m_totalTime.getElapsedTime().asSeconds() << "s";

			m_statisticsText = ss.str();
			m_statisticsUpdateTime -= sf::seconds( 1.0f );
			m_statisticsNumFrames = 0;
		}
	}
}