#pragma once

// Includes
#include "Precompiled.h"
#include "ResourceManager.h"
#include "StateManager.h"
#include "Object.h"

namespace Reflex::Core
{
	// Engine class
	class Engine : private sf::NonCopyable
	{
	public:
		Engine( const std::string& windowName = "Reflex Engine", const bool fullscreen = false );
		Engine( const int screenWidth, const int screenHeight, const std::string& windowName = "Reflex Engine" );
		~Engine();

		void Run();

		template< typename T >
		void RegisterState( const bool isStartingState = false );

		sf::RenderWindow& GetWindow() { return m_window; }

	protected:
		void Setup();
		void KeyboardInput( const sf::Keyboard::Key key, const bool isPressed );
		void ProcessEvents();
		void Update( const float deltaTime );
		void Render();

		void UpdateStatistics( const float deltaTime );

	protected:
		// Core window
		sf::RenderWindow m_window;

		// Resource managers
		TextureManager m_textureManager;
		FontManager m_fontManager;

		// State manager (handles different scenes & transitions, contains worlds which hold objects etc.)
		StateManager m_stateManager;

		// Stats
		const sf::Time m_updateInterval;
		sf::String m_statisticsText;
		sf::Time m_statisticsUpdateTime;
		sf::Clock m_totalTime;
		unsigned int m_statisticsNumFrames = 0U;

		// ImGui
		bool m_showMetrics = false;
		bool m_showStyleEditor = false;
		int m_fpsLimit = 0;
	};

	template< typename T >
	void Engine::RegisterState( const bool isStartingState )
	{
		m_stateManager.RegisterState< T >();

		if( isStartingState )
			m_stateManager.PushState< T >();
	}
}