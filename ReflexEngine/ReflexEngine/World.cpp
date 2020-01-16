#include "Precompiled.h"

#include "World.h"
#include "Object.h"
#include "TransformComponent.h"

#include "RenderSystem.h"
#include "InteractableSystem.h"
#include "MovementSystem.h"
#include "CameraSystem.h"

namespace Reflex::Core
{
	World::World( Context context, sf::FloatRect worldBounds, const unsigned initialMaxObjects )
		: m_context( context )
		, m_worldView( context.window.getDefaultView() )
		, m_worldBounds( worldBounds )
		//, m_tileMap( m_worldBounds )
	{
		m_objects.reserve( initialMaxObjects );
		Setup();
	}

	World::~World()
	{
		DestroyAllObjects();
	}

	void World::Setup()
	{
		AddSystem< Reflex::Systems::RenderSystem >();
		AddSystem< Reflex::Systems::InteractableSystem >();
		AddSystem< Reflex::Systems::MovementSystem >();
		AddSystem< Reflex::Systems::CameraSystem >();

		m_sceneGraphRoot = CreateObject( false )->GetTransform();
	}

	void World::Update( const float deltaTime )
	{
		// Update systems
		for( auto& system : m_systems )
			system.second->Update( deltaTime );

		// Deleting objects
		for( auto& obj : m_markedForDeletion )
		{
			if( !obj.IsValid() )
				continue;

			const auto found = std::find( m_objects.begin(), m_objects.end(), obj.Lock() );
			assert( found != m_objects.end() );
			if( found != m_objects.end() )
				m_objects.erase( found );
		}

		m_markedForDeletion.clear();
	}

	void World::ProcessEvent( const sf::Event& event )
	{
		for( auto& system : m_systems )
			system.second->ProcessEvent( event );
	}

	void World::Render()
	{
		GetWindow().setView( activeCamera ? *activeCamera : m_worldView );

		for( auto& system : m_systems )
			GetWindow().draw( *system.second );
	}

	ObjectHandle World::CreateObject( const sf::Vector2f& position, const float rotation, const sf::Vector2f& scale )
	{
		return CreateObject( true, position, rotation, scale );
	}

	ObjectHandle World::CreateObject( const bool attachToRoot, const sf::Vector2f& position, const float rotation, const sf::Vector2f& scale )
	{
		auto newObject = std::make_shared< Object >( *this );
		newObject->AddComponent< Reflex::Components::Transform >( position, rotation, scale );
		m_objects.emplace_back( newObject );

		if( attachToRoot )
		{
			assert( m_sceneGraphRoot.IsValid() );
			m_sceneGraphRoot->AttachChild( newObject );
		}

		return ObjectHandle( newObject );
	}

	void World::DestroyObject( ObjectHandle object )
	{
		m_markedForDeletion.push_back( object );
	}

	void World::DestroyAllObjects()
	{
		m_objects.clear();
	}

	sf::FloatRect World::GetBounds() const
	{
		return m_worldBounds;
	}

	ObjectHandle World::GetSceneObject( const unsigned index /*= 0U*/ ) const
	{
		assert( m_sceneGraphRoot.IsValid() );
		return m_sceneGraphRoot->GetChild( index );
	}

	Reflex::Components::TransformHandle World::GetSceneRoot() const
	{
		return m_sceneGraphRoot;
	}

	void World::AddComponentToSystems( const ObjectHandle& object, const Handle< Reflex::Components::Component >& componentHandle, const Type& componentType )
	{
		// Here we want to check if we should add this component to any systems
		for( auto iter = m_systems.begin(); iter != m_systems.end(); ++iter )
		{
			// If the system doesn't care about this type, skip it
			const auto& requiredTypes = iter->second->m_requiredComponentTypes;
			if( std::find( requiredTypes.begin(), requiredTypes.end(), componentType ) == requiredTypes.end() )
				continue;

			auto& componentsPerObject = iter->second->m_components;

			std::vector< Handle< Reflex::Components::Component > > tempList;
			bool canAddDueToNewComponent = false;

			// This looks through the required types and sees if the object has one of each of them
			for( auto& requiredType : requiredTypes )
			{
				const auto handle = ( requiredType == componentType ? componentHandle : object->GetComponent( requiredType ) );

				if( !handle.IsValid() )
					break;

				if( handle.Get() == componentHandle.Get() )
					canAddDueToNewComponent = true;

				tempList.push_back( handle );
			}

			// Escape if the object didn't have the components required OR if our new component isn't even the one that is now allowing it to be a part of the system
			if( tempList.size() < requiredTypes.size() || !canAddDueToNewComponent )
				continue;

			const auto insertionIter = iter->second->GetInsertionIndex( tempList );
			iter->second->m_components.insert( insertionIter, std::move( tempList ) );
			iter->second->OnComponentAdded();
		}
	}

	bool World::IsActiveCamera( const Reflex::Components::CameraHandleConst& camera ) const
	{
		return camera && activeCamera == camera;
	}

	void World::SetActiveCamera( const Reflex::Components::CameraHandle& camera )
	{
		if( !camera )
		{
			LOG_CRIT( "World::SetActiveCamera called with an invalid camera component handle" );
			return;
		}

		activeCamera = camera;
		GetWindow().setView( *activeCamera );
	}

	sf::Vector2f World::GetMousePosition( const Reflex::Components::CameraHandle& camera ) const
	{
		if( camera )
			return GetWindow().mapPixelToCoords( sf::Mouse::getPosition( GetWindow() ), *camera );
		return GetWindow().mapPixelToCoords( sf::Mouse::getPosition( GetWindow() ) );
	}
}