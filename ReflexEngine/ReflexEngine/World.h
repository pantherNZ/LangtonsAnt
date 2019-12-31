#pragma once

#include "Precompiled.h"
#include "ResourceManager.h"
#include "Context.h"
#include "System.h"
#include "TransformComponent.h"

// Engine class
namespace Reflex::Core
{
	class Object;

	// World class
	class World : private sf::NonCopyable
	{
	public:
		explicit World( Context context, sf::FloatRect worldBounds, const unsigned initialMaxObjects );
		~World();

		void Update( const float deltaTime );
		void ProcessEvent( const sf::Event& event );
		void Render();

		ObjectHandle CreateObject( const sf::Vector2f& position = sf::Vector2f(), const float rotation = 0.0f, const sf::Vector2f& scale = sf::Vector2f( 1.0f, 1.0f ) );
		ObjectHandle CreateObject( const bool attachToRoot, const sf::Vector2f& position = sf::Vector2f(), const float rotation = 0.0f, const sf::Vector2f& scale = sf::Vector2f( 1.0f, 1.0f ) );

		void DestroyObject( ObjectHandle object );

		void DestroyAllObjects();

		template< class T, typename... Args >
		T* AddSystem( Args&& ... args );

		template< class T >
		T* GetSystem();

		template< class T >
		void RemoveSystem();

		template< typename Func >
		void ForEachObject( Func function );

		sf::RenderWindow& GetWindow() { return m_context.window; }
		TextureManager& GetTextureManager() { return m_context.textureManager; }
		FontManager& GetFontManager() { return m_context.fontManager; }

		const sf::FloatRect GetBounds() const;
		ObjectHandle GetSceneObject( const unsigned index = 0U ) const;
		Reflex::Components::TransformHandle GetSceneRoot() const;

		void AddComponentToSystems( const ObjectHandle& owner, const Handle< Reflex::Components::Component >& componentHandle, const Type& componentType );

	protected:
		void Setup();

	private:
		World() = delete;

	protected:
		Context m_context;
		sf::View m_worldView;
		sf::FloatRect m_worldBounds;

		// Storage for all objects in the game
		std::vector< std::shared_ptr< Object > > m_objects;

		// List of systems, indexed by their type, holds memory for all the Systems
		std::unordered_map< Type, std::unique_ptr< Reflex::Systems::System > > m_systems;

		// Tilemap which stores object handles in the world in an efficient spacial hash map
		//TileMap m_tileMap;
		Reflex::Components::TransformHandle m_sceneGraphRoot;

		// Removes objects / components on frame move instead of during sometime dangerous
		std::vector< ObjectHandle > m_markedForDeletion;
	};

	// Template functions
	template< typename Func >
	void World::ForEachObject( Func function )
	{
		for( unsigned i = 1; i < m_objects.size(); ++i )
			function( *m_objects[i] );
	}

	template< class T, typename... Args >
	T* World::AddSystem( Args&& ... args )
	{
		const auto type = Type( typeid( T ) );

		if( m_systems.find( type ) != m_systems.end() )
		{
			LOG_CRIT( "Trying to add a system that has already been added!" );
			return nullptr;
		}

		auto system = std::make_unique< T >( *this, std::forward< Args >( args )... );

		std::vector< Type > requiredComponentTypes;

		// Register components
		system->RegisterComponents();

		// Look for any existing objects that match what this system requires and add them to the system's list
		for( auto object : m_objects )
		{
			std::vector< Handle< Reflex::Components::Component > > tempList;

			for( auto& requiredType : system->m_requiredComponentTypes )
			{
				const auto handle = object->GetComponent( requiredType );

				if( !handle )
					break;

				tempList.push_back( handle );
			}

			if( tempList.size() < system->m_requiredComponentTypes.size() )
				continue;

			const auto insertionIter = system->GetInsertionIndex( tempList );
			system->m_components.insert( insertionIter, std::move( tempList ) );
		}

		auto result = m_systems.insert( std::make_pair( type, std::move( system ) ) );
		assert( result.second );

		result.first->second->OnSystemStartup();

		return ( T* )result.first->second.get();
	}

	template< class T >
	void World::RemoveSystem()
	{
		const auto systemType = Type( typeid( T ) );

		for( auto iter = m_systems.begin(); iter != m_systems.end(); ++iter )
		{
			if( systemType == Type( typeid( iter->first.get() ) ) )
			{
				iter->first->OnSystemShutdown();
				iter->first.release();
				m_systems.erase( iter );
				break;
			}
		}
	}

	template< class T >
	T* World::GetSystem()
	{
		const auto systemType = Type( typeid( T ) );

		for( auto iter = m_systems.begin(); iter != m_systems.end(); ++iter )
			if( systemType == iter->first )
				return (T* )iter->second.get();

		return nullptr;
	}
}