#pragma once

#include "Precompiled.h"
#include "Component.h"

namespace Reflex::Core { class World; }

namespace Reflex::Systems
{
	using namespace Reflex::Core;

#define RequiresComponent( T ) m_requiredComponentTypes.push_back( Type( typeid( T ) ) );

	class System : private sf::NonCopyable, public sf::Drawable
	{
	public:
		friend class Reflex::Core::World;

		// Constructors / Destructors
		System( World& world ) : m_world( world ) { }
		virtual ~System() { }

		const std::vector< Type >& GetRequiredComponentTypes() const { return m_requiredComponentTypes; }
		World& GetWorld() { return m_world; }

		typedef std::vector< Handle< Reflex::Components::Component > > ComponentsSet;

	protected:
		virtual void RegisterComponents() = 0;
		virtual void Update( const float deltaTime ) { }
		virtual void ProcessEvent( const sf::Event& event ) { }
		virtual void Render( sf::RenderTarget& target, sf::RenderStates states ) const { }

		virtual void OnSystemStartup() { }
		virtual void OnSystemShutdown() { }
		virtual void OnComponentAdded() { }
		virtual std::vector< ComponentsSet >::const_iterator GetInsertionIndex( const ComponentsSet& newSet ) const { return m_components.end(); }

		template< typename T >
		Handle< T > GetSystemComponent( const ComponentsSet& set ) const
		{
			const auto type = Type( typeid( T ) );
			for( unsigned i = 0U; i < m_requiredComponentTypes.size(); ++i )
				if( m_requiredComponentTypes[i] == type )
					return Handle< T >( set[i] );
			return Handle< T >();
		}

		template< typename T, typename Func >
		void ForEachSystemComponent( const Func& f ) const
		{
			for( auto& comp : m_components )
				f( Handle< T >( comp[0] ) );
		}

		template< typename A, typename B, typename Func >
		void ForEachSystemComponent( const Func& f ) const
		{
			for( auto& comp : m_components )
				f( Handle< A >( comp[0] ), Handle< B >( comp[1] ) );
		}

		template< typename A, typename B, typename C, typename Func >
		void ForEachSystemComponent( const Func& f ) const
		{
			for( auto& comp : m_components )
				f( Handle< A >( comp[0] ), Handle< B >( comp[1] ), Handle< C >( comp[2] ) );
		}

	private:
		void draw( sf::RenderTarget& target, sf::RenderStates states ) const final { Render( target, states ); }

	protected:
		std::vector< ComponentsSet > m_components;
		std::vector< Type > m_requiredComponentTypes;

	private:
		World& m_world;
	};
}