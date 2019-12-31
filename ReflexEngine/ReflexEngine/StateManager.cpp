#include "Precompiled.h"
#include "StateManager.h"

namespace Reflex::Core
{
	StateManager::StateManager( Context context )
		: m_context( context )
	{

	}

	void StateManager::Update( const float deltaTime )
	{
		const auto size = m_pendingList.size();
		for( auto itr = m_ActiveStates.rbegin(); itr != m_ActiveStates.rend() && size == m_pendingList.size(); ++itr )
		{
			( *itr )->m_world.Update( deltaTime );
			( *itr )->Update( deltaTime );
		}

		ApplyPendingChanges();
	}

	void StateManager::Render()
	{
		for( auto& state : m_ActiveStates )
		{
			state->m_world.Render();
			state->Render();
		}
	}

	void StateManager::ProcessEvent( const sf::Event& event )
	{
		const auto size = m_pendingList.size();
		for( auto itr = m_ActiveStates.rbegin(); itr != m_ActiveStates.rend() && size == m_pendingList.size(); ++itr )
		{
			( *itr )->m_world.ProcessEvent( event );
			( *itr )->ProcessEvent( event );
		}

		ApplyPendingChanges();
	}

	void StateManager::PopState()
	{
		m_pendingList.push_back( std::make_pair( Type( typeid( int ) ), Action::Pop ) );
	}

	void StateManager::ClearStates()
	{
		m_pendingList.push_back( std::make_pair( Type( typeid( int ) ), Action::Clear ) );
	}

	bool StateManager::IsEmpty() const
	{
		return m_ActiveStates.empty();
	}

	void StateManager::ApplyPendingChanges()
	{
		for( auto& change : m_pendingList )
		{
			switch( change.second )
			{
			case Action::Push:
				m_ActiveStates.push_back( CreateState( change.first ) );
				break;
			case Action::Pop:
				m_ActiveStates.pop_back();
				break;
			case Action::Clear:
				m_ActiveStates.clear();
				break;
			}
		}

		m_pendingList.clear();
	}

	std::unique_ptr< State > StateManager::CreateState( const Type stateType )
	{
		const auto found = m_stateFactories.find( stateType );

		if( found == m_stateFactories.end() )
		{
			LOG_CRIT( "Failed to find creation function for state " << stateType.name() );
			return nullptr;
		}

		return found->second();
	}

	// State class
	State::State( StateManager& stateManager, Context context )
		: m_stateManager( stateManager )
		, m_context( context )
		, m_windowBounds( 0.0f, 0.0f, (float )context.window.getSize().x, (float )context.window.getSize().y )
		, m_world( context, m_windowBounds, 100 )
	{

	}

	void State::RequestRemoveState()
	{
		m_stateManager.PopState();
	}

	void State::RequestRemoveAllStates()
	{
		m_stateManager.ClearStates();
	}
}
