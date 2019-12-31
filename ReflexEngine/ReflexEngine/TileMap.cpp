#include "Precompiled.h"
#include "TileMap.h"

#include "TransformComponent.h"
#include "Object.h"

TODO( "Allow TileMap work when a pos outside the bounds is entered - dynamically resize the array" )

namespace Reflex::Core
{
	TileMap::TileMap( const sf::FloatRect& worldBounds )
		: m_worldBounds( worldBounds )
		//, m_tileMapGridSize( tileMapGridSize )
	{
	}

	TileMap::TileMap( const sf::FloatRect& worldBounds, const unsigned spacialHashMapSize )
		: m_worldBounds( worldBounds )
		//, m_tileMapGridSize( tileMapGridSize )
	{
		Reset( spacialHashMapSize, false );
	}

	void TileMap::Insert( const ObjectHandle& obj )
	{
		if( obj && m_spacialHashMapSize )
		{
			const auto position = obj->GetComponent< Reflex::Components::Transform >()->GetWorldPosition();
			Insert( obj, sf::FloatRect( position, sf::Vector2f( 0.0f, 0.0f ) ) );
		}
	}

	void TileMap::Insert( const ObjectHandle& obj, const sf::FloatRect& boundary )
	{
		if( obj && m_spacialHashMapSize )
		{
			const auto ids = GetID( boundary );

			for( auto& id : ids )
			{
				if( id == -1 )
					continue;

				auto& bucket = m_spacialHashMap[id];
				bucket.emplace_back( obj );
			}
		}
	}

	void TileMap::Remove( const ObjectHandle& obj )
	{
		if( obj && m_spacialHashMapSize )
		{
			const auto id = GetID( obj );

			if( id == -1 )
				return;

			auto& bucket = m_spacialHashMap[id];
			const auto found = std::find( bucket.begin(), bucket.end(), obj );
			assert( found != bucket.end() );
			if( found != bucket.end() )
				bucket.erase( found );
		}
	}

	void TileMap::RemoveByID( const ObjectHandle& obj, const unsigned id )
	{
		if( obj && m_spacialHashMapSize && id < m_spacialHashMap.size() )
		{
			auto& bucket = m_spacialHashMap[id];
			const auto found = std::find( bucket.begin(), bucket.end(), obj );
			assert( found != bucket.end() );
			if( found != bucket.end() )
				bucket.erase( found );
		}
	}

	void TileMap::GetNearby( const ObjectHandle& obj, std::vector< ObjectHandle >& out ) const
	{
		ForEachNearby( obj, [&out]( const ObjectHandle& obj )
		{
			out.push_back( obj );
		} );
	}

	void TileMap::GetNearby( const sf::Vector2f& position, std::vector< ObjectHandle >& out ) const
	{
		ForEachNearby( position, [&out]( const ObjectHandle& obj )
		{
			out.push_back( obj );
		} );
	}

	void TileMap::GetNearby( const ObjectHandle& obj, const sf::FloatRect& boundary, std::vector< ObjectHandle >& out ) const
	{
		ForEachNearby( obj, boundary, [&out]( const ObjectHandle& obj )
		{
			out.push_back( obj );
		} );
	}

	void TileMap::Reset( const bool shouldRePopulate /*= false*/ )
	{
		m_spacialHashMap.clear();
		m_spacialHashMapWidth = ( unsigned )std::ceil( m_worldBounds.width / m_spacialHashMapSize );
		m_spacialHashMapHeight = ( unsigned )std::ceil( m_worldBounds.height / m_spacialHashMapSize );
		m_spacialHashMap.resize( m_spacialHashMapWidth * m_spacialHashMapHeight );

		TODO( "TileMap: Implement shouldRePopulate system from world" );
	}

	void TileMap::Reset( const unsigned spacialHashMapSize, const bool shouldRePopulate /*= false*/ )
	{
		m_spacialHashMapSize = spacialHashMapSize;
		Reset( shouldRePopulate );
	}

	unsigned TileMap::GetID( const ObjectHandle& obj ) const
	{
		assert( obj );
		return GetID( obj->GetComponent< Reflex::Components::Transform >()->GetWorldPosition() );
	}

	unsigned TileMap::GetID( const sf::Vector2f& position ) const
	{
		if( position.x < m_worldBounds.left || position.x > ( m_worldBounds.left + m_worldBounds.width ) ||
			position.y < m_worldBounds.top || position.y > ( m_worldBounds.top + m_worldBounds.height ) ||
			!m_spacialHashMapSize )
			return -1;

		const auto loc = Hash( position );
		return loc.y * m_spacialHashMapWidth + loc.x;
	}

	std::vector< unsigned > TileMap::GetID( const sf::FloatRect& boundary ) const
	{
		const auto locTopLeft = Hash( sf::Vector2f( boundary.left, boundary.top ) );
		const auto locBotRight = Hash( sf::Vector2f( boundary.left + boundary.width, boundary.top + boundary.height ) );
		std::vector< unsigned > ids;

		for( int x = locTopLeft.x; x <= locBotRight.x; ++x )
			for( int y = locTopLeft.y; y <= locBotRight.y; ++y )
				ids.push_back( y * m_spacialHashMapWidth+ x );

		return std::move( ids );
	}

	sf::Vector2i TileMap::Hash( const sf::Vector2f& position ) const
	{
		return sf::Vector2i( int( position.x / m_spacialHashMapSize ), int( position.y / m_spacialHashMapSize ) );
	}
}