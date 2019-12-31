#pragma once

// Includes
#include "Context.h"
#include "World.h"

namespace Reflex::Core
{
	class StateManager;

	class State
	{
	public:
		friend class StateManager;

		State( StateManager& stateManager, Context context );
		virtual ~State() { }

		virtual void Render() { }
		virtual void Update( const float deltaTime ) { }
		virtual void ProcessEvent( const sf::Event& event ) { }

	protected:
		template< typename T >
		void RequestNewState();
		void RequestRemoveState();
		void RequestRemoveAllStates();

		sf::RenderWindow& GetWindow() { return m_context.window; }
		TextureManager& GetTextureManager() { return m_context.textureManager; }
		FontManager& GetFontManager() { return m_context.fontManager; }

	private:
		State();

	private:
		StateManager& m_stateManager;
		Context m_context;

		sf::FloatRect m_windowBounds;
		World m_world;
	};

	class StateManager : private sf::NonCopyable
	{
	public:
		enum class Action
		{
			Push,
			Pop,
			Clear,
		};

		explicit StateManager( Context context );

		template< typename T >
		void RegisterState();

		void Update( const float deltaTime );
		void Render();
		void ProcessEvent( const sf::Event& event );

		template< typename T >
		void PushState();

		void PopState();
		void ClearStates();

		bool IsEmpty() const;

	private:
		template< typename T >
		std::unique_ptr< State > CreateState();
		std::unique_ptr< State > CreateState( const Type stateType );
		void ApplyPendingChanges();

	private:
		std::vector< std::pair< Type, Action > > m_pendingList;
		std::vector< std::unique_ptr< State > > m_ActiveStates;
		Context m_context;

		std::unordered_map< Type, std::function< std::unique_ptr< State >( void ) > > m_stateFactories;
	};

	// Template functions
	template< typename T >
	void StateManager::RegisterState()
	{
		const auto stateType = Type( typeid( T ) );

		m_stateFactories[stateType] = [this]()
		{
			return std::make_unique< T >( *this, m_context );
		};
	}

	template< typename T >
	std::unique_ptr< State > StateManager::CreateState()
	{
		const auto stateType = Type( typeid( T ) );
		return CreateState( stateType );
	}

	template< typename T >
	void StateManager::PushState()
	{
		const auto stateType = Type( typeid( T ) );
		m_pendingList.push_back( std::make_pair( stateType, Action::Push ) );
	}

	template< typename T >
	void State::RequestNewState()
	{
		m_stateManager.PushState< T >();
	}
}